#pragma once

#include <string>

class CommandUtils {
public:
	static bool IsValidPowerOfTwo(size_t size);
	static bool TryParseMemorySize(const std::string& arg, size_t& outSize);
};

