#include "DisplayProcessesCommand.h"
#include "CPUManager.h"
#include "Process.h"
#include <iostream>
#include <iomanip>

bool DisplayProcessesCommand::Execute(const std::vector<std::string>& args) const {
	std::cout << "---------------------------------------\n";
	std::cout << "Running processes:\n";
	
	const auto& processes = CPUManager::GetInstance()->GetAllProcesses();
	
	// Display running processes
	for (const auto& process : processes) {
		if (process->GetStatusEnum() == ProcessStatus::Running) {
			std::cout << process->GetName() << " (" << process->GetStartDate() << " " 
					  << process->GetStartTime() << ") Core: " << process->GetCoreID() 
					  << " " << process->GetExecutedCommandCount() << " / " 
					  << process->GetCommandCount() << "\n";
		}
	}
	
	std::cout << "\nFinished processes:\n";
	
	// Display finished processes
	for (const auto& process : processes) {
		if (process->GetStatusEnum() == ProcessStatus::Terminated) {
			std::cout << process->GetName() << " (" << process->GetEndDate() << " " 
					  << process->GetEndTime() << ") Finished " 
					  << process->GetCommandCount() << " / " 
					  << process->GetCommandCount() << "\n";
		}
	}
	
	std::cout << "---------------------------------------\n";
	return true;
}

std::string DisplayProcessesCommand::Name() const { return "screen"; }
std::string DisplayProcessesCommand::Description() const { return "Displays the CPU utilization and the processes."; }
std::string DisplayProcessesCommand::Argument() const { return "-ls"; }