#pragma once

#include <string>
#include "Scheduler.h"

class AppConfig {
public:
	static AppConfig FromConfigFile(const std::string& configFilePath);

	int GetTotalCores() const;
	SchedulingAlgorithm GetSchedulerAlgorithm() const;
	int GetArtificialCommandDelayMs() const;

private:
	explicit AppConfig(int totalCores, SchedulingAlgorithm schedulerAlgorithm = SchedulingAlgorithm::FCFS, int artificialCommandDelayMs = 50);

	static int ParseConfigFile(const std::string& configFilePath);
	static SchedulingAlgorithm ParseConfigFileSchedulerAlgorithm(const std::string& configFilePath);
	static int ParseConfigFileArtificialCommandDelay(const std::string& configFilePath);

	int totalCores;
	SchedulingAlgorithm schedulerAlgorithm;
	int artificialCommandDelayMs;
};
