#pragma once

#include <cstddef>
#include <string>

class IMemoryAllocator {
public:
	virtual ~IMemoryAllocator() = default;

	virtual void* Allocate(size_t size) = 0;
	virtual void Deallocate(void* ptr) = 0;
	virtual std::string GetVisualizedMemory() = 0;
	virtual size_t GetMaximumSize() const = 0;
	virtual size_t GetAllocatedSize() const = 0;
};
