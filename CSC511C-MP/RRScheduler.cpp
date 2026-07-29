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

bool RRScheduler::ExecuteProcessOnCore(std::shared_ptr<Process>& process, int coreID) {
	EITThread executionThread(process, coreID);
	return executionThread.ExecuteTimeSlice(quantumCommands);
}
