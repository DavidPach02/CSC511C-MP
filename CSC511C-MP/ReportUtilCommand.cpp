#include "ReportUtilCommand.h"

bool ReportUtilCommand::Execute(const std::vector<std::string>& args) const {
	std::cout << Name() + " command recognized. Doing something.\n";
	return true;
}

std::string ReportUtilCommand::Name() const { return "report-util"; }
std::string ReportUtilCommand::Description() const { return "Takes a snapshot of the active sessions and creates a txt file."; }
