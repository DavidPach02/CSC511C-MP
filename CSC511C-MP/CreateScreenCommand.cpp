#include "CreateScreenCommand.h"
#include "ProcessManager.h"
#include <iostream>

bool CreateScreenCommand::Execute(const std::vector<std::string>& args) const
{
	const int argsCount = static_cast<int>(args.size());
	if (argsCount <= 1 || argsCount > 2) {
		std::cout << "\033[31mUsage: screen -s <process_name>\033[0m\n";
		return true;
	}

	if (!SystemState::IsInitialized()) {
		std::cout << "Run initialize before creating a screen.\n";
		return true;
	}

	const std::string processName = args[1];
	ConsoleManager* consoleManager = ConsoleManager::GetInstance();

	if (consoleManager->HasScreen(processName)) {
		std::cout << "\033[31mScreen already exists: " << processName << "\033[0m\n";
		return true;
	}

	if (ProcessManager::GetInstance()->GetProcessByName(processName) != nullptr) {
		std::cout << "\033[31mProcess already exists: " << processName << "\033[0m\n";
		return true;
	}

	const AppConfig& config = SystemState::GetConfig();
	if (!DummyProcessGenerator::GenerateOne(config, processName)) {
		std::cout << "\033[31mFailed to create process: " << processName << "\033[0m\n";
		return true;
	}

	consoleManager->SwitchScreen(processName);
	return true;
}

std::string CreateScreenCommand::Name() const { return "screen -s"; }
std::string CreateScreenCommand::Description() const { return "Creates a new screen with an attached process."; }
