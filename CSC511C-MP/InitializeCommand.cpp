#include "InitializeCommand.h"
#include "AppConfig.h"
#include "SystemState.h"
#include "CPUManager.h"
#include "ProcessManager.h"
#include "Process.h"
#include "Scheduler.h"
#include "CPUTicker.h"
#include "MemoryManager.h"
#include <filesystem>
#include <iostream>

namespace {
	void ClearMemoryLoggerOutputDirectory() {
		const std::filesystem::path outputDirectory = "../output";
		std::error_code error;

		if (!std::filesystem::exists(outputDirectory, error)) {
			std::filesystem::create_directories(outputDirectory, error);
			return;
		}

		for (const auto& entry : std::filesystem::directory_iterator(outputDirectory, error)) {
			if (error) {
				break;
			}
			std::filesystem::remove_all(entry.path(), error);
			if (error) {
				break;
			}
		}
	}
}

bool InitializeCommand::Execute(const std::vector<std::string>& args) const {
	(void)args;

	if (SystemState::IsInitialized()) {
		std::cout << "System is already initialized.\n";
		return true;
	}

	const AppConfig appConfig = AppConfig::FromConfigFile(SystemState::CONFIG_FILE_PATH);
	SystemState::Initialize(appConfig);
	ClearMemoryLoggerOutputDirectory();

	ProcessManager::Initialize();
	CPUManager::Initialize(appConfig.GetNumCpu());
	MemoryManager::Initialize(appConfig.GetMaxOverallMemory());
	Process::SetDelaysPerExec(appConfig.GetDelaysPerExec());

	Scheduler::Initialize(
		appConfig.GetNumCpu(),
		appConfig.GetSchedulerAlgorithm(),
		appConfig.GetQuantumCycles());

	CPUTicker::Start();

	if (!Scheduler::GetInstance()->IsRunning()) {
		Scheduler::GetInstance()->Start();
	}
	
	std::cout << "Initialized CPU configuration from config.txt\n";
	std::cout << "  num-cpu: " << appConfig.GetNumCpu() << "\n";
	std::cout << "  scheduler: "
		<< (appConfig.GetSchedulerAlgorithm() == SchedulingAlgorithm::RR ? "rr" : "fcfs") << "\n";
	std::cout << "  quantum-cycles: " << appConfig.GetQuantumCycles() << "\n";
	std::cout << "  batch-process-freq: " << appConfig.GetBatchProcessFreq() << "\n";
	std::cout << "  delays-per-exec: " << appConfig.GetDelaysPerExec() << "\n";
	std::cout << "  min-ins: " << appConfig.GetMinInstructions() << "\n";
	std::cout << "  max-ins: " << appConfig.GetMaxInstructions() << "\n";
	std::cout << "  max-overall-mem: " << appConfig.GetMaxOverallMemory() << "\n";
	std::cout << "  mem-per-frame: " << appConfig.GetMemoryPerFrame() << "\n";
	std::cout << "  mem-per-proc: " << appConfig.GetMemoryPerProcess() << "\n";

	return true;
}

std::string InitializeCommand::Name() const { return "initialize"; }
std::string InitializeCommand::Description() const {
	return "Reads config.txt and boots the OS simulator.";
}
