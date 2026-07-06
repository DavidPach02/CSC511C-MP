#pragma once

#include "IMemoryAllocator.h"
#include <string>
#include <unordered_map>
#include <vector>

class FlatMemoryAllocator : public IMemoryAllocator{
public:
	FlatMemoryAllocator(size_t maximumSize);
	~FlatMemoryAllocator();

	void* Allocate(size_t size) override;
	void Deallocate(void* ptr) override;
	std::string GetVisualizedMemory() override;

private:
	size_t maximumSize;
	size_t allocatedSize;
	std::vector<char> memory;
	std::unordered_map<size_t, size_t> allocationMap;

	void InitializeMemory();
	bool CanAllocateAt(size_t index, size_t size) const;
	void AllocateAt(size_t index, size_t size);
	void DeallocateAt(size_t index);
};

