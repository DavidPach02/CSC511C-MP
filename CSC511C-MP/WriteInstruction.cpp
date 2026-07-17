#include "WriteInstruction.h"

WriteInstruction::WriteInstruction(std::shared_ptr<Process> process, std::string memoryAddress, std::string defaultValue) 
	: Instruction(process), memoryAddress(memoryAddress), defaultValue(defaultValue){
}

void WriteInstruction::Execute() {
	// TODO: 
	// Operand handling for memory address
	// Operand handling for default value
	// Check for write access violation
	// If valid
	// Create new variable in the symbol table in the memory address provided
	// Assign value to new variable

	// REMOVE THIS 
	std::string message = "[DUMMY] Write instruction";
	process->LogMessage(message);
}
