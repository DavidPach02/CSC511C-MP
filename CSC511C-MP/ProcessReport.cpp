#include "ProcessReport.h"
#include "CPUManager.h"
#include "ProcessManager.h"
#include "Process.h"
#include <iomanip>

std::stringstream ProcessReport::BuildSummary() {
	std::stringstream summary;
	WriteSummary(summary);
	return summary;
}

void ProcessReport::WriteSummary(std::ostream& output) {
	CPUManager* cpuManager = CPUManager::GetInstance();
	output << "CPU Utilization: " << std::fixed << std::setprecision(2)
		<< cpuManager->GetCPUUtilization() << "%\n";
	output << "Cores used: " << (cpuManager->GetTotalCores() - cpuManager->GetAvailableCores()) << "\n";
	output << "Cores available: " << cpuManager->GetAvailableCores() << "\n";
	output << "---------------------------------------\n";
	output << "Running processes:\n";

	const auto& processes = ProcessManager::GetInstance()->GetAllProcesses();
	for (const auto& process : processes) {
		if (process->GetStatusEnum() == ProcessStatus::Running) {
			output << process->GetName() << " (" << process->GetStartDate() << " "
				<< process->GetStartTime() << ") Core: " << process->GetCoreID()
				<< " " << process->GetExecutedCommandCount() << " / "
				<< process->GetCommandCount() << "\n";
		}
	}

	output << "\nFinished processes:\n";
	for (const auto& process : processes) {
		if (process->GetStatusEnum() == ProcessStatus::Terminated) {
			output << process->GetName() << " (" << process->GetEndDate() << " "
				<< process->GetEndTime() << ") Finished "
				<< process->GetCommandCount() << " / "
				<< process->GetCommandCount() << "\n";
		}
	}

	output << "---------------------------------------\n";
}
