#include "SchedulerStopCommand.h"
#include "CPUTicker.h"
#include "ConsoleManager.h"
#include "SystemState.h"
#include <iostream>

bool SchedulerStopCommand::Execute(const std::vector<std::string>& args) const {
	(void)args;

	if (!SystemState::IsInitialized()) {
		std::cout << "Run initialize before scheduler-stop.\n";
		return true;
	}

	if (!ConsoleManager::GetInstance()->IsOnMainConsole()) {
		std::cout << "scheduler-stop is only available on the main menu. Type exit to return.\n";
		return true;
	}

	const bool wasGenerating = CPUTicker::GetInstance()->IsGenerationEnabled();
	CPUTicker::GetInstance()->StopGeneration();

	if (!wasGenerating) {
		std::cout << "Scheduler test is not generating processes.\n";
		return true;
	}

	std::cout << "Stopped generating dummy processes.\n";
	std::cout << "Existing processes will continue until they finish.\n";

	return true;
}

std::string SchedulerStopCommand::Name() const { return "scheduler-stop"; }
std::string SchedulerStopCommand::Description() const {
	return "Stops generating dummy processes.";
}
