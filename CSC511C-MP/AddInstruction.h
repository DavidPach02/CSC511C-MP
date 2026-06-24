#pragma once

#include "Instruction.h"
#include "Operand.h"

class AddInstruction : public Instruction{
public:
	AddInstruction(std::shared_ptr<Process> process, const std::string& destination, const std::string& op1, const std::string& op2);

	void Execute() override;
private:
	std::string destinationVar;
	Operand operand1;
	Operand operand2;
};

