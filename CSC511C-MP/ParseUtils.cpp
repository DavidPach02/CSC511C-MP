#include "ParseUtils.h"
#include <cctype>
#include <algorithm>

std::string ParseUtils::Trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t\n\r");
	if (start == std::string::npos) {
		return "";
	}
	size_t end = str.find_last_not_of(" \t\n\r");
	return str.substr(start, end - start + 1);
}

std::string ParseUtils::ToLower(const std::string& str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(),
		[](char c) { return std::tolower(c); });
	return result;
}

std::string ParseUtils::ToUpper(const std::string& str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(),
		[](char c) { return std::toupper(c); });
	return result;
}

std::optional<int> ParseUtils::ParseInt(const std::string& str) {
	return ParseInt(str.c_str());
}

std::optional<int> ParseUtils::ParseInt(const char* str) {
	if (str == nullptr || *str == '\0') {
		return std::nullopt;
	}

	try {
		int value = std::stoi(str);
		return value;
	}
	catch (...) {
		return std::nullopt;
	}
}

std::optional<std::string> ParseUtils::ParseString(const std::string& str) {
	std::string trimmed = Trim(str);
	if (trimmed.empty()) {
		return std::nullopt;
	}
	return trimmed;
}

std::optional<std::string> ParseUtils::ParseString(const char* str) {
	if (str == nullptr || *str == '\0') {
		return std::nullopt;
	}
	return ParseString(std::string(str));
}

bool ParseUtils::EqualsIgnoreCase(const std::string& a, const std::string& b) {
	return ToLower(a) == ToLower(b);
}

bool ParseUtils::EqualsIgnoreCase(const char* a, const char* b) {
	if (a == nullptr || b == nullptr) {
		return a == b;
	}
	return EqualsIgnoreCase(std::string(a), std::string(b));
}
