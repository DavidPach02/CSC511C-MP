#include "SwitchScreenCommand.h"
#include "ProcessManager.h"
#include "Process.h"

bool SwitchScreenCommand::Execute(const std::vector<std::string>& args) const {
	const int argsCount = static_cast<int>(args.size());
	if (argsCount <= 1 || argsCount > 2) {
		std::cout << "\033[31mUsage: screen -r <process_name>\033[0m\n";
		return true;
	}

	const std::string processName = args[1];
	ConsoleManager* consoleManager = ConsoleManager::GetInstance();

	std::shared_ptr<Process> process = ProcessManager::GetInstance()->GetProcessByName(processName);
	if (process == nullptr) {
		std::cout << "\033[31mProcess " << processName << " not found.\033[0m\n";
		return true;
	}

	if (process->GetStatusEnum() == ProcessStatus::Terminated) {
		if (process->HasMemoryAccessViolation()) {
			std::cout << "\033[31mProcess " << processName
				<< " shut down due to memory access violation error that occurred at "
				<< process->GetMemoryAccessViolationTime() << " - "
				<< process->GetMemoryAccessViolationAddress() << " invalid.\033[0m\n";
		} else {
			std::cout << "\033[31mProcess " << processName << " not found.\033[0m\n";
		}
		return true;
	}

	if (!consoleManager->HasScreen(processName)) {
		std::cout << "\033[31mProcess " << processName << " not found.\033[0m\n";
		return true;
	}

	consoleManager->SwitchScreen(processName);

	return true;
}

std::string SwitchScreenCommand::Name() const { return "screen -r"; }
std::string SwitchScreenCommand::Description() const { return "Resumes access to an existing process screen."; }
