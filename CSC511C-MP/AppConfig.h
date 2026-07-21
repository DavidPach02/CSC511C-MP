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
	int GetTickerDelayMs() const;
	int GetMinInstructions() const;
	int GetMaxInstructions() const;
	int GetMaxOverallMemory() const;
	int GetMemoryPerFrame() const;
	int GetMemoryPerProcess() const;
	int GetMaxCustomInstructions() const;

private:
	AppConfig(
		int numCpu,
		SchedulingAlgorithm schedulerAlgorithm,
		int quantumCycles,
		int batchProcessFreq,
		int delaysPerExec,
		int tickerDelayMs,
		int minInstructions,
		int maxInstructions,
		int maxOverallMemory,
		int memoryPerFrame,
		int memoryPerProcess,
		int maxCustomInstructions);

	static AppConfig ParseConfigFile(const std::string& configFilePath);

	int numCpu;
	SchedulingAlgorithm schedulerAlgorithm;
	int quantumCycles;
	int batchProcessFreq;
	int delaysPerExec;
	int tickerDelayMs;
	int minInstructions;
	int maxInstructions;
	int maxOverallMemory;
	int memoryPerFrame;
	int memoryPerProcess;
	int maxCustomInstructions;
};
