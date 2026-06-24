#include "AConsole.h"


// 4. [JV] Remove autogeneration of process on start -> Scheduler-Start+Scheduler-Stop, 
// CPU Ticker, Custom Thread, CPU Worker, config.txt, Process Manager

AConsole::AConsole(const std::string processName) : name(processName) {
	commandDispatcher = std::make_unique<CommandDispatcher>();
	commandDispatcher->Initialize();
}

std::string AConsole::GetName() const {
	return this->name;
}