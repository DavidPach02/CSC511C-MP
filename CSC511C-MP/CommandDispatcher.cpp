#include "CommandDispatcher.h"
#include <iostream>

void CommandDispatcher::Register(std::unique_ptr<ICommand> command) {
	if (!command) return;
	std::string name = command->Name();
	m_commands.emplace(name, std::move(command));
}

bool CommandDispatcher::DispatchCommand(const std::string& input) const {
	std::string tokens = input;
	if (tokens.empty())
	{
		std::cout << "No command entered.\n";
		return true;
	}

	auto it = m_commands.find(tokens);
	if (it == m_commands.end()) {
		std::cout << "Unknown command: " << tokens << "\n";
		return true;
	}

	std::vector<std::string> args; // No arguments for now, can be extended later
	return it->second->Execute(args);
}

void CommandDispatcher::PrintHelp() const {
	std::cout << "Available commands:\n";
	for (const auto& pair : m_commands) {
		std::cout << "  [" << pair.first << "] " << pair.second->Description() << "\n";
	}
}