#pragma once

#include <sstream>
#include <iostream>

// Formats CPU utilization, memory usage, and process lists for screen -ls and csopesy-log.txt.
namespace ProcessReport {
	std::stringstream BuildSummary();
	void WriteSummary(std::ostream& output);
	void WriteMemoryMap(std::ostream& output);
}
