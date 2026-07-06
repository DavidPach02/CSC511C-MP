#include "FlatMemoryAllocator.h"
#include <iostream>

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize) : maximumSize(maximumSize), allocatedSize(0) {
	memory.resize(maximumSize);
	InitializeMemory();
}

FlatMemoryAllocator::~FlatMemoryAllocator() {
	memory.clear();
}

void* FlatMemoryAllocator::Allocate(size_t size) {
	// Find the first available block of memory that can accommodate the requested size
	for (size_t i = 0; i < maximumSize - size + 1; ++i) {
		if (allocationMap.count(i) == 0 && CanAllocateAt(i, size)) {
			AllocateAt(i, size);
			//std::cout << static_cast<void*>(&memory[i]);
			return &memory[i];
		}
	}
	
	// No available block found, return nullptr
	return nullptr;
}

void FlatMemoryAllocator::Deallocate(void* ptr) {
	size_t index = static_cast<char*>(ptr) - &memory[0];
	if (allocationMap[index]) {
		DeallocateAt(index);
	}
}

std::string FlatMemoryAllocator::GetVisualizedMemory() {
	return std::string(memory.begin(), memory.end());
}

void FlatMemoryAllocator::InitializeMemory() {
	std::fill(memory.begin(), memory.end(), '.'); // '.' represents free memory
	/*for (size_t i = 0; i < maximumSize; ++i) {
		allocationMap[i] = false;
	}*/
}

bool FlatMemoryAllocator::CanAllocateAt(size_t index, size_t size) const {
	return (index + size <= maximumSize);
}

void FlatMemoryAllocator::AllocateAt(size_t index, size_t size) {
	std::fill(memory.begin() + index, memory.begin() + index + size, '#'); // '#' represents allocated memory

	allocationMap[index] = size;
	allocatedSize += size;
}

void FlatMemoryAllocator::DeallocateAt(size_t index) {
	// 1. Look up exactly how big this allocation was
	size_t size = allocationMap[index];

	// 2. Clear the visual memory back to periods for the entire block
	std::fill(memory.begin() + index, memory.begin() + index + size, '.');

	// 3. Erase the block record from your ledger
	allocationMap.erase(index);

	allocatedSize -= size;
}