#include "ForInstruction.h"

ForInstruction::ForInstruction(std::shared_ptr<Process> process, std::vector<std::unique_ptr<Instruction>> instructions, std::string repeatCount)
	: Instruction(process), repeatCount{ repeatCount }, instructions(std::move(instructions)) {
}

void ForInstruction::Execute() {
	uint16_t iterations = repeatCount.Resolve(process);

	for (uint16_t i = 0; i < iterations; i++) {
		for (auto& instruction : instructions) {
			if (instruction != nullptr) {
				instruction->Execute();
			}
		}
	}
}
