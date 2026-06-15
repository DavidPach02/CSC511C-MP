#include "DisplayProcessesCommand.h"

bool DisplayProcessesCommand::Execute(const std::vector<std::string>& args) const {
	CPUManager::GetInstance()->DisplaySnapshot();
	return true;
}

std::string DisplayProcessesCommand::Name() const { return "screen ls"; }
std::string DisplayProcessesCommand::Description() const { return "Displays the CPU utilization and the processes."; }
std::string DisplayProcessesCommand::Argument() const { return "ls"; }