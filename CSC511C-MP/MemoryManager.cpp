#include "MemoryManager.h"
#include "FlatMemoryAllocator.h"

MemoryManager* MemoryManager::instance = nullptr;

MemoryManager* MemoryManager::GetInstance() {
	if (instance == nullptr) {
		instance = new MemoryManager();
	}
	return instance;
}

void MemoryManager::Initialize(int maxOverallMemory) {
	GetInstance()->allocator = std::make_unique<FlatMemoryAllocator>(maxOverallMemory);
}

void MemoryManager::Destroy() {
	delete instance;
	instance = nullptr;
}

MemoryManager::MemoryManager() = default;

bool MemoryManager::TryAllocateForProcess(const std::shared_ptr<Process>& process) {
	if (process == nullptr) {
		return false;
	}

	std::lock_guard<std::mutex> lock(memoryMutex);

	if (process->HasMemoryLoaded()) {
		return true;
	}

	if (allocator == nullptr) {
		return false;
	}

	void* memoryAddress = allocator->Allocate(process->GetMemoryRequired());
	if (memoryAddress == nullptr) {
		return false;
	}

	process->SetMemoryAddress(memoryAddress);
	return true;
}

void MemoryManager::ReleaseProcessMemory(const std::shared_ptr<Process>& process) {
	if (process == nullptr || !process->HasMemoryLoaded()) {
		return;
	}

	std::lock_guard<std::mutex> lock(memoryMutex);

	if (allocator != nullptr) {
		allocator->Deallocate(process->GetMemoryAddress());
	}

	process->SetMemoryAddress(nullptr);
}

size_t MemoryManager::GetUsedMemory() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	if (allocator == nullptr) {
		return 0;
	}
	return allocator->GetAllocatedSize();
}

size_t MemoryManager::GetTotalMemory() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	if (allocator == nullptr) {
		return 0;
	}
	return allocator->GetMaximumSize();
}

std::string MemoryManager::GetVisualizedMemory() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	if (allocator == nullptr) {
		return "";
	}
	return allocator->GetVisualizedMemory();
}
