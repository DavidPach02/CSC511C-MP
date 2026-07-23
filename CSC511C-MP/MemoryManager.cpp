#include "MemoryManager.h"
#include "BackingStore.h"
#include "TextFormatter.h"
#include "MemoryLogger.h"

#include <algorithm>
#include <mutex>

MemoryManager* MemoryManager::instance = nullptr;

/**
 * @brief Returns the singleton MemoryManager instance.
 * @returns Pointer to the global memory manager, creating it on first use.
 */
MemoryManager* MemoryManager::GetInstance() {
	if (instance == nullptr) {
		instance = new MemoryManager();
	}
	return instance;
}

/**
 * @brief Boots the physical frame pool from config (max-overall-mem / mem-per-frame).
 * @param maxOverallMemory Total simulated main memory in bytes.
 * @param memoryPerFrame Frame (page) size in bytes.
 * @note Pre-allocates all frames and resets the backing store file.
 */
void MemoryManager::Initialize(int maxOverallMemory, int memoryPerFrame) {
	MemoryManager* manager = GetInstance();
	manager->totalMemoryBytes = static_cast<size_t>(maxOverallMemory);

	if (memoryPerFrame <= 0) {
		memoryPerFrame = 16;
	}

	manager->frameSizeBytes = static_cast<size_t>(memoryPerFrame);
	manager->frameCount = manager->totalMemoryBytes / manager->frameSizeBytes;
	manager->frameClock = 0;
	manager->pagesPagedIn = 0;
	manager->pagesPagedOut = 0;
	manager->pageTables.clear();
	manager->residentPages.clear();
	manager->instructionRetryPending.clear();
	manager->frames.assign(
		manager->frameCount,
		PageFrame{ false, -1, 0, 0, std::vector<uint8_t>(manager->frameSizeBytes, 0) });

	BackingStore::ResetStore();
}

/**
 * @brief Destroys the singleton memory manager instance.
 */
void MemoryManager::Destroy() {
	delete instance;
	instance = nullptr;
}

MemoryManager::MemoryManager()
	: totalMemoryBytes(0),
	  frameSizeBytes(16),
	  frameCount(0),
	  frameClock(0),
	  pagesPagedIn(0),
	  pagesPagedOut(0) {
}

/**
 * @brief Counts how many physical frames currently hold a resident page.
 * @returns Number of occupied frames (caller must hold memoryMutex).
 */
size_t MemoryManager::CountOccupiedFramesLocked() const {
	size_t occupiedFrames = 0;
	for (const PageFrame& frame : frames) {
		if (frame.occupied) {
			++occupiedFrames;
		}
	}
	return occupiedFrames;
}

/**
 * @brief Converts a process virtual memory size into a page count.
 * @param memoryBytes Process virtual memory allocation in bytes.
 * @returns Number of virtual pages (memoryBytes / frameSizeBytes).
 */
size_t MemoryManager::ComputePageCount(size_t memoryBytes) const {
	if (frameSizeBytes == 0 || memoryBytes == 0) {
		return 0;
	}

	return memoryBytes / frameSizeBytes;
}

/**
 * @brief Looks up a process virtual page table entry.
 * @param processId Process to query.
 * @returns Pointer to page table, or nullptr if the process is not registered.
 */
const MemoryManager::ProcessPageTable* MemoryManager::FindProcessPageTableLocked(int processId) const {
	const auto tableIt = pageTables.find(processId);
	if (tableIt == pageTables.end()) {
		return nullptr;
	}

	return &tableIt->second;
}

/**
 * @brief Checks whether a byte range is a legal emulated virtual address.
 * @param processId Process performing the access.
 * @param address Starting virtual address.
 * @param byteCount Number of bytes to access.
 * @returns True if [address, address + byteCount) fits in the 64 KiB virtual address space.
 * @note Addresses above the process allocation size (e.g. 0x500 on a 64-byte process) are valid
 *       and page-fault in on demand; only out-of-range uint16 accesses are violations.
 */
bool MemoryManager::IsProcessAddressValidLocked(int processId, uint16_t address, size_t byteCount) const {
	const ProcessPageTable* pageTable = FindProcessPageTableLocked(processId);
	if (pageTable == nullptr || byteCount == 0) {
		return false;
	}

	const size_t endAddress = static_cast<size_t>(address) + byteCount;
	return endAddress <= MAX_PROCESS_VIRTUAL_BYTES;
}

/**
 * @brief Grows the process page table so a virtual page can be demand-paged.
 * @param processId Owner process.
 * @param virtualPage Virtual page index being accessed.
 * @returns False when the page index exceeds the 64 KiB virtual address space.
 */
