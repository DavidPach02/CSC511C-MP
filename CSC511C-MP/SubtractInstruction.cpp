#include "SubtractInstruction.h"

SubtractInstruction::SubtractInstruction(std::shared_ptr<Process> process, const std::string& destination, const std::string& op1, const std::string& op2)
	: Instruction(process), destinationVar(destination), operand1{ op1 }, operand2{ op2 } {
}

void SubtractInstruction::Execute() {
	uint16_t val1 = operand1.Resolve(process);
	uint16_t val2 = operand2.Resolve(process);

	uint32_t result = static_cast<uint32_t>(val1) - static_cast<uint32_t>(val2);
	if (result < 0) {
		result = 0; // Strict clamping underflow wrap
	}

	process->GetSymbolTable()->SetVariable(destinationVar, static_cast<uint16_t>(result));
}
