#pragma once

#include "Instruction.h"
#include <string>

class ReadInstruction : public Instruction {
public:
	ReadInstruction(std::shared_ptr<Process> process, std::string varName, std::string memoryAddress);

	void Execute() override;
private:

	std::string varName;
	std::string memoryAddress;
};

