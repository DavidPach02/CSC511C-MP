#include "AppConfig.h"
#include "ParseUtils.h"
#include <string>
#include <fstream>
#include <sstream>

namespace
{
	constexpr int DEFAULT_TOTAL_CORES = 4;
	constexpr SchedulingAlgorithm DEFAULT_SCHEDULER_ALGORITHM = SchedulingAlgorithm::FCFS;
	constexpr int DEFAULT_ARTIFICIAL_COMMAND_DELAY_MS = 50;
}

int AppConfig::GetTotalCores() const
{
	return totalCores;
}

SchedulingAlgorithm AppConfig::GetSchedulerAlgorithm() const
{
	return schedulerAlgorithm;
}

int AppConfig::GetArtificialCommandDelayMs() const
{
	return artificialCommandDelayMs;
}

AppConfig::AppConfig(int totalCores, SchedulingAlgorithm schedulerAlgorithm, int artificialCommandDelayMs)
	: totalCores(totalCores), schedulerAlgorithm(schedulerAlgorithm), artificialCommandDelayMs(artificialCommandDelayMs)
{
}

AppConfig AppConfig::FromConfigFile(const std::string& configFilePath)
{
	return AppConfig(
		ParseConfigFile(configFilePath),
		ParseConfigFileSchedulerAlgorithm(configFilePath),
		ParseConfigFileArtificialCommandDelay(configFilePath)
	);
}

int AppConfig::ParseConfigFile(const std::string& configFilePath)
{
	std::ifstream configFile(configFilePath);
	if (!configFile.is_open()) {
		return DEFAULT_TOTAL_CORES;
	}

	std::string line;
	while (std::getline(configFile, line)) {
		line = ParseUtils::Trim(line);

		// Skip comments and empty lines
		if (line.empty() || line[0] == '#') {
			continue;
		}

		// Parse space-separated name value format
		std::istringstream iss(line);
		std::string name, value;
		
		if (iss >> name >> value) {
			if (name == "cores") {
				int parsedValue = 0;
				if (ParseUtils::TryParseInt(value, parsedValue) && parsedValue > 0) {
					return parsedValue;
				}
			}
		}
	}

	return DEFAULT_TOTAL_CORES;
}

SchedulingAlgorithm AppConfig::ParseConfigFileSchedulerAlgorithm(const std::string& configFilePath)
{
	std::ifstream configFile(configFilePath);
	if (!configFile.is_open()) {
		return DEFAULT_SCHEDULER_ALGORITHM;
	}

	std::string line;
	while (std::getline(configFile, line)) {
		line = ParseUtils::Trim(line);

		// Skip comments and empty lines
		if (line.empty() || line[0] == '#') {
			continue;
		}

		// Parse space-separated name value format
		std::istringstream iss(line);
		std::string name, value;
		
		if (iss >> name >> value) {
			if (name == "scheduler-algorithm") {
				// Use case-insensitive comparison
				if (ParseUtils::EqualsIgnoreCase(value, "fcfs")) {
					return SchedulingAlgorithm::FCFS;
				}
			}
		}
	}

	return DEFAULT_SCHEDULER_ALGORITHM;
}

int AppConfig::ParseConfigFileArtificialCommandDelay(const std::string& configFilePath)
{
	std::ifstream configFile(configFilePath);
	if (!configFile.is_open()) {
		return DEFAULT_ARTIFICIAL_COMMAND_DELAY_MS;
	}

	std::string line;
	while (std::getline(configFile, line)) {
		line = ParseUtils::Trim(line);

		// Skip comments and empty lines
		if (line.empty() || line[0] == '#') {
			continue;
		}

		// Parse space-separated name value format
		std::istringstream iss(line);
		std::string name, value;

		if (iss >> name >> value) {
			if (name == "artificial-command-delay-ms") {
				int parsedValue = 0;
				if (ParseUtils::TryParseInt(value, parsedValue) && parsedValue >= 0) {
					return parsedValue;
				}
			}
		}
	}

	return DEFAULT_ARTIFICIAL_COMMAND_DELAY_MS;
}
