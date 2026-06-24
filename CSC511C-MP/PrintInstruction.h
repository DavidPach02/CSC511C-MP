#pragma once

#include "Instruction.h"
#include <string>
#include <memory>

class PrintInstruction : public Instruction {
public:
	PrintInstruction(std::shared_ptr<Process> process, std::string message, std::string varName = "");

	void Execute() override;
private:
	std::string message;
	std::string varName;
};

