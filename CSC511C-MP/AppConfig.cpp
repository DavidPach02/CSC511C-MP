#include "AppConfig.h"
#include "ParseUtils.h"
#include <algorithm>
#include <fstream>
#include <random>
#include <sstream>
#include <vector>

namespace {
	constexpr int DEFAULT_NUM_CPU = 4;
	constexpr SchedulingAlgorithm DEFAULT_SCHEDULER = SchedulingAlgorithm::FCFS;
	constexpr int DEFAULT_QUANTUM_CYCLES = 5;
	constexpr int DEFAULT_BATCH_PROCESS_FREQ = 1;
	constexpr int DEFAULT_DELAYS_PER_EXEC = 2;
	constexpr int DEFAULT_TICKER_DELAY_MS = 33;
	constexpr int DEFAULT_MIN_INSTRUCTIONS = 25;
	constexpr int DEFAULT_MAX_INSTRUCTIONS = 100;
	constexpr int DEFAULT_MAX_OVERALL_MEM = 16384;
	constexpr int DEFAULT_MEM_PER_FRAME = 16;
	constexpr int DEFAULT_MIN_MEM_PER_PROC = 64;
	constexpr int DEFAULT_MAX_MEM_PER_PROC = 4096;
	constexpr int DEFAULT_MAX_CUSTOM_INSTRUCTIONS = 50;
	constexpr int MIN_PROCESS_MEMORY = 1 << 6;   // 64 bytes
	constexpr int MAX_PROCESS_MEMORY = 1 << 16;  // 65536 bytes

	bool IsValidProcessMemorySize(int size) {
		return size >= MIN_PROCESS_MEMORY
			&& size <= MAX_PROCESS_MEMORY
			&& (size & (size - 1)) == 0;
	}

	bool IsValidPhysicalMemorySize(int size) {
		return size >= MIN_PROCESS_MEMORY
			&& size <= MAX_PROCESS_MEMORY
			&& (size & (size - 1)) == 0;
	}

	void NormalizePhysicalMemoryConfig(int& maxOverallMemory, int& memoryPerFrame) {
		if (!IsValidPhysicalMemorySize(maxOverallMemory)) {
			maxOverallMemory = DEFAULT_MAX_OVERALL_MEM;
		}

		if (!IsValidPhysicalMemorySize(memoryPerFrame)) {
			memoryPerFrame = DEFAULT_MEM_PER_FRAME;
		}

		if (memoryPerFrame > maxOverallMemory) {
			memoryPerFrame = DEFAULT_MEM_PER_FRAME;
		}

		if (maxOverallMemory % memoryPerFrame != 0) {
			maxOverallMemory = (maxOverallMemory / memoryPerFrame) * memoryPerFrame;
			if (maxOverallMemory < memoryPerFrame) {
				maxOverallMemory = DEFAULT_MAX_OVERALL_MEM;
				memoryPerFrame = DEFAULT_MEM_PER_FRAME;
			}
		}
	}

	bool TryApplyProcessMemorySize(int candidate, int& minMemoryPerProcess, int& maxMemoryPerProcess) {
		if (!IsValidProcessMemorySize(candidate)) {
			return false;
		}

		minMemoryPerProcess = candidate;
		maxMemoryPerProcess = candidate;
		return true;
	}

	std::vector<int> BuildValidProcessMemorySizes(int minMemoryPerProcess, int maxMemoryPerProcess) {
		std::vector<int> validSizes;
		for (int candidate = MIN_PROCESS_MEMORY; candidate <= MAX_PROCESS_MEMORY; candidate <<= 1) {
			if (candidate >= minMemoryPerProcess && candidate <= maxMemoryPerProcess) {
				validSizes.push_back(candidate);
			}
		}

		if (validSizes.empty()) {
			validSizes.push_back(DEFAULT_MIN_MEM_PER_PROC);
		}

		return validSizes;
	}
}

int AppConfig::GetNumCpu() const { return numCpu; }
SchedulingAlgorithm AppConfig::GetSchedulerAlgorithm() const { return schedulerAlgorithm; }
int AppConfig::GetQuantumCycles() const { return quantumCycles; }
int AppConfig::GetBatchProcessFreq() const { return batchProcessFreq; }
int AppConfig::GetDelaysPerExec() const { return delaysPerExec; }
int AppConfig::GetTickerDelayMs() const { return tickerDelayMs; }
int AppConfig::GetMinInstructions() const { return minInstructions; }
int AppConfig::GetMaxInstructions() const { return maxInstructions; }
int AppConfig::GetMaxOverallMemory() const { return maxOverallMemory; }
int AppConfig::GetMemoryPerFrame() const { return memoryPerFrame; }
int AppConfig::GetMinMemoryPerProcess() const { return minMemoryPerProcess; }
int AppConfig::GetMaxMemoryPerProcess() const { return maxMemoryPerProcess; }
int AppConfig::GetMaxCustomInstructions() const { return maxCustomInstructions; }

