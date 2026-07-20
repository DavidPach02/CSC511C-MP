#include "MemoryManager.h"
#include "FlatMemoryAllocator.h"
#include "TextFormatter.h"
#include "MemoryLogger.h"

#include <algorithm>

MemoryManager* MemoryManager::instance = nullptr;

MemoryManager* MemoryManager::GetInstance() {
	if (instance == nullptr) {
		instance = new MemoryManager();
	}
	return instance;
}

void MemoryManager::Initialize(int maxOverallMemory, int memoryPerFrame) {
	MemoryManager* manager = GetInstance();
	manager->allocator = std::make_unique<FlatMemoryAllocator>(maxOverallMemory);

	if (memoryPerFrame <= 0) {
		memoryPerFrame = 16;
	}

	manager->frameSizeBytes = static_cast<size_t>(memoryPerFrame);
	manager->frameCount = static_cast<size_t>(maxOverallMemory) / manager->frameSizeBytes;
	manager->frameClock = 0;
	manager->pagesPagedIn = 0;
	manager->pagesPagedOut = 0;
	manager->residentPages.clear();
	manager->backingStore.clear();
	manager->frames.assign(manager->frameCount, PageFrame{ false, -1, 0, 0, std::vector<uint8_t>(manager->frameSizeBytes, 0) });
}

void MemoryManager::Destroy() {
	delete instance;
	instance = nullptr;
}

MemoryManager::MemoryManager()
	: frameSizeBytes(16),
	  frameCount(0),
	  frameClock(0),
	  pagesPagedIn(0),
	  pagesPagedOut(0) {
}

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

	RemoveProcessPagesLocked(process->GetID());

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

size_t MemoryManager::GetFreeMemory() const {
	return GetTotalMemory() - GetUsedMemory();
}

float MemoryManager::GetMemoryUtilization() const {
	return (GetUsedMemory() / GetTotalMemory()) * 100;
}

std::string MemoryManager::GetVisualizedMemory() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	if (allocator == nullptr) {
		return "";
	}

	return allocator->GetVisualizedMemory();		
}

std::string MemoryManager::GetMemoryStats() const {
	return MemoryLogger::PrintMemoryStats();
}

std::string MemoryManager::GetVirtualMemoryStats() const {
	return MemoryLogger::PrintVirtualMemoryStats();
}

std::optional<size_t> MemoryManager::GetAddressOffset(const void* ptr) const {
	if (ptr == nullptr) {
		return std::nullopt;
	}

	std::lock_guard<std::mutex> lock(memoryMutex);
	if (allocator == nullptr) {
		return std::nullopt;
	}

	FlatMemoryAllocator* flatAllocator = dynamic_cast<FlatMemoryAllocator*>(allocator.get());
	if (flatAllocator == nullptr) {
		return std::nullopt;
	}

	return flatAllocator->GetOffsetOfPointer(ptr);
}

bool MemoryManager::ReadProcessMemory(const std::shared_ptr<Process>& process, uint16_t address, uint16_t& outValue) {
	if (process == nullptr || address == 0xFFFF) {
		return false;
	}

	std::lock_guard<std::mutex> lock(memoryMutex);

	uint8_t lowByte = 0;
	uint8_t highByte = 0;
	if (!ReadByteLocked(process->GetID(), address, lowByte)) {
		return false;
	}
	if (!ReadByteLocked(process->GetID(), static_cast<uint16_t>(address + 1), highByte)) {
		return false;
	}

	outValue = static_cast<uint16_t>(static_cast<uint16_t>(highByte) << 8 | lowByte);
	return true;
}

bool MemoryManager::WriteProcessMemory(const std::shared_ptr<Process>& process, uint16_t address, uint16_t value) {
	if (process == nullptr || address == 0xFFFF) {
		return false;
	}

	std::lock_guard<std::mutex> lock(memoryMutex);

	const uint8_t lowByte = static_cast<uint8_t>(value & 0xFF);
	const uint8_t highByte = static_cast<uint8_t>((value >> 8) & 0xFF);
	if (!WriteByteLocked(process->GetID(), address, lowByte)) {
		return false;
	}
	if (!WriteByteLocked(process->GetID(), static_cast<uint16_t>(address + 1), highByte)) {
		return false;
	}

	return true;
}

size_t MemoryManager::GetPagesPagedIn() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return pagesPagedIn;
}

size_t MemoryManager::GetPagesPagedOut() const {
	std::lock_guard<std::mutex> lock(memoryMutex);
	return pagesPagedOut;
}

