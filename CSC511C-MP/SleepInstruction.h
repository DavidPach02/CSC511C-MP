#pragma once

#include "Instruction.h"
#include "Operand.h"
#include <string>

class SleepInstruction : public Instruction {
public:
	SleepInstruction(std::shared_ptr<Process> process, std::string sleepDuration);

	void Execute() override;
private:
	Operand sleepDuration;
};
