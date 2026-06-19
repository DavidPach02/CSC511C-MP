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
}

void BaseScreen::Display() {
}

void BaseScreen::PrintProcessInfo() const {
}
