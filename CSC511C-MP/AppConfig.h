#pragma once

#include <string>
#include "Scheduler.h"

class AppConfig {
public:
	static AppConfig FromConfigFile(const std::string& configFilePath);

	int GetNumCpu() const;
	SchedulingAlgorithm GetSchedulerAlgorithm() const;
	int GetQuantumCycles() const;
	int GetBatchProcessFreq() const;
	int GetDelaysPerExec() const;
	int GetMinInstructions() const;
	int GetMaxInstructions() const;

private:
	AppConfig(
		int numCpu,
		SchedulingAlgorithm schedulerAlgorithm,
		int quantumCycles,
		int batchProcessFreq,
		int delaysPerExec,
		int minInstructions,
		int maxInstructions);

	static AppConfig ParseConfigFile(const std::string& configFilePath);

	int numCpu;
	SchedulingAlgorithm schedulerAlgorithm;
	int quantumCycles;
	int batchProcessFreq;
	int delaysPerExec;
	int minInstructions;
	int maxInstructions;
};
