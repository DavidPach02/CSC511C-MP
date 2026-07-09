#include "RRScheduler.h"
#include "EITThread.h"

RRScheduler::RRScheduler(int totalCores, int quantumCommands)
	: Scheduler(totalCores), quantumCommands(quantumCommands > 0 ? quantumCommands : 1) {
}

SchedulingAlgorithm RRScheduler::GetAlgorithm() const {
	return SchedulingAlgorithm::RR;
}

std::string RRScheduler::GetAlgorithmName() const {
	return "Round Robin (RR)";
}

void RRScheduler::RunCore(int coreID) {
	(void)coreID;
	while (true) {
		std::shared_ptr<Process> process;

		if (!DequeueProcess(process)) {
			return;
		}

		if (process->GetStatusEnum() == ProcessStatus::Sleeping) {
			process->WakeIfReady();
			if (process->GetStatusEnum() == ProcessStatus::Sleeping) {
				RequeueProcess(process);
				continue;
			}
		}
        
		if (!PrepareProcessForExecution(process)) {
			RequeueProcess(process);
			continue;
		}

		EITThread executionThread(process, coreID);

		if (executionThread.ExecuteTimeSlice(quantumCommands)) {
			RequeueProcess(process);
		} else {
			FinalizeProcess(process);
		}
	}
}
