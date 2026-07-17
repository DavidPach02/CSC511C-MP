#include "InstructionBuilder.h"
#include "PrintInstruction.h"
#include "DeclareVariableInstruction.h"
#include "AddInstruction.h"
#include "SubtractInstruction.h"
#include "ForInstruction.h"
#include "SleepInstruction.h"
#include "ReadInstruction.h"
#include "WriteInstruction.h"

ParsedInstruction InstructionBuilder::ParseInstruction(const std::string& instruction) {
	ParsedInstruction result;
	std::string trimmed = Trim(instruction);

	size_t openParen = trimmed.find('(');
	size_t closeParen = trimmed.find_last_of(')');

	// 1. Is it a nested/function style command? e.g., FOR(...) or PRINT(...)
	if (openParen != std::string::npos && closeParen != std::string::npos && closeParen > openParen) {
		result.command = Trim(trimmed.substr(0, openParen));
		result.isNested = true;

		// Extract just what is INSIDE the parentheses
		// e.g., "5, PRINT('Hello, World'), ADD varA varA varB"
		std::string innerArgs = trimmed.substr(openParen + 1, closeParen - openParen - 1);

		// Split inner contents by comma, respecting internal single-quoted strings and parens
		result.args = SplitBy(innerArgs, ',');
	}
	// 2. Is it a flat space-separated command? e.g., DECLARE varA 10
	else {
		size_t spacePos = trimmed.find(' ');
		if (spacePos != std::string::npos) {
			result.command = trimmed.substr(0, spacePos);

			// Extract the remainder of the string after the command name
			std::string remainingArgs = trimmed.substr(spacePos + 1);

			// Split flat arguments by space (disable paren checks to treat strings uniformly)
			result.args = SplitBy(remainingArgs, ' ', false);
		}
		else {
			// Command with no arguments (e.g., "HALT")
			result.command = trimmed;
		}
	}

	return result;
}

bool InstructionBuilder::BuildInstructionsFromString(const std::string& instructionString, std::shared_ptr<Process> process) {
	std::vector<std::string> instructionLines = SplitBy(instructionString, ';', true);

	for (const std::string& line : instructionLines) {
		ParsedInstruction parsed = ParseInstruction(line);
		//std::cout << "Command: " << parsed.command << std::endl;

		std::unique_ptr<Instruction> instruction = CreateInstruction(parsed, process);
		if (!instruction) {
			return false;
		}

		process->AddInstruction(std::move(instruction));
	}

	return true;
}

