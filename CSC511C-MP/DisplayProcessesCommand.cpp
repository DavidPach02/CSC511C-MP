#include "DisplayProcessesCommand.h"
#include "ProcessReport.h"
#include <iostream>

bool DisplayProcessesCommand::Execute(const std::vector<std::string>& args) const {
	(void)args;
	ProcessReport::WriteSummary(std::cout);
	return true;
}

std::string DisplayProcessesCommand::Name() const { return "screen"; }
std::string DisplayProcessesCommand::Description() const {
	return "Lists CPU utilization and running/finished processes.";
}
std::string DisplayProcessesCommand::Argument() const { return "-ls"; }
