#include "ExitCommand.h"
#include <thread>

bool ExitCommand::Execute(const std::vector<std::string>& args) const {
		// Print a shutdown message and wait for a moment before exiting to give the user feedback that the command was recognized.
	std::cout << "Shutting down system.";
	std::this_thread::sleep_for(std::chrono::seconds(2));
	return false;
}

std::string ExitCommand::Name() const { return "exit"; }
std::string ExitCommand::Description() const { return "Shuts down the system."; }