#include "Instruction.h"

Instruction::Instruction(std::shared_ptr<Process> process) : process(process) {
}

std::string Instruction::GetExecutionDate() const
{
	return this->executionDate;
}

std::string Instruction::GetExecutionTime() const
{
	return this->executionTime;
}

int Instruction::GetExecutionCore() const
{
	return this->executionCore;
}

void Instruction::LogExecution() {
	executionDate = TimeUtility::GetCurrentDateString("/");
	executionTime = TimeUtility::GetCurrentTimeString(false, ":");
	executionCore = process->GetCoreID();
}
