#include "ReadInstruction.h"

ReadInstruction::ReadInstruction(std::shared_ptr<Process> process, std::string varName, std::string memoryAddress)
	: Instruction(process), varName(varName), memoryAddress(memoryAddress) {
}

void ReadInstruction::Execute(){
	// TODO: 
	// Operand handling for memory address
	// Check for read access violation
	// If valid
	// Retrieval of value from symbol table using mem address
	// Create new variable in the symbol table
	// Assign value to new variable

	// REMOVE THIS 
	std::string message = "[DUMMY] Read instruction";
	process->LogMessage(message);
}
