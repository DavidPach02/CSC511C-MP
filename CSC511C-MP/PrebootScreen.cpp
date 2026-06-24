#include "PrebootScreen.h"

PrebootScreen::PrebootScreen() : AConsole("PREBOOT_SCREEN") {
	commandDispatcher->Register(std::make_unique<InitializeCommand>());
	//commandDispatcher->Register(std::make_unique<HelpCommand>(this->commandDispatcher));
}

void PrebootScreen::OnEnabled() {
	PrintPrompt();
}

void PrebootScreen::Update() {
	std::string command = "";
	std::cout << "Enter command: ";
	std::getline(std::cin, command);

	if (commandDispatcher->DispatchCommand(command) && command != "initialize") {
		std::cout << "Press any key to continue.";
		_getch();

#ifdef _WIN32
		std::system("cls");     // Windows
#else
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
