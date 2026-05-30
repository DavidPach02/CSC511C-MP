#pragma once

#include <string>
#include <vector>

class ICommand {
public:
	virtual ~ICommand() = default;
	// Executes the command with the given arguments. Returns false if the command signals to exit the application.
	virtual bool Execute(const std::vector<std::string>& args) const = 0;
	// Returns the name of the command (e.g., "exit", "clear").
	virtual std::string Name() const = 0;
	// Returns a brief description of what the command does, used for help text.
	virtual std::string Description() const = 0;
	// Stores the line to match the command
	virtual std::string Argument() const { return ""; }
};
