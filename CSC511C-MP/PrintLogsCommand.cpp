#include "PrintLogsCommand.h"

#include "Process.h"
#include "TimeUtility.h"
#include <iostream>
#include <fstream>
#include <sstream>

PrintLogsCommand::PrintLogsCommand(std::shared_ptr<Process> process) : process(process) {
}

bool PrintLogsCommand::Execute(const std::vector<std::string>& args) const {
	if (!this->process) {
		std::cout << "no process attached" << std::endl;
		return true;
	}

	std::cout << "Logs:" << std::endl << this->process->GetLogs() << std::endl;
	std::cout << "Current instruction line: " << this->process->GetExecutedCommandCount() << std::endl;
	std::cout << "Lines of code: " << this->process->GetCommandCount() << std::endl;

	return true;
}

std::string PrintLogsCommand::Name() const { return "process-smi"; }
std::string PrintLogsCommand::Description() const { return "Displays the log messages of the process."; }
