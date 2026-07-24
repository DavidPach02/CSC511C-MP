#pragma once

#include "FIFOPageReplacement.h"
#include "PagingTypes.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

// Demand paging: load pages on access, evict to backing store when frames are full.
class DemandPager {
public:
	static constexpr size_t MAX_PROCESS_VIRTUAL_BYTES = 1 << 16;

	DemandPager(
		std::vector<PageFrame>& frames,
		std::unordered_map<int, ProcessPageTable>& pageTables,
		std::unordered_map<int, std::unordered_map<uint32_t, size_t>>& residentPages,
		FIFOPageReplacement& pageReplacement,
		size_t& pagesPagedIn,
		size_t& pagesPagedOut);

	void Reset(size_t frameCount, size_t frameSizeBytes);

	size_t GetFrameSizeBytes() const;
	size_t GetFrameCount() const;

	void RegisterProcess(int processId, size_t memoryBytes);
	void ReleaseProcessPages(int processId);

	MemoryAccessResult EnsurePagesResident(int processId, uint16_t address, size_t byteCount);

	bool ReadByte(int processId, uint16_t address, uint8_t& outByte) const;
	bool WriteByte(int processId, uint16_t address, uint8_t value);

private:
	std::vector<PageFrame>& frames;
	std::unordered_map<int, ProcessPageTable>& pageTables;
	std::unordered_map<int, std::unordered_map<uint32_t, size_t>>& residentPages;
	FIFOPageReplacement& pageReplacement;
	size_t& pagesPagedIn;
	size_t& pagesPagedOut;

	size_t frameSizeBytes;
	size_t frameCount;
	uint64_t frameClock;

	size_t ComputePageCount(size_t memoryBytes) const;
	const ProcessPageTable* FindPageTable(int processId) const;
	bool IsAddressValid(int processId, uint16_t address, size_t byteCount) const;
	bool EnsureVirtualPageMapped(int processId, uint32_t virtualPage);
	bool IsPageResident(int processId, uint32_t virtualPage) const;
	std::optional<size_t> GetFrameIndex(int processId, uint32_t virtualPage) const;

	bool LoadPage(int processId, uint32_t virtualPage);
	std::optional<size_t> FindFreeFrameIndex() const;
	void PageOut(size_t frameIndex);
	void PageIn(int processId, uint32_t virtualPage, size_t frameIndex);
	void ResetFrame(size_t frameIndex);
};
