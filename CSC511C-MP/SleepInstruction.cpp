#include "SleepInstruction.h"

#include <algorithm>
#include <string>

SleepInstruction::SleepInstruction(std::shared_ptr<Process> process, std::string sleepDuration)
	: Instruction(std::move(process)), sleepDuration{ std::move(sleepDuration) } {
}

void SleepInstruction::Execute() {
	LogExecution();

	const uint16_t resolvedDuration = sleepDuration.Resolve(process);
	const std::uint8_t duration = static_cast<std::uint8_t>(
		std::min(resolvedDuration, static_cast<uint16_t>(20)));

	process->SleepForTicks(duration);

	std::string message = "Sleeping for " + std::to_string(static_cast<int>(duration)) + " tick(s).";
	process->LogMessage(message);
}
