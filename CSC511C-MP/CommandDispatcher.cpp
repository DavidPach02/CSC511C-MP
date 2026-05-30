#include "CommandDispatcher.h"
#include <iostream>
#include "Commands.h"
#include "ExitCommand.h"
#include "ClearCommand.h"
#include "InitializeCommand.h"
#include "ScreenCommand.h"
#include "ReportUtilCommand.h"
#include "HelpCommand.h"

void CommandDispatcher::Initialize() {
	this->Register(std::make_unique<InitializeCommand>());
	this->Register(std::make_unique<ClearCommand>());
	this->Register(std::make_unique<HelpCommand>(this));
	this->Register(std::make_unique<ScreenCommand>());
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

bool CommandDispatcher::DispatchCommand(const std::string& input) const {
	// TODO: Extend this to support command arguments in the future. For now, we assume commands have no arguments.
	std::string tokens = input;
	if (tokens.empty())
	{
		std::cout << "No command entered.\n";
		return true;
	}

	// Look up the command in the map
	auto it = m_commands.find(tokens);
	// If the command is not found, print an error message
	if (it == m_commands.end()) {
		std::cout << "\033[31mUnknown command: " << tokens << "\033[0m\n";
		// Return true to continue running the application even if the command is unknown
		return true;
	}

	std::vector<std::string> args; // No arguments for now, can be extended later
	// Execute the command and return its result
	return it->second->Execute(args);
}

void CommandDispatcher::PrintHelp() const {
	std::cout << "Available commands:\n";
	for (const auto& pair : m_commands) {
		std::cout << "  [ \033[33m" << pair.first << "\033[0m ] " << pair.second->Description() << "\n";
	}
}