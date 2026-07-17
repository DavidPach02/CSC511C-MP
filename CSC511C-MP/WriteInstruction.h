#pragma once

#include "Instruction.h"
#include <string>

class WriteInstruction : public Instruction {
public:
	WriteInstruction(std::shared_ptr<Process> process, std::string memoryAddress, std::string defaultValue);

	void Execute() override;
private:

	std::string memoryAddress;
	std::string defaultValue;
};

