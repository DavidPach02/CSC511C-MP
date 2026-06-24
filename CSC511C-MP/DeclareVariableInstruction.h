#pragma once

#include "Instruction.h"
#include <string>
#include <cstdint>
#include <memory>

class DeclareVariableInstruction : public Instruction {
public:
	DeclareVariableInstruction(std::shared_ptr<Process> process, std::string varName, uint16_t defaultValue = 0);

	void Execute() override;
private:
	std::string varName;
	uint16_t defaultValue;
};

