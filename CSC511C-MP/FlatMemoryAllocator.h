#pragma once

#include "IMemoryAllocator.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class FlatMemoryAllocator : public IMemoryAllocator {
public:
	explicit FlatMemoryAllocator(size_t maximumSize);
	~FlatMemoryAllocator() override = default;

	void* Allocate(size_t size) override;
	void Deallocate(void* ptr) override;
	std::string GetVisualizedMemory() override;
	std::optional<size_t> GetOffsetOfPointer(const void* ptr) const;

	size_t GetMaximumSize() const override;
	size_t GetAllocatedSize() const override;

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
