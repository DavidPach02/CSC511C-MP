#include "WriteInstruction.h"

#include "MemoryManager.h"

#include <algorithm>
#include <limits>
#include <string>

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

	bool TryResolveValueOperand(const std::shared_ptr<Process>& process, const std::string& rawValue, uint16_t& outValue) {
		try {
			size_t parsedChars = 0;
			const unsigned long parsedValue = std::stoul(rawValue, &parsedChars, 0);
			if (parsedChars == rawValue.size()) {
				const unsigned long clamped = std::min<unsigned long>(parsedValue, std::numeric_limits<uint16_t>::max());
				outValue = static_cast<uint16_t>(clamped);
				return true;
			}
		} catch (...) {
		}

		if (process == nullptr || process->GetSymbolTable() == nullptr) {
			return false;
		}

		const int symbolValue = process->GetSymbolTable()->GetVariable(rawValue);
		const int clamped = std::clamp(symbolValue, 0, 65535);
		outValue = static_cast<uint16_t>(clamped);
		return true;
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

	uint16_t value = 0;
	if (!TryResolveValueOperand(process, defaultValue, value)) {
		std::string error = "Memory access violation: invalid WRITE value operand '" + defaultValue + "'. Process terminated.";
		process->LogMessage(error);
		process->Terminate();
		return;
	}

	MemoryManager* memoryManager = MemoryManager::GetInstance();
	if (memoryManager == nullptr) {
		std::string error = "Memory manager unavailable during WRITE. Process terminated.";
		process->LogMessage(error);
		process->Terminate();
		return;
	}

	const MemoryAccessResult accessResult = memoryManager->WriteProcessMemory(process, address, value);
	if (accessResult == MemoryAccessResult::PageFaultRetry) {
		return;
	}
	if (accessResult == MemoryAccessResult::AccessViolation) {
		std::string error = "Memory access violation: WRITE at address " + memoryAddress + ". Process terminated.";
		process->LogMessage(error);
		process->TerminateDueToMemoryAccessViolation(memoryAddress);
		return;
	}
}
