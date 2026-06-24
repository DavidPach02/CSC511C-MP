#pragma once

#include "ICommand.h"
#include "IHelpDisplayable.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class CommandDispatcher : public IHelpDisplayable {
public:
	~CommandDispatcher() = default;

	// Initializes the dispatcher with available commands. This should be called once at the start of the application.
	void Initialize();
	// Registers a new command with the dispatcher. The command is identified by its Name() method.
	void Register(std::unique_ptr<ICommand> command);
	// Dispatches the input command to the appropriate ICommand implementation. Returns false if the command signals to exit the application.
	bool DispatchCommand(const std::string& input) const;
	// Prints the help text for all registered commands.
	void PrintHelp() const override;

private:
	// TODO: Extend this to support command arguments in the future. For now, we assume commands have no arguments.
	std::vector<std::string> Tokenize(const std::string& input) const;
	// Maps command names to their corresponding ICommand implementations.
	std::unordered_map<std::string, std::unique_ptr<ICommand>> m_commands;
};