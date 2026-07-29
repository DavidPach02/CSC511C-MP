#pragma once

#include "Scheduler.h"

// First Come First Served: each process runs all instructions before the next is dequeued.
class FCFSScheduler : public Scheduler {
public:
	explicit FCFSScheduler(int totalCores);

	SchedulingAlgorithm GetAlgorithm() const override;
	std::string GetAlgorithmName() const override;

protected:
	bool ExecuteProcessOnCore(std::shared_ptr<Process>& process, int coreID) override;
};
