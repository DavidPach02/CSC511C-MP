#pragma once

#include <cstdint>
#include <vector>

enum class MemoryAccessResult {
	Success,
	AccessViolation,
	PageFaultRetry
};

// One pre-allocated physical frame in main memory (fixed size = mem-per-frame).
struct PageFrame {
	bool occupied = false;
	int processId = -1;
	uint32_t virtualPage = 0;
	uint64_t loadedAt = 0;
	std::vector<uint8_t> bytes;
};

// Per-process virtual memory metadata (no RAM allocated until a page fault).
struct ProcessPageTable {
	size_t memoryBytes = 0;
	size_t pageCount = 0;
};
