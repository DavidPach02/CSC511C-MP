#include "PrebootScreen.h"

#include <iostream>

#ifdef _WIN32
#include <conio.h>
#endif

PrebootScreen::PrebootScreen() : AConsole("PREBOOT_SCREEN") {
	commandDispatcher->Register(std::make_unique<InitializeCommand>());
	commandDispatcher->Register(std::make_unique<HelpCommand>(this->commandDispatcher.get()));
}

void PrebootScreen::OnEnabled() {
	PrintPrompt();
}

void PrebootScreen::Update() {
	std::string command = "";
	std::cout << "Enter command: ";
	std::getline(std::cin, command);

	if (commandDispatcher->DispatchCommand(command) && command != "initialize") {
		if (command == "exit") {
			std::cout << "Closing console." << std::endl;
			isExitConsole = true;
			return;
		}
		std::cout << "Press any key to continue.";

#ifdef _WIN32
		_getch();
		std::system("cls");     // Windows
#else
		std::cin.get();
		std::system("clear");   // Linux/macOS
#endif

		refreshed = true;
	}
}

void PrebootScreen::Display() {
	if (refreshed) {
		PrintPrompt();
		refreshed = false;
	}
}

void PrebootScreen::PrintPrompt() const {
	std::cout << "Type '\033[33minitialize\033[0m' to start the emulator.\n";
}
