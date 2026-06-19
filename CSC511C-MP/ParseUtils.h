#pragma once

#include <string>
#include <optional>

class ParseUtils {
public:
	// String utilities
	static std::string Trim(const std::string& str);
	static std::string ToLower(const std::string& str);
	static std::string ToUpper(const std::string& str);
	
	// Parsing utilities
	static std::optional<int> ParseInt(const std::string& str);
	static std::optional<int> ParseInt(const char* str);
	static std::optional<std::string> ParseString(const std::string& str);
	static std::optional<std::string> ParseString(const char* str);
	
	// Comparison utilities
	static bool EqualsIgnoreCase(const std::string& a, const std::string& b);
	static bool EqualsIgnoreCase(const char* a, const char* b);
};
