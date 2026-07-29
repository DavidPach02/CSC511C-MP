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

bool FCFSScheduler::ExecuteProcessOnCore(std::shared_ptr<Process>& process, int coreID) {
	EITThread executionThread(process, coreID);
	executionThread.RunToCompletion();

	if (process->GetStatusEnum() == ProcessStatus::Terminated) {
		return false;
	}

	return process->GetStatusEnum() == ProcessStatus::Sleeping;
}
