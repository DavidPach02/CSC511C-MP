#pragma once

#include "IMemoryAllocator.h"
#include "Process.h"
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <sstream>

// Singleton first-fit memory manager. Allocates mem-per-proc when a process is scheduled;
// memory is held until the process finishes execution.
class MemoryManager {
public:
	static MemoryManager* GetInstance();
	static void Initialize(int maxOverallMemory, int memoryPerFrame = 16);
	static void Destroy();

	bool TryAllocateForProcess(const std::shared_ptr<Process>& process);
	void ReleaseProcessMemory(const std::shared_ptr<Process>& process);

	size_t GetUsedMemory() const;
	size_t GetTotalMemory() const;
	size_t GetFreeMemory() const;
	float GetMemoryUtilization() const;
	std::string GetVisualizedMemory() const;
	std::string GetMemoryStats() const;
	std::string GetVirtualMemoryStats() const;
	std::optional<size_t> GetAddressOffset(const void* ptr) const;

	bool ReadProcessMemory(const std::shared_ptr<Process>& process, uint16_t address, uint16_t& outValue);
	bool WriteProcessMemory(const std::shared_ptr<Process>& process, uint16_t address, uint16_t value);

	size_t GetPagesPagedIn() const;
	size_t GetPagesPagedOut() const;

private:
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
	std::unique_ptr<IMemoryAllocator> allocator;

	size_t frameSizeBytes;
	size_t frameCount;
	uint64_t frameClock;
	size_t pagesPagedIn;
	size_t pagesPagedOut;
	std::vector<PageFrame> frames;
	std::unordered_map<int, std::unordered_map<uint32_t, size_t>> residentPages;
	std::unordered_map<int, std::unordered_map<uint32_t, std::vector<uint8_t>>> backingStore;

	bool EnsurePageLoadedLocked(int processId, uint32_t virtualPage);
	bool ReadByteLocked(int processId, uint16_t address, uint8_t& outByte);
	bool WriteByteLocked(int processId, uint16_t address, uint8_t value);
	size_t SelectEvictionFrameLocked() const;
	void RemoveProcessPagesLocked(int processId);
};
