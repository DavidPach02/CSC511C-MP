#include "CommandUtils.h"
#include <stdexcept>

bool CommandUtils::IsValidPowerOfTwo(size_t size) {
	constexpr size_t minLimit = 1 << 6;  // 64 bytes
	constexpr size_t maxLimit = 1 << 16; // 65536 bytes

	return size >= minLimit && size <= maxLimit && (size & (size - 1)) == 0;
}

bool CommandUtils::TryParseMemorySize(const std::string& arg, size_t& outSize) {
	try {
		size_t processedSize = 0;
		unsigned long long parsedSize = std::stoull(arg, &processedSize);

		if (processedSize != arg.size()) {
			return false; // Not a valid number
		}
		if (parsedSize > std::numeric_limits<size_t>::max()) {
			return false; // Out of range for size_t
		}

		outSize = static_cast<size_t>(parsedSize);
		return true;
	}
	catch (const std::invalid_argument&) {
		return false; // Not a valid number
	}
	catch (const std::out_of_range&) {
		return false; // Out of range for size_t
	}
	return false;
}