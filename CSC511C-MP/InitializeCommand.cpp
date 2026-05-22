#include "InitializeCommand.h"

bool InitializeCommand::Execute(const std::vector<std::string>& args) const {
	// TODO: Boot up the OS with this command.
	std::cout << Name() + " command recognized. Doing something.\n";
	return true;
}

std::string InitializeCommand::Name() const { return "initialize"; }
std::string InitializeCommand::Description() const { return "Boots up the OS simulator."; }