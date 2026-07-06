#pragma once

#include <string>

class IMemoryAllocator {
public:
	virtual void* Allocate(size_t size) = 0;
	virtual void Deallocate(void* ptr) = 0;
	virtual std::string GetVisualizedMemory() = 0;
};