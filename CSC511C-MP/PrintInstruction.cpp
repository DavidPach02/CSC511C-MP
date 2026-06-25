#include "PrintInstruction.h"

PrintInstruction::PrintInstruction(std::shared_ptr<Process> process, std::string message, std::string varName)
	: Instruction(process), message(message), varName(varName){
}

void PrintInstruction::Execute() {
	if (!process) {
		return;
	}

	LogExecution();

	std::string processedMessage = message;

	size_t placeHolderPos = processedMessage.find("%i");
	if (placeHolderPos != std::string::npos) {
		uint16_t varValue = process->GetSymbolTable()->GetVariable(varName);
		std::string valueString = std::to_string(varValue);

		processedMessage.replace(placeHolderPos, 2, valueString);
	}
	
	std::string finalMessage = "(" + this->executionDate + " " + this->executionTime + ") Core: " +
		std::to_string(this->executionCore) + " \"" + processedMessage + "\"";

	process->LogMessage(finalMessage);
}
