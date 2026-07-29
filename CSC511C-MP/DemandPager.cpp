#include "DemandPager.h"
#include "BackingStore.h"

#include <algorithm>
#include <optional>

DemandPager::DemandPager(
	std::vector<PageFrame>& framesRef,
	std::unordered_map<int, ProcessPageTable>& pageTablesRef,
	std::unordered_map<int, std::unordered_map<uint32_t, size_t>>& residentPagesRef,
	FIFOPageReplacement& pageReplacementRef,
	size_t& pagesPagedInRef,
	size_t& pagesPagedOutRef)
	: frames(framesRef),
	  pageTables(pageTablesRef),
	  residentPages(residentPagesRef),
	  pageReplacement(pageReplacementRef),
	  pagesPagedIn(pagesPagedInRef),
	  pagesPagedOut(pagesPagedOutRef),
	  frameSizeBytes(16),
	  frameCount(0),
	  frameClock(0) {
}

void DemandPager::Reset(size_t newFrameCount, size_t newFrameSizeBytes) {
	frameCount = newFrameCount;
	frameSizeBytes = newFrameSizeBytes;
	frameClock = 0;
	pageTables.clear();
	residentPages.clear();
	frames.assign(
		frameCount,
		PageFrame{ false, -1, 0, 0, std::vector<uint8_t>(frameSizeBytes, 0) });
	pageReplacement.Reset(frameCount);
	BackingStore::ResetStore();
}

size_t DemandPager::GetFrameSizeBytes() const {
	return frameSizeBytes;
}

size_t DemandPager::GetFrameCount() const {
	return frameCount;
}

size_t DemandPager::ComputePageCount(size_t memoryBytes) const {
	if (frameSizeBytes == 0 || memoryBytes == 0) {
		return 0;
	}
	return memoryBytes / frameSizeBytes;
}

const ProcessPageTable* DemandPager::FindPageTable(int processId) const {
	const auto tableIt = pageTables.find(processId);
	if (tableIt == pageTables.end()) {
		return nullptr;
	}
	return &tableIt->second;
}

void DemandPager::RegisterProcess(int processId, size_t memoryBytes) {
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

bool DemandPager::IsAddressValid(int processId, uint16_t address, size_t byteCount) const {
	if (FindPageTable(processId) == nullptr || byteCount == 0) {
		return false;
	}

	const size_t endAddress = static_cast<size_t>(address) + byteCount;
	return endAddress <= MAX_PROCESS_VIRTUAL_BYTES;
}

bool DemandPager::EnsureVirtualPageMapped(int processId, uint32_t virtualPage) {
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

bool DemandPager::IsPageResident(int processId, uint32_t virtualPage) const {
	return GetFrameIndex(processId, virtualPage).has_value();
}

std::optional<size_t> DemandPager::GetFrameIndex(int processId, uint32_t virtualPage) const {
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

MemoryAccessResult DemandPager::EnsurePagesResident(int processId, uint16_t address, size_t byteCount) {
	if (byteCount == 0 || frameSizeBytes == 0 || !IsAddressValid(processId, address, byteCount)) {
		return MemoryAccessResult::AccessViolation;
	}

	const uint32_t firstPage = static_cast<uint32_t>(address / frameSizeBytes);
	const uint32_t lastPage = static_cast<uint32_t>((address + byteCount - 1) / frameSizeBytes);

	bool pageFaultOccurred = false;
	for (uint32_t virtualPage = firstPage; virtualPage <= lastPage; ++virtualPage) {
		if (IsPageResident(processId, virtualPage)) {
			continue;
		}

		if (!LoadPage(processId, virtualPage)) {
			return MemoryAccessResult::AccessViolation;
		}
		pageFaultOccurred = true;
	}

	return pageFaultOccurred ? MemoryAccessResult::PageFaultRetry : MemoryAccessResult::Success;
}

bool DemandPager::ReadByte(int processId, uint16_t address, uint8_t& outByte) const {
	if (frameSizeBytes == 0) {
		return false;
	}

	const std::optional<size_t> frameIndex = GetFrameIndex(
		processId, static_cast<uint32_t>(address / frameSizeBytes));
	if (!frameIndex.has_value()) {
		return false;
	}

	outByte = frames[*frameIndex].bytes[address % frameSizeBytes];
	return true;
}

bool DemandPager::WriteByte(int processId, uint16_t address, uint8_t value) {
	if (frameSizeBytes == 0) {
		return false;
	}

	const std::optional<size_t> frameIndex = GetFrameIndex(
		processId, static_cast<uint32_t>(address / frameSizeBytes));
	if (!frameIndex.has_value()) {
		return false;
	}

	frames[*frameIndex].bytes[address % frameSizeBytes] = value;
	return true;
}

bool DemandPager::LoadPage(int processId, uint32_t virtualPage) {
	if (frameSizeBytes == 0 || frameCount == 0) {
		return false;
	}

	if (!EnsureVirtualPageMapped(processId, virtualPage)) {
		return false;
	}

	if (IsPageResident(processId, virtualPage)) {
		return true;
	}

	const std::optional<size_t> freeFrameIndex = FindFreeFrameIndex();
	size_t frameIndex = 0;
	if (freeFrameIndex.has_value()) {
		frameIndex = *freeFrameIndex;
	} else {
		frameIndex = pageReplacement.SelectVictimFrame();
		PageOut(frameIndex);
	}

	PageIn(processId, virtualPage, frameIndex);
	return true;
}

std::optional<size_t> DemandPager::FindFreeFrameIndex() const {
	for (size_t index = 0; index < frames.size(); ++index) {
		if (!frames[index].occupied) {
			return index;
		}
	}
	return std::nullopt;
}

void DemandPager::PageOut(size_t frameIndex) {
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

	pageReplacement.RecordFrameEviction(frameIndex);
	ResetFrame(frameIndex);
	++pagesPagedOut;
}

void DemandPager::PageIn(int processId, uint32_t virtualPage, size_t frameIndex) {
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
	pageReplacement.RecordFrameLoad(frameIndex);
	++pagesPagedIn;
}

void DemandPager::ResetFrame(size_t frameIndex) {
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

void DemandPager::ReleaseProcessPages(int processId) {
	const auto residentIt = residentPages.find(processId);
	if (residentIt != residentPages.end()) {
		for (const auto& pageEntry : residentIt->second) {
			pageReplacement.RecordFrameEviction(pageEntry.second);
			ResetFrame(pageEntry.second);
		}
		residentPages.erase(residentIt);
	}

	pageTables.erase(processId);
	BackingStore::RemovePagesForProcess(processId);
}
