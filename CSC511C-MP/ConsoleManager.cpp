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
	GetInstance();
}

void ConsoleManager::Destroy() {
	delete instance;
	instance = nullptr;
}

void ConsoleManager::Process() {
	if (this->currentConsole != nullptr) {
		this->currentConsole->Process();
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

	system("cls");
    this->previousConsole = currentConsole;
    this->currentConsole = screen->second; // Use .get() to convert shared_ptr to raw pointer
	this->currentConsole->OnEnabled();
}

void ConsoleManager::RegisterScreen(const std::shared_ptr<BaseScreen> screenRef) {
	if (this->screenTable.find(screenRef->GetName()) != this->screenTable.end()) {
		std::cout << "\033[31mScreen already registered: " << screenRef->GetName() << "\033[0m\n";
		return;
	}

	screenTable[screenRef->GetName()] = screenRef;
	std::cout << "Added screen: " << screenRef->GetName() << "\n";
	std::cout << "Current screens: " << screenTable.size() << "\n";
}

bool ConsoleManager::GetIsRunnning() const
{
	return running;
}

ConsoleManager::ConsoleManager() {
	this->running = true;

	// Create a map of screens
	const std::shared_ptr<MainConsole> mainConsole = std::make_shared<MainConsole>();

	screenTable[MAIN_CONSOLE_NAME] = mainConsole;

	this->SwitchScreen(mainConsole->GetName());
}