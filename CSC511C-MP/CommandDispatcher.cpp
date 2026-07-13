#include "CommandDispatcher.h"
#include <iostream>
#include "ExitCommand.h"
#include "ClearCommand.h"
#include "InitializeCommand.h"
#include "ScreenCommand.h"
#include "ReportUtilCommand.h"
#include "HelpCommand.h"
#include "SchedulerStartCommand.h"
#include "SchedulerStopCommand.h"
#include "DisplayMemoryCommand.h"
#include "DisplayVirtualMemCommand.h"
#include "SystemState.h"

void CommandDispatcher::Initialize() {
	this->Register(std::make_unique<InitializeCommand>());
	this->Register(std::make_unique<ClearCommand>());
	this->Register(std::make_unique<HelpCommand>(this));
	this->Register(std::make_unique<ScreenCommand>());
	this->Register(std::make_unique<DisplayMemoryCommand>());
	this->Register(std::make_unique<DisplayVirtualMemCommand>());
	this->Register(std::make_unique<SchedulerStartCommand>());
	this->Register(std::make_unique<SchedulerStopCommand>());
	this->Register(std::make_unique<ReportUtilCommand>());
	this->Register(std::make_unique<ExitCommand>());
}

void CommandDispatcher::Register(std::unique_ptr<ICommand> command) {
	if (!command) return;
	// Get the command name
	std::string name = command->Name();
	// Add the command to the map, replacing any existing command with the same name
	m_commands.emplace(name, std::move(command));
}

std::vector<std::string> CommandDispatcher::Tokenize(const std::string& input) const {
	std::vector<std::string> tokens;
	size_t index = 0;
	const size_t length = input.length();

	while (index < length) {
		// Skip leading whitespace
		while (index < length && std::isspace(input[index])) {
			++index;
		}
		if (index >= length) break;

		// Find the end of the token
		size_t start = index;
		while (index < length && !std::isspace(input[index])) {
			++index;
		}
		tokens.push_back(input.substr(start, index - start));
	}

	return tokens;
}

bool CommandDispatcher::DispatchCommand(const std::string& input) const {
	// TODO: Extend this to support command arguments in the future. For now, we assume commands have no arguments.
	std::vector<std::string> tokens = this->Tokenize(input);
	if (tokens.empty())
	{
		std::cout << "No command entered.\n";
		return true;
	}

	const std::string& command = tokens[0];

	if (!SystemState::IsInitialized()
		&& command != "initialize"
		&& command != "exit"
		&& command != "help"
		&& command != "clear") {
		std::cout << "Run initialize before using other commands.\n";
		return true;
	}

	// Look up the command in the map
	auto it = m_commands.find(command);
	// If the command is not found, print an error message
	if (it == m_commands.end()) {
		std::cout << "\033[31mUnknown command: " << command << "\033[0m\n";
		// Return true to continue running the application even if the command is unknown
		return true;
	}

	// For debugging
	//std::cout << "Processing command: " << command << " \n";

	std::vector<std::string> args; 
	if (tokens.size() > 1) {
		args.assign(tokens.begin() + 1, tokens.end());

		// For debugging
		/*std::cout << "Args: ";

		for (size_t i = 0; i < args.size(); i++) {
			std::cout << args[i] << ", ";
		}
		std::cout << "\n";*/
	}
	
	return it->second->Execute(args);
}

void CommandDispatcher::PrintHelp() const {
	std::cout << "Available commands:\n";
	for (const auto& pair : m_commands) {
		std::cout << "  [ \033[33m" << pair.first << "\033[0m ] " << pair.second->Description() << "\n";
	}
}