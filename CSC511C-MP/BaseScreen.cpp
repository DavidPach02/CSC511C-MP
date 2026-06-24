#include "BaseScreen.h"
#include "ConsoleManager.h"
#include <iostream>

BaseScreen::BaseScreen(std::shared_ptr<Process> process) : AConsole(process->GetName()), process(process) {
	commandDispatcher->Register(std::make_unique<ExitScreenCommand>());
	commandDispatcher->Register(std::make_unique<ClearCommand>());
	commandDispatcher->Register(std::make_unique<PrintLogsCommand>(this->process));
	commandDispatcher->Register(std::make_unique<HelpCommand>(commandDispatcher.get()));
}

void BaseScreen::OnEnabled() {
	DisplayHeader();
}

void BaseScreen::Update() {
	std::string command = "";
	std::cout << "Enter command: ";
	std::getline(std::cin, command);

	commandDispatcher->DispatchCommand(command);

	if (command == "clear") {
		refreshed = true;
	}
}

void BaseScreen::Display() {
	if (refreshed) {
		DisplayHeader();
		refreshed = false;
	}
}

void BaseScreen::PrintProcessInfo() const {
	this->process->PrintInfo();
}

void BaseScreen::DisplayHeader() const {
	std::cout << "Process Name: " << this->process->GetName() << std::endl;
	std::cout << "ID: " << this->process->GetID() << std::endl;
}
