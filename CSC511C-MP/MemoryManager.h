#pragma once

#include "Process.h"
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

enum class MemoryAccessResult {
	Success,
	AccessViolation,
	PageFaultRetry
};

// MO2 memory manager: pre-allocated frame pool, per-process page tables, demand paging.
class MemoryManager {
public:
	static constexpr size_t SYMBOL_TABLE_SEGMENT_BYTES = 64;
	static constexpr uint16_t SYMBOL_TABLE_SEGMENT_END = 64;
	static constexpr size_t MAX_PROCESS_VIRTUAL_BYTES = 1 << 16;

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

	// Clears the page-fault retry flag at the start of each instruction attempt.
	void ResetPageFaultRetryFlag(int processId);
	// Returns true once if a page fault occurred during the last attempt (instruction should retry).
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
	struct ProcessPageTable {
		size_t memoryBytes = 0;
		size_t pageCount = 0;
	};

	MemoryManager();
	~MemoryManager() = default;
	MemoryManager(const MemoryManager&) = delete;
	MemoryManager& operator=(const MemoryManager&) = delete;

	static MemoryManager* instance;

	struct PageFrame {
		bool occupied;
		int processId;
		uint32_t virtualPage;
		uint64_t loadedAt;
		std::vector<uint8_t> bytes;
	};

	mutable std::mutex memoryMutex;

	size_t totalMemoryBytes;
	size_t frameSizeBytes;
	size_t frameCount;
	uint64_t frameClock;
	size_t pagesPagedIn;
	size_t pagesPagedOut;
	std::vector<PageFrame> frames;
	std::unordered_map<int, ProcessPageTable> pageTables;
	std::unordered_map<int, std::unordered_map<uint32_t, size_t>> residentPages;
	std::unordered_map<int, bool> instructionRetryPending;

	size_t CountOccupiedFramesLocked() const;
	size_t ComputePageCount(size_t memoryBytes) const;
	bool IsProcessAddressValidLocked(int processId, uint16_t address, size_t byteCount) const;
	bool EnsureVirtualPageMappedLocked(int processId, uint32_t virtualPage);
	const ProcessPageTable* FindProcessPageTableLocked(int processId) const;
	std::optional<size_t> GetResidentFrameIndexLocked(int processId, uint32_t virtualPage) const;
	bool IsPageResidentLocked(int processId, uint32_t virtualPage) const;
	MemoryAccessResult EnsureResidentPagesForAccessLocked(int processId, uint16_t address, size_t byteCount);

	void RegisterProcessLocked(int processId, size_t memoryBytes);
	void UnregisterProcessLocked(int processId);

	bool LoadPageOnDemandLocked(int processId, uint32_t virtualPage);
	size_t FindEmptyFrameIndexLocked() const;
	size_t SelectFifoVictimFrameLocked() const;
	void PageOutFrameLocked(size_t frameIndex);
	void PageInFrameLocked(int processId, uint32_t virtualPage, size_t frameIndex);
	void ResetPhysicalFrameLocked(size_t frameIndex);
	bool ReadByteFromResidentFrameLocked(int processId, uint16_t address, uint8_t& outByte) const;
	bool WriteByteToResidentFrameLocked(int processId, uint16_t address, uint8_t value);
	void ReleaseProcessResidentPagesLocked(int processId);
};
