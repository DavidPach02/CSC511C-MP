#include "AppConfig.h"
#include "ParseUtils.h"
#include <fstream>
#include <sstream>

namespace {
	constexpr int DEFAULT_NUM_CPU = 4;
	constexpr SchedulingAlgorithm DEFAULT_SCHEDULER = SchedulingAlgorithm::FCFS;
	constexpr int DEFAULT_QUANTUM_CYCLES = 5;
	constexpr int DEFAULT_BATCH_PROCESS_FREQ = 1;
	constexpr int DEFAULT_DELAYS_PER_EXEC = 0;
	constexpr int DEFAULT_MIN_INSTRUCTIONS = 25;
	constexpr int DEFAULT_MAX_INSTRUCTIONS = 100;
}

int AppConfig::GetNumCpu() const { return numCpu; }
SchedulingAlgorithm AppConfig::GetSchedulerAlgorithm() const { return schedulerAlgorithm; }
int AppConfig::GetQuantumCycles() const { return quantumCycles; }
int AppConfig::GetBatchProcessFreq() const { return batchProcessFreq; }
int AppConfig::GetDelaysPerExec() const { return delaysPerExec; }
int AppConfig::GetMinInstructions() const { return minInstructions; }
int AppConfig::GetMaxInstructions() const { return maxInstructions; }

AppConfig::AppConfig(
	int numCpu,
	SchedulingAlgorithm schedulerAlgorithm,
	int quantumCycles,
	int batchProcessFreq,
	int delaysPerExec,
	int minInstructions,
	int maxInstructions)
	: numCpu(numCpu),
	  schedulerAlgorithm(schedulerAlgorithm),
	  quantumCycles(quantumCycles),
	  batchProcessFreq(batchProcessFreq),
	  delaysPerExec(delaysPerExec),
	  minInstructions(minInstructions),
	  maxInstructions(maxInstructions) {
}

AppConfig AppConfig::FromConfigFile(const std::string& configFilePath) {
	return ParseConfigFile(configFilePath);
}

AppConfig AppConfig::ParseConfigFile(const std::string& configFilePath) {
	int numCpu = DEFAULT_NUM_CPU;
	SchedulingAlgorithm schedulerAlgorithm = DEFAULT_SCHEDULER;
	int quantumCycles = DEFAULT_QUANTUM_CYCLES;
	int batchProcessFreq = DEFAULT_BATCH_PROCESS_FREQ;
	int delaysPerExec = DEFAULT_DELAYS_PER_EXEC;
	int minInstructions = DEFAULT_MIN_INSTRUCTIONS;
	int maxInstructions = DEFAULT_MAX_INSTRUCTIONS;

	std::ifstream configFile(configFilePath);
	if (!configFile.is_open()) {
		return AppConfig(
			numCpu, schedulerAlgorithm, quantumCycles, batchProcessFreq,
			delaysPerExec, minInstructions, maxInstructions);
	}

	std::string line;
	while (std::getline(configFile, line)) {
		line = ParseUtils::Trim(line);
		if (line.empty() || line[0] == '#') {
			continue;
		}

		std::istringstream iss(line);
		std::string name;
		std::string value;
		if (!(iss >> name >> value)) {
			continue;
		}

		auto parsedInt = ParseUtils::ParseInt(value);

		if (name == "num-cpu" && parsedInt.has_value() && parsedInt.value() > 0) {
			numCpu = parsedInt.value();
		} else if (name == "scheduler") {
			if (ParseUtils::EqualsIgnoreCase(value, "fcfs")) {
				schedulerAlgorithm = SchedulingAlgorithm::FCFS;
			} else if (ParseUtils::EqualsIgnoreCase(value, "rr")) {
				schedulerAlgorithm = SchedulingAlgorithm::RR;
			}
		} else if (name == "quantum-cycles" && parsedInt.has_value() && parsedInt.value() > 0) {
			quantumCycles = parsedInt.value();
		} else if (name == "batch-process-freq" && parsedInt.has_value() && parsedInt.value() > 0) {
			batchProcessFreq = parsedInt.value();
		} else if (name == "delays-per-exec" && parsedInt.has_value() && parsedInt.value() >= 0) {
			delaysPerExec = parsedInt.value();
		} else if (name == "min-ins" && parsedInt.has_value() && parsedInt.value() > 0) {
			minInstructions = parsedInt.value();
		} else if (name == "max-ins" && parsedInt.has_value() && parsedInt.value() > 0) {
			maxInstructions = parsedInt.value();
		}
	}

	if (minInstructions > maxInstructions) {
		std::swap(minInstructions, maxInstructions);
	}

	return AppConfig(
		numCpu, schedulerAlgorithm, quantumCycles, batchProcessFreq,
		delaysPerExec, minInstructions, maxInstructions);
}