bool MemoryManager::EnsureVirtualPageMappedLocked(int processId, uint32_t virtualPage) {
	if (frameSizeBytes == 0) {
		return false;
	}

	const auto tableIt = pageTables.find(processId);
	if (tableIt == pageTables.end()) {
		return false;
	}

	const size_t maxPageCount = MAX_PROCESS_VIRTUAL_BYTES / frameSizeBytes;
	if (static_cast<size_t>(virtualPage) >= maxPageCount) {
		return false;
	}

	const size_t requiredPageCount = static_cast<size_t>(virtualPage) + 1;
	if (tableIt->second.pageCount < requiredPageCount) {
		tableIt->second.pageCount = requiredPageCount;
	}

	return true;
}

/**
 * @brief Creates a virtual page table only (no frames allocated yet — demand paging).
 * @param processId Process id to register.
 * @param memoryBytes Virtual memory size for the process.
 */
void MemoryManager::RegisterProcessLocked(int processId, size_t memoryBytes) {
	if (processId < 0 || memoryBytes == 0 || frameSizeBytes == 0) {
		return;
	}

	const size_t pageCount = ComputePageCount(memoryBytes); 
	if (pageCount == 0) {
		return;
	}

	ProcessPageTable pageTable;
	pageTable.memoryBytes = memoryBytes;
	pageTable.pageCount = pageCount;
	pageTables[processId] = pageTable;
}

/**
 * @brief Tears down a process page table, resident frames, and backing-store pages.
 * @param processId Process to unregister.
 */
void MemoryManager::UnregisterProcessLocked(int processId) {
	ReleaseProcessResidentPagesLocked(processId);
	pageTables.erase(processId);
	instructionRetryPending.erase(processId);
}

/**
 * @brief Clears the page-fault retry flag before an instruction runs.
 * @param processId Process about to execute one instruction attempt.
 */
void MemoryManager::ResetPageFaultRetryFlag(int processId) {
	std::lock_guard<std::mutex> lock(memoryMutex);
	instructionRetryPending[processId] = false;
}

/**
 * @brief Returns whether the last instruction attempt faulted in pages and must retry.
 * @param processId Process that just attempted an instruction.
 * @returns True once if a page fault occurred; clears the flag after reading.
 */
bool MemoryManager::ConsumePageFaultRetryFlag(int processId) {
	std::lock_guard<std::mutex> lock(memoryMutex);
	const auto retryIt = instructionRetryPending.find(processId);
	if (retryIt == instructionRetryPending.end() || !retryIt->second) {
		return false;
	}
	retryIt->second = false;
	return true;
}

/**
 * @brief Checks if a virtual page is currently mapped to a physical frame.
 * @param processId Owner process.
 * @param virtualPage Virtual page index.
 * @returns True if the page is resident in RAM.
 */
bool MemoryManager::IsPageResidentLocked(int processId, uint32_t virtualPage) const {
	return GetResidentFrameIndexLocked(processId, virtualPage).has_value();
}

/**
 * @brief Resolves virtual page to physical frame index via the resident page map.
 * @param processId Owner process.
 * @param virtualPage Virtual page index.
 * @returns Frame index if resident; std::nullopt otherwise.
 */
std::optional<size_t> MemoryManager::GetResidentFrameIndexLocked(int processId, uint32_t virtualPage) const {
	const auto processResidentIt = residentPages.find(processId);
	if (processResidentIt == residentPages.end()) {
		return std::nullopt;
	}

	const auto frameIt = processResidentIt->second.find(virtualPage);
	if (frameIt == processResidentIt->second.end() || frameIt->second >= frames.size()) {
		return std::nullopt;
	}

	return frameIt->second;
}

/**
 * @brief Demand paging gate: ensures all pages touched by an access are resident before read/write.
 * @param processId Process performing the memory access.
 * @param address Starting virtual address.
 * @param byteCount Number of bytes the instruction will read or write.
 * @returns Success when pages are ready; PageFaultRetry when pages were just loaded (retry instruction);
 *          AccessViolation when the address is out of range or loading fails.
 * @note Does not perform the actual read/write — only resolves page faults for the address range.
 */
