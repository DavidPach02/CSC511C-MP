#pragma once

#include "ICommand.h"
#include "CommandDispatcher.h"
#include <iostream>
#include <string>

class CommandDispatcher;

class HelpCommand : public ICommand {
public:
	// The constructor takes a pointer to the CommandDispatcher so that it can call PrintHelp() when executed.
	explicit HelpCommand(const CommandDispatcher* dispatcher);

	bool Execute(const std::vector<std::string>& args) const override;
	std::string Name() const override;
	std::string Description() const override;
private:
	// A pointer to the CommandDispatcher.
	const CommandDispatcher* m_dispatcher;
};