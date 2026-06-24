#include "SleepInstruction.h"

#include <string>

SleepInstruction::SleepInstruction(std::shared_ptr<Process> process, std::uint8_t sleepDuration)
	: Instruction(std::move(process)), sleepDuration(sleepDuration) {
}

void SleepInstruction::Execute() {
	LogExecution();

	process->SleepForTicks(sleepDuration);

	std::string message = "Sleeping for " + std::to_string(static_cast<int>(sleepDuration)) + " tick(s).";
	process->LogMessage(message);
}
