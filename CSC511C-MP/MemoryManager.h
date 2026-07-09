#pragma once

#include "IMemoryAllocator.h"
#include "Process.h"
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>

// Singleton first-fit memory manager. Allocates mem-per-proc when a process is scheduled;
// memory is held until the process finishes execution.
class MemoryManager {
public:
	static MemoryManager* GetInstance();
	static void Initialize(int maxOverallMemory);
	static void Destroy();

	bool TryAllocateForProcess(const std::shared_ptr<Process>& process);
	void ReleaseProcessMemory(const std::shared_ptr<Process>& process);

	size_t GetUsedMemory() const;
	size_t GetTotalMemory() const;
	std::string GetVisualizedMemory() const;

private:
	MemoryManager();
	~MemoryManager() = default;
	MemoryManager(const MemoryManager&) = delete;
	MemoryManager& operator=(const MemoryManager&) = delete;

	static MemoryManager* instance;

	mutable std::mutex memoryMutex;
	std::unique_ptr<IMemoryAllocator> allocator;
};