MemoryAccessResult MemoryManager::EnsureResidentPagesForAccessLocked(int processId, uint16_t address, size_t byteCount) {
	if (byteCount == 0 || frameSizeBytes == 0 || !IsProcessAddressValidLocked(processId, address, byteCount)) {
		return MemoryAccessResult::AccessViolation;
	}

	const uint32_t firstPage = static_cast<uint32_t>(address / frameSizeBytes);
	const uint32_t lastPage = static_cast<uint32_t>((address + byteCount - 1) / frameSizeBytes);

	bool pageFaultOccurred = false;
	for (uint32_t virtualPage = firstPage; virtualPage <= lastPage; ++virtualPage) {
		if (IsPageResidentLocked(processId, virtualPage)) {
			continue;
		}

		if (!LoadPageOnDemandLocked(processId, virtualPage)) {
			return MemoryAccessResult::AccessViolation;
		}
		pageFaultOccurred = true;
	}

	if (pageFaultOccurred) {
		instructionRetryPending[processId] = true;
		return MemoryAccessResult::PageFaultRetry;
	}

	return MemoryAccessResult::Success;
}

void MemoryManager::RegisterProcess(int processId, size_t memoryBytes) {
	std::lock_guard<std::mutex> lock(memoryMutex);
	RegisterProcessLocked(processId, memoryBytes);
}

void MemoryManager::UnregisterProcess(int processId) {
	std::lock_guard<std::mutex> lock(memoryMutex);
	UnregisterProcessLocked(processId);
}

bool MemoryManager::IsProcessRegistered(int processId) const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return pageTables.find(processId) != pageTables.end();
}

size_t MemoryManager::GetPageCountForProcess(size_t memoryBytes) const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return ComputePageCount(memoryBytes);
}

size_t MemoryManager::GetRegisteredProcessMemoryBytes(int processId) const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	const ProcessPageTable* pageTable = FindProcessPageTableLocked(processId);
	if (pageTable == nullptr) {
		return 0;
	}

	return pageTable->memoryBytes;
}

/**
 * @brief Called when a process finishes: clears its virtual mapping and frees its frames.
 * @param process Process being removed from the scheduler.
 */
void MemoryManager::ReleaseProcessMemory(const std::shared_ptr<Process>& process) {
	if (process == nullptr) {
		return;
	}

	std::lock_guard<std::mutex> lock(memoryMutex);
	process->SetMemoryAddress(nullptr);
	UnregisterProcessLocked(process->GetID());
}

size_t MemoryManager::GetUsedMemory() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return CountOccupiedFramesLocked() * frameSizeBytes;
}

size_t MemoryManager::GetTotalMemory() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return totalMemoryBytes;
}

size_t MemoryManager::GetFreeMemory() const {
	return GetTotalMemory() - GetUsedMemory();
}

float MemoryManager::GetMemoryUtilization() const {
	const size_t totalMemory = GetTotalMemory();
	if (totalMemory == 0) {
		return 0.0f;
	}

	return (static_cast<float>(GetUsedMemory()) / static_cast<float>(totalMemory)) * 100.0f;
}

std::string MemoryManager::GetVisualizedMemory() const {
	return GetFrameMapVisualization();
}

std::string MemoryManager::GetFrameMapVisualization() const {
	std::lock_guard<std::mutex> lock(memoryMutex);

	std::string frameMap;
	frameMap.reserve(frames.size());
	for (const PageFrame& frame : frames) {
		frameMap.push_back(frame.occupied ? '#' : '.');
	}
	return frameMap;
}

size_t MemoryManager::GetFrameCount() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return frameCount;
}

size_t MemoryManager::GetFrameSizeBytes() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return frameSizeBytes;
}

size_t MemoryManager::GetOccupiedFrameCount() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return CountOccupiedFramesLocked();
}

size_t MemoryManager::GetFreeFrameCount() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	const size_t occupiedFrames = CountOccupiedFramesLocked();
	if (frameCount < occupiedFrames) {
		return 0;
	}
	return frameCount - occupiedFrames;
}

size_t MemoryManager::GetResidentPageCount(int processId) const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	const auto residentIt = residentPages.find(processId);
	if (residentIt == residentPages.end()) {
		return 0;
	}
	return residentIt->second.size();
}

std::string MemoryManager::GetMemoryStats() const {
	return MemoryLogger::PrintMemoryStats();
}

std::string MemoryManager::GetVirtualMemoryStats() const {
	return MemoryLogger::PrintVirtualMemoryStats();
}

std::optional<size_t> MemoryManager::GetAddressOffset(const void* ptr) const {
	(void)ptr;
	return std::nullopt;
}

/**
 * @brief Reads a 16-bit value from process virtual memory (demand paging + resident access).
 * @param process Process performing the READ instruction.
 * @param address Virtual byte address.
 * @param outValue Output value on Success.
 * @returns PageFaultRetry if pages were faulted in (instruction must retry before reading).
 */