size_t AppConfig::RollSchedulerProcessMemory() const {
	const std::vector<int> validSizes = BuildValidProcessMemorySizes(minMemoryPerProcess, maxMemoryPerProcess);
	std::mt19937 generator(std::random_device{}());
	std::uniform_int_distribution<size_t> sizePick(0, validSizes.size() - 1);
	return static_cast<size_t>(validSizes[sizePick(generator)]);
}

AppConfig::AppConfig(
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
	int minMemoryPerProcess,
	int maxMemoryPerProcess,
	int maxCustomInstructions)
	: numCpu(numCpu),
	  schedulerAlgorithm(schedulerAlgorithm),
	  quantumCycles(quantumCycles),
	  batchProcessFreq(batchProcessFreq),
	  delaysPerExec(delaysPerExec),
	  tickerDelayMs(tickerDelayMs),
	  minInstructions(minInstructions),
	  maxInstructions(maxInstructions),
	  maxOverallMemory(maxOverallMemory),
	  memoryPerFrame(memoryPerFrame),
	  minMemoryPerProcess(minMemoryPerProcess),
	  maxMemoryPerProcess(maxMemoryPerProcess),
	  maxCustomInstructions(maxCustomInstructions) {
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
	int tickerDelayMs = DEFAULT_TICKER_DELAY_MS;
	int minInstructions = DEFAULT_MIN_INSTRUCTIONS;
	int maxInstructions = DEFAULT_MAX_INSTRUCTIONS;
	int maxOverallMemory = DEFAULT_MAX_OVERALL_MEM;
	int memoryPerFrame = DEFAULT_MEM_PER_FRAME;
	int minMemoryPerProcess = DEFAULT_MIN_MEM_PER_PROC;
	int maxMemoryPerProcess = DEFAULT_MAX_MEM_PER_PROC;
	int maxCustomInstructions = DEFAULT_MAX_CUSTOM_INSTRUCTIONS;

	std::ifstream configFile(configFilePath);
	if (!configFile.is_open()) {
		return AppConfig(
			numCpu, schedulerAlgorithm, quantumCycles, batchProcessFreq,
			delaysPerExec, tickerDelayMs, minInstructions, maxInstructions,
			maxOverallMemory, memoryPerFrame, minMemoryPerProcess, maxMemoryPerProcess,
			maxCustomInstructions);
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
		} else if (name == "ticker-delay-ms" && parsedInt.has_value() && parsedInt.value() > 0) {
			tickerDelayMs = parsedInt.value();
		} else if (name == "min-ins" && parsedInt.has_value() && parsedInt.value() > 0) {
			minInstructions = parsedInt.value();
		} else if (name == "max-ins" && parsedInt.has_value() && parsedInt.value() > 0) {
			maxInstructions = parsedInt.value();
		} else if (name == "max-overall-mem" && parsedInt.has_value() && parsedInt.value() > 0) {
			maxOverallMemory = parsedInt.value();
		} else if (name == "mem-per-frame" && parsedInt.has_value() && parsedInt.value() > 0) {
			memoryPerFrame = parsedInt.value();
		} else if (name == "min-mem-per-proc" && parsedInt.has_value()) {
			if (IsValidProcessMemorySize(parsedInt.value())) {
				minMemoryPerProcess = parsedInt.value();
			}
		} else if (name == "max-mem-per-proc" && parsedInt.has_value()) {
			if (IsValidProcessMemorySize(parsedInt.value())) {
				maxMemoryPerProcess = parsedInt.value();
			}
		} else if (name == "mem-per-proc" && parsedInt.has_value()) {
			TryApplyProcessMemorySize(parsedInt.value(), minMemoryPerProcess, maxMemoryPerProcess);
		}
	}

	if (minInstructions > maxInstructions) {
		std::swap(minInstructions, maxInstructions);
	}

	if (minMemoryPerProcess > maxMemoryPerProcess) {
		std::swap(minMemoryPerProcess, maxMemoryPerProcess);
	}

	if (!IsValidProcessMemorySize(minMemoryPerProcess)) {
		minMemoryPerProcess = DEFAULT_MIN_MEM_PER_PROC;
	}

	if (!IsValidProcessMemorySize(maxMemoryPerProcess)) {
		maxMemoryPerProcess = DEFAULT_MAX_MEM_PER_PROC;
	}

	if (minMemoryPerProcess > maxMemoryPerProcess) {
		std::swap(minMemoryPerProcess, maxMemoryPerProcess);
	}

	NormalizePhysicalMemoryConfig(maxOverallMemory, memoryPerFrame);

	return AppConfig(
		numCpu, schedulerAlgorithm, quantumCycles, batchProcessFreq,
		delaysPerExec, tickerDelayMs, minInstructions, maxInstructions,
		maxOverallMemory, memoryPerFrame, minMemoryPerProcess, maxMemoryPerProcess,
		maxCustomInstructions);
}
