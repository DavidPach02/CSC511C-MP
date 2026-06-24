#include "AConsole.h"

AConsole::AConsole(const std::string processName) : name(processName) {
	commandDispatcher = std::make_unique<CommandDispatcher>();
	commandDispatcher->Initialize();
}

std::string AConsole::GetName() const {
	return this->name;
}