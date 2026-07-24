#include "WriteInstruction.h"

#include "MemoryManager.h"
#include "Operand.h"

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

WriteInstruction::WriteInstruction(std::shared_ptr<Process> process, std::string memoryAddress, std::string defaultValue) 
	: Instruction(process), memoryAddress(memoryAddress), defaultValue(defaultValue){
}

void WriteInstruction::Execute() {
	uint16_t address = 0;
	if (!TryParseAddress(memoryAddress, address)) {
		std::string error = "Memory access violation: invalid WRITE address '" + memoryAddress + "'. Process terminated.";
		process->LogMessage(error);
		process->TerminateDueToMemoryAccessViolation(memoryAddress);
		return;
	}

	Operand valueOperand{ defaultValue };
	const uint16_t value = valueOperand.Resolve(process);

	MemoryManager* memoryManager = MemoryManager::GetInstance();
	if (memoryManager == nullptr) {
		std::string error = "Memory manager unavailable during WRITE. Process terminated.";
		process->LogMessage(error);
		process->Terminate();
		return;
	}

	if (!memoryManager->WriteProcessMemory(process, address, value)) {
		std::string error = "Memory access violation: WRITE at address " + memoryAddress + ". Process terminated.";
		process->LogMessage(error);
		process->TerminateDueToMemoryAccessViolation(memoryAddress);
		return;
	}
}
