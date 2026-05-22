#include "ScreenCommand.h"

bool ScreenCommand::Execute(const std::vector<std::string>& args) const {
	// TODO: Display all active sessions
	std::cout << Name() + " command recognized. Doing something.\n";
	return true;
}

std::string ScreenCommand::Name() const { return "screen"; }
std::string ScreenCommand::Description() const { return "Displays all active sessions."; }