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

bool ParseUtils::TryParseInt(const std::string& str, int& outValue) {
	return TryParseInt(str.c_str(), outValue);
}

bool ParseUtils::TryParseInt(const char* str, int& outValue) {
	if (str == nullptr || *str == '\0') {
		return false;
	}

	try {
		outValue = std::stoi(str);
		return true;
	}
	catch (...) {
		return false;
	}
}

bool ParseUtils::TryParseString(const std::string& str, std::string& outValue) {
	std::string trimmed = Trim(str);
	if (trimmed.empty()) {
		return false;
	}
	outValue = trimmed;
	return true;
}

bool ParseUtils::TryParseString(const char* str, std::string& outValue) {
	if (str == nullptr || *str == '\0') {
		return false;
	}
	return TryParseString(std::string(str), outValue);
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
