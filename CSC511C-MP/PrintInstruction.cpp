#include "PrintInstruction.h"

PrintInstruction::PrintInstruction(std::shared_ptr<Process> process, std::string message, std::string varName)
	: Instruction(process), message(message), varName(varName){
}

void PrintInstruction::Execute() {
	if (!process) {
		return;
	}

	LogExecution();

	// TODO: Add extension for variable support.
	std::string finalMessage = "(" + this->executionDate + " " + this->executionTime + ") Core: " +
		std::to_string(this->executionCore) + " \"" + message + "\"";
	process->LogMessage(finalMessage);
}
