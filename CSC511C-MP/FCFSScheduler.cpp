#include "FCFSScheduler.h"
#include "EITThread.h"

FCFSScheduler::FCFSScheduler(int totalCores) : Scheduler(totalCores) {
}

SchedulingAlgorithm FCFSScheduler::GetAlgorithm() const {
	return SchedulingAlgorithm::FCFS;
}

std::string FCFSScheduler::GetAlgorithmName() const {
	return "First Come, First Served (FCFS)";
}

void FCFSScheduler::RunCore(int coreID) {
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

		EITThread executionThread(process, coreID);
		executionThread.RunToCompletion();

		if (process->GetStatusEnum() == ProcessStatus::Sleeping) {
			RequeueProcess(process);
		}
	}
}
