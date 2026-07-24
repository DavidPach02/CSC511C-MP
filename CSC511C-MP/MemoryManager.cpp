#include "MemoryManager.h"
#include "MemoryLogger.h"

#include <mutex>

MemoryManager* MemoryManager::instance = nullptr;

MemoryManager* MemoryManager::GetInstance() {
	if (instance == nullptr) {
		instance = new MemoryManager();
	}
	return instance;
}

MemoryManager::MemoryManager()
	: totalMemoryBytes(0),
	  pagesPagedIn(0),
	  pagesPagedOut(0),
	  pageReplacement(0),
	  demandPager(
		  frames,
		  pageTables,
		  residentPages,
		  pageReplacement,
		  pagesPagedIn,
		  pagesPagedOut) {
}

void MemoryManager::Initialize(int maxOverallMemory, int memoryPerFrame) {
	MemoryManager* manager = GetInstance();
	manager->totalMemoryBytes = static_cast<size_t>(maxOverallMemory);

	if (memoryPerFrame <= 0) {
		memoryPerFrame = 16;
	}

	const size_t frameSizeBytes = static_cast<size_t>(memoryPerFrame);
	const size_t frameCount = manager->totalMemoryBytes / frameSizeBytes;

	manager->pagesPagedIn = 0;
	manager->pagesPagedOut = 0;
	manager->instructionRetryPending.clear();
	manager->demandPager.Reset(frameCount, frameSizeBytes); 
}

void MemoryManager::Destroy() {
	delete instance;
	instance = nullptr;
}

size_t MemoryManager::CountOccupiedFramesLocked() const {
	size_t occupiedFrames = 0;
	for (const PageFrame& frame : frames) {
		if (frame.occupied) {
			++occupiedFrames;
		}
	}
	return occupiedFrames;
}

void MemoryManager::ResetPageFaultRetryFlag(int processId) {
	std::lock_guard<std::mutex> lock(memoryMutex);
	instructionRetryPending[processId] = false;
}

bool MemoryManager::ConsumePageFaultRetryFlag(int processId) {
	std::lock_guard<std::mutex> lock(memoryMutex);
	const auto retryIt = instructionRetryPending.find(processId);
	if (retryIt == instructionRetryPending.end() || !retryIt->second) {
		return false;
	}
	retryIt->second = false;
	return true;
}

void MemoryManager::RegisterProcess(int processId, size_t memoryBytes) {
	std::lock_guard<std::mutex> lock(memoryMutex);
	demandPager.RegisterProcess(processId, memoryBytes);
}

void MemoryManager::UnregisterProcess(int processId) {
	std::lock_guard<std::mutex> lock(memoryMutex);
	demandPager.ReleaseProcessPages(processId);
	instructionRetryPending.erase(processId);
}

bool MemoryManager::IsProcessRegistered(int processId) const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return pageTables.find(processId) != pageTables.end();
}

size_t MemoryManager::GetPageCountForProcess(size_t memoryBytes) const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	if (demandPager.GetFrameSizeBytes() == 0 || memoryBytes == 0) {
		return 0;
	}
	return memoryBytes / demandPager.GetFrameSizeBytes();
}

size_t MemoryManager::GetRegisteredProcessMemoryBytes(int processId) const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	const auto tableIt = pageTables.find(processId);
	if (tableIt == pageTables.end()) {
		return 0;
	}
	return tableIt->second.memoryBytes;
}

void MemoryManager::ReleaseProcessMemory(const std::shared_ptr<Process>& process) {
	if (process == nullptr) {
		return;
	}

	std::lock_guard<std::mutex> lock(memoryMutex);
	process->SetMemoryAddress(nullptr);
	demandPager.ReleaseProcessPages(process->GetID());
	instructionRetryPending.erase(process->GetID());
}

size_t MemoryManager::GetUsedMemory() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return CountOccupiedFramesLocked() * demandPager.GetFrameSizeBytes();
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
	return demandPager.GetFrameCount();
}

size_t MemoryManager::GetFrameSizeBytes() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return demandPager.GetFrameSizeBytes();
}

size_t MemoryManager::GetOccupiedFrameCount() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return CountOccupiedFramesLocked();
}

size_t MemoryManager::GetFreeFrameCount() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	const size_t occupiedFrames = CountOccupiedFramesLocked();
	const size_t frameCount = demandPager.GetFrameCount();
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

MemoryAccessResult MemoryManager::ReadProcessMemory(
	const std::shared_ptr<Process>& process,
	uint16_t address,
	uint16_t& outValue) {
	if (process == nullptr) {
		return MemoryAccessResult::AccessViolation;
	}

	std::lock_guard<std::mutex> lock(memoryMutex);

	const MemoryAccessResult prepareResult = demandPager.EnsurePagesResident(
		process->GetID(), address, sizeof(uint16_t));
	if (prepareResult != MemoryAccessResult::Success) {
		if (prepareResult == MemoryAccessResult::PageFaultRetry) {
			instructionRetryPending[process->GetID()] = true;
		}
		return prepareResult;
	}

	uint8_t lowByte = 0;
	uint8_t highByte = 0;
	if (!demandPager.ReadByte(process->GetID(), address, lowByte)) {
		return MemoryAccessResult::AccessViolation;
	}
	if (!demandPager.ReadByte(process->GetID(), static_cast<uint16_t>(address + 1), highByte)) {
		return MemoryAccessResult::AccessViolation;
	}

	outValue = static_cast<uint16_t>(static_cast<uint16_t>(highByte) << 8 | lowByte);
	return MemoryAccessResult::Success;
}

MemoryAccessResult MemoryManager::WriteProcessMemory(
	const std::shared_ptr<Process>& process,
	uint16_t address,
	uint16_t value) {
	if (process == nullptr) {
		return MemoryAccessResult::AccessViolation;
	}

	std::lock_guard<std::mutex> lock(memoryMutex);

	const MemoryAccessResult prepareResult = demandPager.EnsurePagesResident(
		process->GetID(), address, sizeof(uint16_t));
	if (prepareResult != MemoryAccessResult::Success) {
		if (prepareResult == MemoryAccessResult::PageFaultRetry) {
			instructionRetryPending[process->GetID()] = true;
		}
		return prepareResult;
	}

	const uint8_t lowByte = static_cast<uint8_t>(value & 0xFF);
	const uint8_t highByte = static_cast<uint8_t>((value >> 8) & 0xFF);
	if (!demandPager.WriteByte(process->GetID(), address, lowByte)) {
		return MemoryAccessResult::AccessViolation;
	}
	if (!demandPager.WriteByte(process->GetID(), static_cast<uint16_t>(address + 1), highByte)) {
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
