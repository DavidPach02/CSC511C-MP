#include "ReadInstruction.h"

#include "MemoryManager.h"

#include <limits>

namespace {
	bool TryParseAddress(const std::string& rawAddress, uint16_t& outAddress) {
		try {
			size_t parsedChars = 0;
			const unsigned long parsedValue = std::stoul(rawAddress, &parsedChars, 0);
			if (parsedChars != rawAddress.size()) {
				return false;
			}
			if (parsedValue > std::numeric_limits<uint16_t>::max()) {
				return false;
			}
			outAddress = static_cast<uint16_t>(parsedValue);
			return true;
		} catch (...) {
			return false;
		}
	}
}

ReadInstruction::ReadInstruction(std::shared_ptr<Process> process, std::string varName, std::string memoryAddress)
	: Instruction(process), varName(varName), memoryAddress(memoryAddress) {
}

void ReadInstruction::Execute(){
	uint16_t address = 0;
	if (!TryParseAddress(memoryAddress, address)) {
		std::string error = "Memory access violation: invalid READ address '" + memoryAddress + "'. Process terminated.";
		process->LogMessage(error);
		process->TerminateDueToMemoryAccessViolation(memoryAddress);
		return;
	}

	MemoryManager* memoryManager = MemoryManager::GetInstance();
	if (memoryManager == nullptr) {
		std::string error = "Memory manager unavailable during READ. Process terminated.";
		process->LogMessage(error);
		process->Terminate();
		return;
	}

	uint16_t value = 0;
	if (!memoryManager->ReadProcessMemory(process, address, value)) {
		std::string error = "Memory access violation: READ at address " + memoryAddress + ". Process terminated.";
		process->LogMessage(error);
		process->TerminateDueToMemoryAccessViolation(memoryAddress);
		return;
	}

	process->GetSymbolTable()->SetVariable(varName, value);
}
