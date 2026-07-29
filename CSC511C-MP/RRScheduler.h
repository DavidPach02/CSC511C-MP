#pragma once

#include "Scheduler.h"

// Round Robin: each process runs up to quantum-cycles commands, then returns to the queue.
class RRScheduler : public Scheduler {
public:
	RRScheduler(int totalCores, int quantumCommands);

	SchedulingAlgorithm GetAlgorithm() const override;
	std::string GetAlgorithmName() const override;

protected:
	bool ExecuteProcessOnCore(std::shared_ptr<Process>& process, int coreID) override;

private:
	int quantumCommands;
};
