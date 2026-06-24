#pragma once

#include "Scheduler.h"

// Round Robin: each process runs up to quantum-cycles commands, then returns to the queue.
class RRScheduler : public Scheduler {
public:
	RRScheduler(int totalCores, int quantumCommands);

	SchedulingAlgorithm GetAlgorithm() const override;
	std::string GetAlgorithmName() const override;

protected:
	void RunCore(int coreID) override;

private:
	int quantumCommands;
};