std::unique_ptr<Instruction> InstructionBuilder::CreateInstruction(ParsedInstruction& parsed, std::shared_ptr<Process> process) {
	if (parsed.command == "DECLARE") {
		if (parsed.args.size() != 2) {
			std::cout << "\033[31mInvalid DECLARE instruction format.Expected: DECLARE <var_name> <default_value>\033[0m\n";
			return nullptr;
		}
		//std::cout << "Creating DECLARE instruction for variable: " << parsed.args[0] << " with default value: " << parsed.args[1] << std::endl;
		return std::make_unique<DeclareVariableInstruction>(process, parsed.args[0], static_cast<uint16_t>(std::stoi(parsed.args[1])));
	}
	else if (parsed.command == "ADD") {
		if (parsed.args.size() != 3) {
			std::cout << "\033[31mInvalid ADD instruction format. Expected: ADD <var_name> <operand1> <operand2>\033[0m\n";
			return nullptr;
		}
		//std::cout << "Creating ADD instruction for variable: " << parsed.args[0] << " with operands: " << parsed.args[1] << ", " << parsed.args[2] << std::endl;
		return std::make_unique<AddInstruction>(process, parsed.args[0], parsed.args[1], parsed.args[2]);
	}
	else if (parsed.command == "SUBTRACT") {
		if (parsed.args.size() != 3) {
			std::cout << "\033[31mInvalid SUBTRACT instruction format. Expected: SUBTRACT <var_name> <operand1> <operand2>\033[0m\n";
			return nullptr;
		}
		//std::cout << "Creating SUBTRACT instruction for variable: " << parsed.args[0] << " with operands: " << parsed.args[1] << ", " << parsed.args[2] << std::endl;
		return std::make_unique<SubtractInstruction>(process, parsed.args[0], parsed.args[1], parsed.args[2]);
	}
	else if (parsed.command == "PRINT") {
		if (parsed.args.empty()) {
			std::cout << "\033[31mInvalid PRINT instruction format. Expected: PRINT('<message>', [<var_name>])\033[0m\n";
			return nullptr;
		}
		//std::cout << "Creating PRINT instruction with message: " << parsed.args[0] << std::endl;
		return std::make_unique<PrintInstruction>(process, parsed.args[0], parsed.args.size() > 1 ? parsed.args[1] : "");
	}
	else if (parsed.command == "READ") {
		if (parsed.args.size() != 2) {
			std::cout << "\033[31mInvalid READ instruction format.Expected: READ <var_name> <memory_address>\033[0m\n";
			return nullptr;
		}
		return std::make_unique<ReadInstruction>(process, parsed.args[0], parsed.args[1]);
	}
	else if (parsed.command == "WRITE") {
		if (parsed.args.size() != 2) {
			std::cout << "\033[31mInvalid WRITE instruction format.Expected: READ <memory_address> <default_value>\033[0m\n";
			return nullptr;
		}
		return std::make_unique<WriteInstruction>(process, parsed.args[0], parsed.args[1]);
	}
	else if (parsed.command == "SLEEP") {
		if (parsed.args.size() != 1) {
			std::cout << "\033[31mInvalid SLEEP instruction format. Expected: SLEEP <duration>\033[0m\n";
			return nullptr;
		}
		//std::cout << "Creating SLEEP instruction with duration: " << parsed.args[0] << std::endl;
		return std::make_unique<SleepInstruction>(process, parsed.args[0]);
	}
	else if (parsed.command == "FOR") {
		if (parsed.args.size() < 2) {
			std::cout << "\033[31mInvalid FOR instruction format. Expected: FOR(<iterations>, <instruction1>, [<instruction2>, ...])\033[0m\n";
			return nullptr;
		}
		
		//std::cout << "Creating FOR instruction with iterations: " << parsed.args[0] << std::endl;

		// Create a vector to hold the body of the FOR loop
		std::vector<std::unique_ptr<Instruction>> forBody;
		for (size_t i = 1; i < parsed.args.size(); ++i) {
			ParsedInstruction nestedParsed = ParseInstruction(parsed.args[i]);
			forBody.push_back(CreateInstruction(nestedParsed, process));
		}
		return std::make_unique<ForInstruction>(process, std::move(forBody), parsed.args[0]);
	}

	return nullptr;
}

// Helper function to trim leading and trailing whitespace
std::string InstructionBuilder::Trim(const std::string& str) {
	size_t first = str.find_first_not_of(" \t\n\r");
	if (first == std::string::npos) return "";
	size_t last = str.find_last_not_of(" \t\n\r");
	return str.substr(first, (last - first + 1));
}

std::vector<std::string> InstructionBuilder::SplitBy(const std::string& input, char delimiter, bool respectParentheses) {
	std::vector<std::string> tokens;
	std::string currentToken;
	int parenDepth = 0;

	// Track double quotes and single quotes separately
	bool inDoubleQuotes = false;
	bool inSingleQuotes = false;
	bool escaped = false;

	for (size_t i = 0; i < input.length(); ++i) {
		char c = input[i];

		if (escaped) {
			currentToken += c;
			escaped = false;
		}
		else if (c == '\\') {
			currentToken += c;
			escaped = true;
		}
		else if (c == '"' && !inSingleQuotes) {
			// Toggle double quotes only if we aren't currently inside single quotes
			currentToken += c;
			inDoubleQuotes = !inDoubleQuotes;
		}
		else if (c == '\'' && !inDoubleQuotes) {
			// Toggle single quotes only if we aren't currently inside double quotes
			currentToken += c;
			inSingleQuotes = !inSingleQuotes;
		}
		else if (c == '(' && !inDoubleQuotes && !inSingleQuotes) {
			currentToken += c;
			if (respectParentheses) ++parenDepth;
		}
		else if (c == ')' && !inDoubleQuotes && !inSingleQuotes) {
			currentToken += c;
			if (respectParentheses) --parenDepth;
		}
		else if (c == delimiter && !inDoubleQuotes && !inSingleQuotes && parenDepth == 0) {
			// Split boundary reached safely outside of any quotes or parentheses!
			std::string trimmed = Trim(currentToken);
			if (!trimmed.empty()) {
				tokens.push_back(trimmed);
			}
			currentToken.clear();
		}
		else {
			currentToken += c;
		}
	}

	std::string trimmed = Trim(currentToken);
	if (!trimmed.empty()) {
		tokens.push_back(trimmed);
	}

	return tokens;
}