#include "ExitCommand.h"

bool ExitCommand::Execute(const std::vector<std::string>& args) const {
		// Print a shutdown message and wait for a moment before exiting to give the user feedback that the command was recognized.
	std::cout << "Shutting down system.";
	Sleep(2000);
	return false;
}

std::string ExitCommand::Name() const { return "exit"; }
std::string ExitCommand::Description() const { return "Shuts down the system."; }