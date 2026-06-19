#pragma once
#include <string>

class ParseUtils {
public:
	// String utilities
	static std::string Trim(const std::string& str);
	static std::string ToLower(const std::string& str);
	static std::string ToUpper(const std::string& str);
	
	// Parsing utilities — return true and set outValue on success
	static bool TryParseInt(const std::string& str, int& outValue);
	static bool TryParseInt(const char* str, int& outValue);
	static bool TryParseString(const std::string& str, std::string& outValue);
	static bool TryParseString(const char* str, std::string& outValue);
	
	// Comparison utilities
	static bool EqualsIgnoreCase(const std::string& a, const std::string& b);
	static bool EqualsIgnoreCase(const char* a, const char* b);
};
