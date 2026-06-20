#include "SwitchScreenCommand.h"

bool SwitchScreenCommand::Execute(const std::vector<std::string>& args) const {
	const int argsCount = static_cast<int>(args.size());
	if (argsCount <= 1 || argsCount > 2) {
		std::cout << "\033[31mUsage: screen -r <process_name>\033[0m\n";
		return true;
	}

	const std::string processName = args[1];
	ConsoleManager* consoleManager = ConsoleManager::GetInstance();

	// TODO: Also reject finished processes when Process status is tracked in CPUManager.
	if (!consoleManager->HasScreen(processName)) {
		std::cout << "Process " << processName << " not found.\n";
		return true;
	}

	consoleManager->SwitchScreen(processName);
	std::cout << "You are now on process: \033[33m" << processName << "\033[0m.\n";

	// TODO: List all processes on this screen when process tracking is implemented.

	return true;
}

std::string SwitchScreenCommand::Name() const { return "screen -r"; }
std::string SwitchScreenCommand::Description() const { return "Resumes access to an existing process screen."; }