MemoryAccessResult MemoryManager::ReadProcessMemory(const std::shared_ptr<Process>& process, uint16_t address, uint16_t& outValue) {
	if (process == nullptr) {
		return MemoryAccessResult::AccessViolation;
	}

	std::lock_guard<std::mutex> lock(memoryMutex);

	const MemoryAccessResult prepareResult = EnsureResidentPagesForAccessLocked(process->GetID(), address, sizeof(uint16_t));
	if (prepareResult != MemoryAccessResult::Success) {
		return prepareResult;
	}

	uint8_t lowByte = 0;
	uint8_t highByte = 0;
	if (!ReadByteFromResidentFrameLocked(process->GetID(), address, lowByte)) {
		return MemoryAccessResult::AccessViolation;
	}
	if (!ReadByteFromResidentFrameLocked(process->GetID(), static_cast<uint16_t>(address + 1), highByte)) {
		return MemoryAccessResult::AccessViolation;
	}

	outValue = static_cast<uint16_t>(static_cast<uint16_t>(highByte) << 8 | lowByte);
	return MemoryAccessResult::Success;
}

/**
 * @brief Writes a 16-bit value to process virtual memory (demand paging + resident access).
 * @param process Process performing the WRITE instruction.
 * @param address Virtual byte address.
 * @param value Value to store.
 * @returns PageFaultRetry if pages were faulted in (instruction must retry before writing).
 */
MemoryAccessResult MemoryManager::WriteProcessMemory(const std::shared_ptr<Process>& process, uint16_t address, uint16_t value) {
	if (process == nullptr) {
		return MemoryAccessResult::AccessViolation;
	}

	std::lock_guard<std::mutex> lock(memoryMutex);

	const MemoryAccessResult prepareResult = EnsureResidentPagesForAccessLocked(process->GetID(), address, sizeof(uint16_t));
	if (prepareResult != MemoryAccessResult::Success) {
		return prepareResult;
	}

	const uint8_t lowByte = static_cast<uint8_t>(value & 0xFF);
	const uint8_t highByte = static_cast<uint8_t>((value >> 8) & 0xFF);
	if (!WriteByteToResidentFrameLocked(process->GetID(), address, lowByte)) {
		return MemoryAccessResult::AccessViolation;
	}
	if (!WriteByteToResidentFrameLocked(process->GetID(), static_cast<uint16_t>(address + 1), highByte)) {
		return MemoryAccessResult::AccessViolation;
	}

	return MemoryAccessResult::Success;
}

size_t MemoryManager::GetPagesPagedIn() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return pagesPagedIn;
}

size_t MemoryManager::GetPagesPagedOut() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return pagesPagedOut;
}

/**
 * @brief Handles a page fault: assigns a frame (evicting if needed) and maps the virtual page.
 * @param processId Process that referenced a non-resident page.
 * @param virtualPage Virtual page index to load.
 * @returns True if the page is now resident; false on invalid page or missing page table.
 * @note Uses FindEmptyFrameIndexLocked first; evicts FIFO victim when physical memory is full.
 */
bool MemoryManager::LoadPageOnDemandLocked(int processId, uint32_t virtualPage) {
	if (frameSizeBytes == 0 || frameCount == 0) {
		return false;
	}

	if (!EnsureVirtualPageMappedLocked(processId, virtualPage)) {
		return false;
	}

	if (IsPageResidentLocked(processId, virtualPage)) {
		return true;
	}

	size_t frameIndex = FindEmptyFrameIndexLocked();
	if (frameIndex >= frameCount) {
		frameIndex = SelectFifoVictimFrameLocked();
		PageOutFrameLocked(frameIndex);
	}

	PageInFrameLocked(processId, virtualPage, frameIndex);
	return true;
}

/**
 * @brief Finds the first unoccupied physical frame.
 * @returns Frame index, or frameCount if no empty frame exists (memory is full).
 */
size_t MemoryManager::FindEmptyFrameIndexLocked() const {
	for (size_t index = 0; index < frames.size(); ++index) {
		if (!frames[index].occupied) {
			return index;
		}
	}
	return frameCount;
}

/**
 * @brief Page-out: evicts a resident frame to the backing store and frees the physical frame.
 * @param frameIndex Physical frame to evict.
 * @note Increments pagesPagedOut and updates residentPages for the victim process.
 */
