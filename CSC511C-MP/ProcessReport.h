#pragma once

#include <sstream>
#include <iostream>

// Formats CPU utilization and running/finished process lists for screen -ls and csopesy-log.txt.
namespace ProcessReport {
	std::stringstream BuildSummary();
	void WriteSummary(std::ostream& output);
}
