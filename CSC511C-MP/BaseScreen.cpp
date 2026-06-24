#include "BaseScreen.h"
#include "ConsoleManager.h"
#include <iostream>

BaseScreen::BaseScreen(const std::string processName) : AConsole(processName) {
}

void BaseScreen::OnEnabled() {
}

void BaseScreen::Process() {
	std::string command = "";
	std::cout << "Enter command: ";
	std::getline(std::cin, command);

	if (command == "exit") {
		ConsoleManager::GetInstance()->ReturnToPreviousScreen();
		return;
	}

	if (command == "scheduler-start" || command == "scheduler-stop") {
		std::cout << command << " is only available on the main menu. Type exit to return.\n";
		return;
	}

	if (!command.empty()) {
		std::cout << "Unknown command. Available: exit\n";
	}
}

void BaseScreen::Display() {
}

void BaseScreen::PrintProcessInfo() const {
}
