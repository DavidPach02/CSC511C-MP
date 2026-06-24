#include "ConsoleManager.h"
#include <iostream>
#include "MainConsole.h"

ConsoleManager* ConsoleManager::instance = nullptr;
ConsoleManager* ConsoleManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ConsoleManager();
	}
	return instance;
}

void ConsoleManager::Initialize() {
	// Create a map of screens
	const std::shared_ptr<MainConsole> mainConsole = std::make_shared<MainConsole>();

	screenTable[MAIN_CONSOLE_NAME] = mainConsole;

	this->SwitchScreen(mainConsole->GetName());
}

void ConsoleManager::Destroy() {
	delete instance;
	instance = nullptr;
}

void ConsoleManager::Update() {
	if (this->currentConsole != nullptr) {
		this->currentConsole->Update();
	}
	
}

void ConsoleManager::Render() {
	if (this->currentConsole != nullptr) {
		this->currentConsole->Display();
	}
}

void ConsoleManager::Exit() {
	running = false;
}

void ConsoleManager::SwitchScreen(const std::string& name) {
    auto screen = screenTable.find(name);
    if (screen == screenTable.end()) {
        std::cout << "\033[31mScreen not found: " << name << "\033[0m\n";
        return;
    }

	// Clear the console screen using the system command.
	#ifdef _WIN32
        std::system("cls");     // Windows
    #else
        std::system("clear");   // Linux/macOS
    #endif
	
    this->previousConsole = currentConsole;
    this->currentConsole = screen->second; // Use .get() to convert shared_ptr to raw pointer
	this->currentConsole->OnEnabled();
}

bool ConsoleManager::HasScreen(const std::string& name) const {
	return screenTable.find(name) != screenTable.end();
}

bool ConsoleManager::IsOnMainConsole() const {
	return currentConsole != nullptr && currentConsole->GetName() == MAIN_CONSOLE_NAME;
}

void ConsoleManager::ReturnToPreviousScreen() {
	// If the current screen is the main screen, do not return to the previous screen.
	if (this->currentConsole != nullptr && this->currentConsole->GetName() == MAIN_CONSOLE_NAME) {
		return;
	}

	this->SwitchScreen(MAIN_CONSOLE_NAME);
}

void ConsoleManager::RegisterScreen(const std::shared_ptr<AConsole> screenRef, bool announce) {
	if (this->screenTable.find(screenRef->GetName()) != this->screenTable.end()) {
		if (announce) {
			std::cout << "\033[31mScreen already registered: " << screenRef->GetName() << "\033[0m\n";
		}
		return;
	}

	screenTable[screenRef->GetName()] = screenRef;
	if (announce) {
		std::cout << "Added screen: " << screenRef->GetName() << "\n";
		std::cout << "Current screens: " << screenTable.size() << "\n";
	}
}

void ConsoleManager::UnregisterScreen(const std::string& name) {
	if (name == MAIN_CONSOLE_NAME) {
		return;
	}

	if (screenTable.find(name) == screenTable.end()) {
		return;
	}

	if (currentConsole != nullptr && currentConsole->GetName() == name) {
		auto mainScreen = screenTable.find(MAIN_CONSOLE_NAME);
		if (mainScreen != screenTable.end()) {
			SwitchScreen(MAIN_CONSOLE_NAME);
		}
	}

	screenTable.erase(name);
}

bool ConsoleManager::GetIsRunnning() const
{
	return running;
}

ConsoleManager::ConsoleManager() {
	this->running = true;
}