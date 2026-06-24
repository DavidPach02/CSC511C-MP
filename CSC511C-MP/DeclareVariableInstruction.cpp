#include "DeclareVariableInstruction.h"

DeclareVariableInstruction::DeclareVariableInstruction(std::shared_ptr<Process> process, std::string varName, uint16_t defaultValue)
	: Instruction(process), varName(varName), defaultValue(defaultValue){
}

void DeclareVariableInstruction::Execute() {
	process->GetSymbolTable()->SetVariable(varName, defaultValue);
}
