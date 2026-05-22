#include "ClearCommand.h"

bool ClearCommand::Execute(const std::vector<std::string>& args) const  {
	// Clear the console screen using the system command.
	std::system("cls");
	return true;
}

std::string ClearCommand::Name() const { return "clear"; }
std::string ClearCommand::Description() const { return "Clears the screen."; }