bool MemoryManager::EnsurePageLoadedLocked(int processId, uint32_t virtualPage) {
	if (frameSizeBytes == 0 || frameCount == 0) {
		return false;
	}

	auto processResidentIt = residentPages.find(processId);
	if (processResidentIt != residentPages.end()) {
		auto pageIt = processResidentIt->second.find(virtualPage);
		if (pageIt != processResidentIt->second.end()) {
			return true;
		}
	}

	size_t frameIndex = frameCount;
	for (size_t index = 0; index < frames.size(); ++index) {
		if (!frames[index].occupied) {
			frameIndex = index;
			break;
		}
	}

	if (frameIndex == frameCount) {
		frameIndex = SelectEvictionFrameLocked();
		if (frameIndex >= frameCount) {
			return false;
		}

		PageFrame& victim = frames[frameIndex];
		if (victim.occupied) {
			backingStore[victim.processId][victim.virtualPage] = victim.bytes;
			auto victimProcessIt = residentPages.find(victim.processId);
			if (victimProcessIt != residentPages.end()) {
				victimProcessIt->second.erase(victim.virtualPage);
				if (victimProcessIt->second.empty()) {
					residentPages.erase(victimProcessIt);
				}
			}
			++pagesPagedOut;
		}
	}

	PageFrame& frame = frames[frameIndex];
	frame.occupied = true;
	frame.processId = processId;
	frame.virtualPage = virtualPage;
	frame.loadedAt = ++frameClock;

	auto processStoreIt = backingStore.find(processId);
	if (processStoreIt != backingStore.end()) {
		auto pageStoreIt = processStoreIt->second.find(virtualPage);
		if (pageStoreIt != processStoreIt->second.end() && pageStoreIt->second.size() == frameSizeBytes) {
			frame.bytes = pageStoreIt->second;
		} else {
			std::fill(frame.bytes.begin(), frame.bytes.end(), 0);
		}
	} else {
		std::fill(frame.bytes.begin(), frame.bytes.end(), 0);
	}

	residentPages[processId][virtualPage] = frameIndex;
	++pagesPagedIn;
	return true;
}

bool MemoryManager::ReadByteLocked(int processId, uint16_t address, uint8_t& outByte) {
	if (frameSizeBytes == 0) {
		return false;
	}

	const uint32_t virtualPage = static_cast<uint32_t>(address / frameSizeBytes);
	const size_t pageOffset = static_cast<size_t>(address % frameSizeBytes);
	if (!EnsurePageLoadedLocked(processId, virtualPage)) {
		return false;
	}

	const auto processPagesIt = residentPages.find(processId);
	if (processPagesIt == residentPages.end()) {
		return false;
	}
	const auto frameIt = processPagesIt->second.find(virtualPage);
	if (frameIt == processPagesIt->second.end() || frameIt->second >= frames.size()) {
		return false;
	}

	outByte = frames[frameIt->second].bytes[pageOffset];
	return true;
}

bool MemoryManager::WriteByteLocked(int processId, uint16_t address, uint8_t value) {
	if (frameSizeBytes == 0) {
		return false;
	}

	const uint32_t virtualPage = static_cast<uint32_t>(address / frameSizeBytes);
	const size_t pageOffset = static_cast<size_t>(address % frameSizeBytes);
	if (!EnsurePageLoadedLocked(processId, virtualPage)) {
		return false;
	}

	const auto processPagesIt = residentPages.find(processId);
	if (processPagesIt == residentPages.end()) {
		return false;
	}
	const auto frameIt = processPagesIt->second.find(virtualPage);
	if (frameIt == processPagesIt->second.end() || frameIt->second >= frames.size()) {
		return false;
	}

	frames[frameIt->second].bytes[pageOffset] = value;
	return true;
}

size_t MemoryManager::SelectEvictionFrameLocked() const {
	if (frames.empty()) {
		return frameCount;
	}

	size_t oldestFrameIndex = frameCount;
	uint64_t oldestClock = 0;
	bool firstFound = false;

	for (size_t index = 0; index < frames.size(); ++index) {
		if (!frames[index].occupied) {
			return index;
		}

		if (!firstFound || frames[index].loadedAt < oldestClock) {
			oldestClock = frames[index].loadedAt;
			oldestFrameIndex = index;
			firstFound = true;
		}
	}

	return oldestFrameIndex;
}

void MemoryManager::RemoveProcessPagesLocked(int processId) {
	auto residentIt = residentPages.find(processId);
	if (residentIt != residentPages.end()) {
		for (const auto& pageEntry : residentIt->second) {
			const size_t frameIndex = pageEntry.second;
			if (frameIndex >= frames.size()) {
				continue;
			}

			PageFrame& frame = frames[frameIndex];
			frame.occupied = false;
			frame.processId = -1;
			frame.virtualPage = 0;
			frame.loadedAt = 0;
			std::fill(frame.bytes.begin(), frame.bytes.end(), 0);
		}
		residentPages.erase(residentIt);
	}

	backingStore.erase(processId);
}
