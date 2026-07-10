#include "FlatMemoryAllocator.h"

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize)
	: maximumSize(maximumSize), allocatedSize(0) {
	memory.resize(maximumSize);
	InitializeMemory();
}

void* FlatMemoryAllocator::Allocate(size_t size) {
	if (size == 0 || size > maximumSize) {
		return nullptr;
	}

	for (size_t index = 0; index <= maximumSize - size; ++index) {
		if (CanAllocateAt(index, size)) {
			AllocateAt(index, size);
			return &memory[index];
		}
	}

	return nullptr;
}

void FlatMemoryAllocator::Deallocate(void* ptr) {
	if (ptr == nullptr || memory.empty()) {
		return;
	}

	const char* memoryStart = memory.data();
	const char* memoryEnd = memoryStart + memory.size();
	const char* target = static_cast<char*>(ptr);
	if (target < memoryStart || target >= memoryEnd) {
		return;
	}

	const size_t index = static_cast<size_t>(target - memoryStart);
	for (auto iterator = allocationMap.begin(); iterator != allocationMap.end(); ++iterator) {
		const size_t blockStart = iterator->first;
		const size_t blockSize = iterator->second;
		if (index >= blockStart && index < blockStart + blockSize) {
			DeallocateAt(blockStart);
			return;
		}
	}
}

std::string FlatMemoryAllocator::GetVisualizedMemory() {
	return std::string(memory.begin(), memory.end());
}

std::optional<size_t> FlatMemoryAllocator::GetOffsetOfPointer(const void* ptr) const {
	if (ptr == nullptr || memory.empty()) {
		return std::nullopt;
	}

	const char* memoryStart = memory.data();
	const char* memoryEnd = memoryStart + memory.size();
	const char* target = static_cast<const char*>(ptr);

	if (target < memoryStart || target >= memoryEnd) {
		return std::nullopt;
	}

	return static_cast<size_t>(target - memoryStart);
}

size_t FlatMemoryAllocator::GetMaximumSize() const {
	return maximumSize;
}

size_t FlatMemoryAllocator::GetAllocatedSize() const {
	return allocatedSize;
}

void FlatMemoryAllocator::InitializeMemory() {
	std::fill(memory.begin(), memory.end(), '.');
}

bool FlatMemoryAllocator::CanAllocateAt(size_t index, size_t size) const {
	if (index + size > maximumSize) {
		return false;
	}

	const size_t requestEnd = index + size;
	for (const auto& allocationEntry : allocationMap) {
		const size_t blockStart = allocationEntry.first;
		const size_t blockEnd = blockStart + allocationEntry.second;
		const bool separated = requestEnd <= blockStart || index >= blockEnd;
		if (!separated) {
			return false;
		}
	}

	return true;
}

void FlatMemoryAllocator::AllocateAt(size_t index, size_t size) {
	std::fill(memory.begin() + index, memory.begin() + index + size, '#');
	allocationMap[index] = size;
	allocatedSize += size;
}

void FlatMemoryAllocator::DeallocateAt(size_t index) {
	const size_t size = allocationMap.at(index);
	std::fill(memory.begin() + index, memory.begin() + index + size, '.');
	allocationMap.erase(index);
	allocatedSize -= size;
}
