#include "HelpCommand.h"

// The constructor takes a pointer to the CommandDispatcher so that it can call PrintHelp() when executed.
HelpCommand::HelpCommand(const CommandDispatcher* dispatcher) : m_dispatcher(dispatcher) {}

bool HelpCommand::Execute(const std::vector<std::string>& args) const {
	if (m_dispatcher) m_dispatcher->PrintHelp();
	return true;
}

std::string HelpCommand::Name() const { return "help"; }
std::string HelpCommand::Description() const { return "Displays the available commands."; }