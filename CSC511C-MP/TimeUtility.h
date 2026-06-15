#pragma once

#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>

class TimeUtility {
public:
	static std::string GetCurrentTimeString(const bool militaryTime = true, const std::string& separator = "");
	static std::string GetCurrentDateString(const std::string& separator = "");
};

