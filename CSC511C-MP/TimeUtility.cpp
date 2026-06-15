#include "TimeUtility.h"

std::string TimeUtility::GetCurrentTimeString(const std::string& separator) {
	// Get current time point from system clock
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);

	// Convert to local time structure
	std::tm local_time;
#if defined(_MSC_VER)
	localtime_s(&local_time, &now_time); // Windows safe version
#else
	localtime_r(&now_time, &local_time); // POSIX/Linux safe version
#endif

	// Format string to HH:MM:SS (e.g., 14:30:05)
	std::stringstream ss;
	ss << std::setfill('0')
		<< std::setw(2) << local_time.tm_hour << separator
		<< std::setw(2) << local_time.tm_min << separator
		<< std::setw(2) << local_time.tm_sec;

	return ss.str();
}

std::string TimeUtility::GetCurrentDateString(const std::string& separator) {
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);

	std::tm local_time;
#if defined(_MSC_VER)
	localtime_s(&local_time, &now_time);
#else
	localtime_r(&now_time, &local_time);
#endif

	// Format string to MM/DD/YYYY (e.g., 06/08/2026)
	std::stringstream ss;
	ss << std::setfill('0')
		<< std::setw(2) << (local_time.tm_mon + 1) << separator
		<< std::setw(2) << local_time.tm_mday << separator
		<< (local_time.tm_year + 1900);

	return ss.str();
}