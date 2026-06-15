#include <iostream>
#include <string>
#include <cstdlib>
#include "ConsoleManager.h"
#include "CPUManager.h"

int main(){
	ConsoleManager::Initialize();
	CPUManager::Initialize(4);

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