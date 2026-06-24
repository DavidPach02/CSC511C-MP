#include "DummyProcessGenerator.h"
#include "AppConfig.h"
#include "Scheduler.h"
#include "ProcessManager.h"
#include "ConsoleManager.h"
#include "BaseScreen.h"
#include "Process.h"
#include "PrintCommand.h"
#include <memory>
#include <random>
#include <string>

int DummyProcessGenerator::nextProcessId = 1;
std::vector<std::string> DummyProcessGenerator::createdScreenNames;

std::string DummyProcessGenerator::MakeProcessName(int processId) {
	if (processId < 10) {
		return "p0" + std::to_string(processId);
	}
	return "p" + std::to_string(processId);
}

void DummyProcessGenerator::Reset() {
	ConsoleManager* consoleManager = ConsoleManager::GetInstance();
	for (const std::string& screenName : createdScreenNames) {
		consoleManager->UnregisterScreen(screenName);
	}

	createdScreenNames.clear();
	nextProcessId = 1;
}

bool DummyProcessGenerator::GenerateOne(const AppConfig& appConfig) {
	if (Scheduler::GetInstance() == nullptr) {
		return false;
	}

	std::random_device randomDevice;
	std::mt19937 generator(randomDevice());
	std::uniform_int_distribution<int> commandCountDistribution(
		appConfig.GetMinInstructions(),
		appConfig.GetMaxInstructions());

	const int commandCount = commandCountDistribution(generator);
	const std::string processName = MakeProcessName(nextProcessId);

	auto process = std::make_shared<Process>(nextProcessId, processName);
	++nextProcessId;

	for (int commandIndex = 0; commandIndex < commandCount; ++commandIndex) {
		process->AddCommand(std::make_unique<PrintCommand>(*process));
	}

	// Register the screen for the process
	ConsoleManager::GetInstance()->RegisterScreen(std::make_shared<BaseScreen>(processName), false);
	// Add the screen name to the list of created screen names
	createdScreenNames.push_back(processName);
    // Add the process to the scheduler
	Scheduler::GetInstance()->AddProcess(process);
	// Add the process to the process manager
	ProcessManager::GetInstance()->AddProcess(process);

	return true;
}
