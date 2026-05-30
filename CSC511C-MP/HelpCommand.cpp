#include "HelpCommand.h"

// The constructor takes a pointer to the CommandDispatcher so that it can call PrintHelp() when executed.
HelpCommand::HelpCommand(const IHelpDisplayable* targetDisplayable) : helpDisplayable(targetDisplayable) {}

bool HelpCommand::Execute(const std::vector<std::string>& args) const {
	if (helpDisplayable) helpDisplayable->PrintHelp();
	return true;
}

std::string HelpCommand::Name() const { return "help"; }
std::string HelpCommand::Description() const { return "Displays the available commands and/or subcommands."; }
std::string HelpCommand::Argument() const { return "help"; }