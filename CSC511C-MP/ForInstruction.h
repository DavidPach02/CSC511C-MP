#pragma once

#include "Instruction.h"
#include "Operand.h"
#include <vector>
#include <cstdint>

class ForInstruction : public Instruction {
public:
	ForInstruction(std::shared_ptr<Process> process, std::vector<std::unique_ptr<Instruction>> instructions, std::string repeatCount);

	void Execute() override;
private:
	Operand repeatCount;
	std::vector<std::unique_ptr<Instruction>> instructions;
};

