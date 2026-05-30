#include <iostream>
#include <string>
#include <cstdlib>
#include "Commands.h"
#include "CommandDispatcher.h"
#include "ICommand.h"
#include "ConsoleManager.h"

int main(){
	ConsoleManager::Initialize();

    bool running = true;
	// Main command loop
    while (running) {
		ConsoleManager::GetInstance()->Process();
		ConsoleManager::GetInstance()->Render();
		
		running = ConsoleManager::GetInstance()->GetIsRunnning();
    }

	ConsoleManager::Destroy();

    return 0;
}