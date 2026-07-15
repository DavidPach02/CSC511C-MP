#include "CreateScreenCommand.h"
#include "ProcessManager.h"
#include <iostream>

bool CreateScreenCommand::Execute(const std::vector<std::string>& args) const {
	if (!SystemState::IsInitialized()) {
		std::cout << "Run initialize before creating a screen.\n";
		return true;
	}

	const int argsCount = static_cast<int>(args.size());
	if (argsCount < 3 || argsCount > 3) {
		std::cout << "\033[31mUsage: screen -s <process_name> <process_mem_size>\033[0m\n";
		return true;
	}

	size_t requestedMemorySize = 0;
	if (!TryParseMemorySize(args[2], requestedMemorySize)) {
		std::cout << "\033[31mInvalid memory size: " << args[2] << "\033[0m\n";
		return true;
	}

	if (!IsValidPowerOfTwo(requestedMemorySize)) {
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
	if (!DummyProcessGenerator::GenerateOne(config, processName, requestedMemorySize)) {
		std::cout << "\033[31mFailed to create process: " << processName << "\033[0m\n";
		return true;
	}

	consoleManager->SwitchScreen(processName);
	return true;
}

std::string CreateScreenCommand::Name() const { return "screen -s"; }
std::string CreateScreenCommand::Description() const { return "Creates a new screen with an attached process."; }

bool CreateScreenCommand::IsValidPowerOfTwo(size_t size) const {
	constexpr size_t minLimit = 1 << 6;  // 64 bytes
	constexpr size_t maxLimit = 1 << 16; // 65536 bytes

	return size >= minLimit && size <= maxLimit && (size & (size - 1)) == 0;
}

bool CreateScreenCommand::TryParseMemorySize(const std::string& arg, size_t& outSize) const {
	try {
		size_t processedSize = 0;
		unsigned long long parsedSize = std::stoull(arg, &processedSize);
		
		if (processedSize != arg.size()) {
			return false; // Not a valid number
		}
		if (parsedSize > std::numeric_limits<size_t>::max()) {
			return false; // Out of range for size_t
		}
		
		outSize = static_cast<size_t>(parsedSize);
		return true;
	}
	catch(const std::invalid_argument&) {
		return false; // Not a valid number
	}
	catch (const std::out_of_range&) {
		return false; // Out of range for size_t
	}
	return false;
}