void MemoryManager::PageOutFrameLocked(size_t frameIndex) {
	if (frameIndex >= frames.size() || !frames[frameIndex].occupied) {
		return;
	}

	const PageFrame victim = frames[frameIndex];
	BackingStore::StoreEvictedPage(victim.processId, victim.virtualPage, victim.bytes);

	auto victimProcessIt = residentPages.find(victim.processId);
	if (victimProcessIt != residentPages.end()) {
		victimProcessIt->second.erase(victim.virtualPage);
		if (victimProcessIt->second.empty()) {
			residentPages.erase(victimProcessIt);
		}
	}

	ResetPhysicalFrameLocked(frameIndex);
	++pagesPagedOut;
}

/**
 * @brief Page-in: maps a virtual page into a physical frame (from backing store or zero-filled).
 * @param processId Owner process.
 * @param virtualPage Virtual page being loaded.
 * @param frameIndex Target physical frame (must be free or already reset).
 * @note Increments pagesPagedIn. Uses BackingStore::LoadStoredPage when the page was evicted before.
 */
void MemoryManager::PageInFrameLocked(int processId, uint32_t virtualPage, size_t frameIndex) {
	if (frameIndex >= frames.size()) {
		return;
	}

	PageFrame& frame = frames[frameIndex];
	frame.occupied = true;
	frame.processId = processId;
	frame.virtualPage = virtualPage;
	frame.loadedAt = ++frameClock;

	std::vector<uint8_t> storedPage;
	if (BackingStore::LoadStoredPage(processId, virtualPage, storedPage, frameSizeBytes)) {
		frame.bytes = std::move(storedPage);
	} else {
		std::fill(frame.bytes.begin(), frame.bytes.end(), 0);
	}

	residentPages[processId][virtualPage] = frameIndex;
	++pagesPagedIn;
}

/**
 * @brief Marks a physical frame as free and clears its metadata and byte contents.
 * @param frameIndex Frame to reset.
 */
void MemoryManager::ResetPhysicalFrameLocked(size_t frameIndex) {
	if (frameIndex >= frames.size()) {
		return;
	}

	PageFrame& frame = frames[frameIndex];
	frame.occupied = false;
	frame.processId = -1;
	frame.virtualPage = 0;
	frame.loadedAt = 0;
	std::fill(frame.bytes.begin(), frame.bytes.end(), 0);
}

/**
 * @brief Reads one byte from an already-resident page (no page fault handling).
 * @param processId Owner process.
 * @param address Virtual byte address.
 * @param outByte Output byte on success.
 * @returns False if the page is not resident (caller should have faulted it in first).
 */
bool MemoryManager::ReadByteFromResidentFrameLocked(int processId, uint16_t address, uint8_t& outByte) const {
	if (frameSizeBytes == 0) {
		return false;
	}

	const std::optional<size_t> frameIndex = GetResidentFrameIndexLocked(
		processId, static_cast<uint32_t>(address / frameSizeBytes));
	if (!frameIndex.has_value()) {
		return false;
	}

	outByte = frames[*frameIndex].bytes[address % frameSizeBytes];
	return true;
}

/**
 * @brief Writes one byte to an already-resident page (no page fault handling).
 * @param processId Owner process.
 * @param address Virtual byte address.
 * @param value Byte to write.
 * @returns False if the page is not resident (caller should have faulted it in first).
 */
bool MemoryManager::WriteByteToResidentFrameLocked(int processId, uint16_t address, uint8_t value) {
	if (frameSizeBytes == 0) {
		return false;
	}

	const std::optional<size_t> frameIndex = GetResidentFrameIndexLocked(
		processId, static_cast<uint32_t>(address / frameSizeBytes));
	if (!frameIndex.has_value()) {
		return false;
	}

	frames[*frameIndex].bytes[address % frameSizeBytes] = value;
	return true;
}

/**
 * @brief FIFO page replacement: selects the frame loaded earliest (lowest loadedAt clock).
 * @returns Index of the victim frame to evict.
 */
size_t MemoryManager::SelectFifoVictimFrameLocked() const {
	size_t victimIndex = 0;
	uint64_t oldestClock = frames[0].loadedAt;

	for (size_t index = 1; index < frames.size(); ++index) {
		if (frames[index].loadedAt < oldestClock) {
			oldestClock = frames[index].loadedAt;
			victimIndex = index;
		}
	}

	return victimIndex;
}

/**
 * @brief Frees all resident frames and backing-store pages owned by one process.
 * @param processId Process exiting or being unregistered.
 */
void MemoryManager::ReleaseProcessResidentPagesLocked(int processId) {
	const auto residentIt = residentPages.find(processId);
	if (residentIt != residentPages.end()) {
		for (const auto& pageEntry : residentIt->second) {
			ResetPhysicalFrameLocked(pageEntry.second);
		}
		residentPages.erase(residentIt);
	}

	BackingStore::RemovePagesForProcess(processId);
}
