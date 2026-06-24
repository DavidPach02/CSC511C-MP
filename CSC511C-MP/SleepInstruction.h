#pragma once

#include "Instruction.h"
#include <cstdint>

class SleepInstruction : public Instruction{
public:
	SleepInstruction(std::shared_ptr<Process> process, std::uint8_t sleepDuration);

	void Execute() override;
private:
    std::uint8_t sleepDuration;
};
