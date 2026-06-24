#include "ExitScreenCommand.h"

bool ExitScreenCommand::Execute(const std::vector<std::string>& args) const {
	// Print a shutdown message and wait for a moment before exiting to give the user feedback that the command was recognized.
	ConsoleManager::GetInstance()->ReturnToPreviousScreen();
	return false;
}

std::string ExitScreenCommand::Name() const { return "exit"; }
std::string ExitScreenCommand::Description() const { return "Returns to the main console."; }