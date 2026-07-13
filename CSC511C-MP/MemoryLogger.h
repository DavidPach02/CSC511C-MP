#pragma once

#include <cstdint>
#include <string>

class MemoryLogger {
public:
	static void LogTickSnapshot(uint64_t tick);
	static std::string PrintMemoryStats();
	static std::string PrintVirtualMemoryStats();
};
