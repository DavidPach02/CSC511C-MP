#include "CreateProcessInstructionCommand.h"
#include "SystemState.h"
#include "CommandUtils.h"
#include "ConsoleManager.h"
#include "ProcessManager.h"
#include "DummyProcessGenerator.h"
#include <iostream>

bool CreateProcessInstructionCommand::Execute(const std::vector<std::string>& args) const {
	if (!SystemState::IsInitialized()) {
		std::cout << "Run initialize before creating a screen.\n";
		return true;
	}

	const int argsCount = static_cast<int>(args.size());
	if (argsCount != 4) {
		std::cout << "\033[31mUsage: screen -c <process_name> <process_mem_size> \"<instructions>\" \033[0m\n";
		return true;
	}

	size_t requestedMemorySize = 0;
	if (!CommandUtils::TryParseMemorySize(args[2], requestedMemorySize)) {
		std::cout << "\033[31mInvalid memory size: " << args[2] << "\033[0m\n";
		return true;
	}

	if (!CommandUtils::IsValidPowerOfTwo(requestedMemorySize)) {
		std::cout << "\033[31mMemory size must be a power of two between 2^6 (64) and 2^16 (65536) bytes.\033[0m\n";
		return true;
	}

	const std::string processName = args[1];
	ConsoleManager* consoleManager = ConsoleManager::GetInstance();

	if (ProcessManager::GetInstance()->GetProcessByName(processName) != nullptr) {
		std::cout << "\033[31mProcess already exists: " << processName << "\033[0m\n";
		return true;
	}

	const AppConfig& config = SystemState::GetConfig();
	if (!DummyProcessGenerator::GenerateOneWithInstruction(config, processName, requestedMemorySize, args[3])) {
		std::cout << "\033[31mFailed to create process: " << processName << " with invalid instructions.\033[0m\n";
		return true;
	}

	std::cout << "Successfully created process with instructions: " << processName << " [" << requestedMemorySize << " bytes]\n";
	//consoleManager->SwitchScreen(processName);
	return true;
}

std::string CreateProcessInstructionCommand::Name() const { return "screen -c"; }
std::string CreateProcessInstructionCommand::Description() const {	return "Creates a new screen with an attached process and a set of instructions."; }

