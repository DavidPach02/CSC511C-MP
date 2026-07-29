#pragma once

#include "DemandPager.h"
#include "FIFOPageReplacement.h"
#include "PagingTypes.h"
#include "Process.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// MO2 memory facade: pre-allocated frame pool, demand paging, and FIFO page replacement.
class MemoryManager {
public:
	static constexpr size_t SYMBOL_TABLE_SEGMENT_BYTES = 64;
	static constexpr uint16_t SYMBOL_TABLE_SEGMENT_END = 64;
	static constexpr size_t MAX_PROCESS_VIRTUAL_BYTES = DemandPager::MAX_PROCESS_VIRTUAL_BYTES;

	static MemoryManager* GetInstance();
	static void Initialize(int maxOverallMemory, int memoryPerFrame = 16);
	static void Destroy();

	void RegisterProcess(int processId, size_t memoryBytes);
	void UnregisterProcess(int processId);
	bool IsProcessRegistered(int processId) const;
	size_t GetPageCountForProcess(size_t memoryBytes) const;
	size_t GetRegisteredProcessMemoryBytes(int processId) const;

	void ReleaseProcessMemory(const std::shared_ptr<Process>& process);

	size_t GetUsedMemory() const;
	size_t GetTotalMemory() const;
	size_t GetFreeMemory() const;
	float GetMemoryUtilization() const;
	std::string GetVisualizedMemory() const;
	std::string GetMemoryStats() const;
	std::string GetVirtualMemoryStats() const;
	std::optional<size_t> GetAddressOffset(const void* ptr) const;

	void ResetPageFaultRetryFlag(int processId);
	bool ConsumePageFaultRetryFlag(int processId);

	MemoryAccessResult ReadProcessMemory(const std::shared_ptr<Process>& process, uint16_t address, uint16_t& outValue);
	MemoryAccessResult WriteProcessMemory(const std::shared_ptr<Process>& process, uint16_t address, uint16_t value);

	size_t GetPagesPagedIn() const;
	size_t GetPagesPagedOut() const;

	size_t GetFrameCount() const;
	size_t GetFrameSizeBytes() const;
	size_t GetOccupiedFrameCount() const;
	size_t GetFreeFrameCount() const;
	size_t GetResidentPageCount(int processId) const;
	std::string GetFrameMapVisualization() const;

private:
	MemoryManager();
	~MemoryManager() = default;
	MemoryManager(const MemoryManager&) = delete;
	MemoryManager& operator=(const MemoryManager&) = delete;

	static MemoryManager* instance;

	mutable std::mutex memoryMutex;

	size_t totalMemoryBytes;
	size_t pagesPagedIn;
	size_t pagesPagedOut;

	std::vector<PageFrame> frames;
	std::unordered_map<int, ProcessPageTable> pageTables;
	std::unordered_map<int, std::unordered_map<uint32_t, size_t>> residentPages;
	std::unordered_map<int, bool> instructionRetryPending;

	FIFOPageReplacement pageReplacement;
	DemandPager demandPager;

	size_t CountOccupiedFramesLocked() const;
};
