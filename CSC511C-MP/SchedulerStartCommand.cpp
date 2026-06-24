#include "SchedulerStartCommand.h"
#include "SystemState.h"
#include "Scheduler.h"
#include "CPUTicker.h"
#include "ConsoleManager.h"
#include <iostream>

bool SchedulerStartCommand::Execute(const std::vector<std::string>& args) const {
	(void)args;

	if (!SystemState::IsInitialized()) {
		std::cout << "Run initialize before scheduler-start.\n";
		return true;
	}

	if (!ConsoleManager::GetInstance()->IsOnMainConsole()) {
		std::cout << "scheduler-start is only available on the main menu.\n";
		return true;
	}

	if (CPUTicker::GetInstance()->IsGenerationEnabled()) {
		std::cout << "Scheduler test is already running.\n";
		return true;
	}

	// Get the app config
	const AppConfig& appConfig = SystemState::GetConfig();

	Scheduler::Initialize(
		appConfig.GetNumCpu(),
		appConfig.GetSchedulerAlgorithm(),
		appConfig.GetQuantumCycles());

	CPUTicker::Start();
	CPUTicker::GetInstance()->SetGenerationEnabled(true);

	// If the scheduler is not running, start it
	if (!Scheduler::GetInstance()->IsRunning()) {
		Scheduler::GetInstance()->Start();
	}

	std::cout << "Scheduler test started (" << Scheduler::GetInstance()->GetAlgorithmName() << ").\n";
	std::cout << "Generating a new process every " << appConfig.GetBatchProcessFreq()
		<< " CPU tick(s). Use screen -ls to view status.\n";

	return true;
}

std::string SchedulerStartCommand::Name() const { return "scheduler-start"; }
std::string SchedulerStartCommand::Description() const {
	return "Continuously generates dummy processes every N CPU ticks.";
}
