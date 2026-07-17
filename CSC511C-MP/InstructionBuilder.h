#pragma once

#include <vector>
#include <memory>
#include <string>
#include "Instruction.h"

struct ParsedInstruction {
	std::string command;
	std::vector<std::string> args;
	bool isNested = false;
};

class InstructionBuilder {
public:
	static bool BuildInstructionsFromString(const std::string& instructionString, std::shared_ptr<Process> process);
	static std::unique_ptr<Instruction> CreateInstruction(ParsedInstruction& parsed, std::shared_ptr<Process> process);
	static std::string Trim(const std::string& str);
	static std::vector<std::string> SplitBy(const std::string& input, char delimiter, bool respectParentheses = true);
	static ParsedInstruction ParseInstruction(const std::string& instruction);
};

