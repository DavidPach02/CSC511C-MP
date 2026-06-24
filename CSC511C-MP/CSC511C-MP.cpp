#include <iostream>
#include "ConsoleManager.h"
#include "Scheduler.h"
#include "CPUTicker.h"
#include "CPUManager.h"
#include "ProcessManager.h"
#include "SystemState.h"

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	ConsoleManager::Initialize();

	bool running = true;
	while (running) {
		ConsoleManager::GetInstance()->Process();
		ConsoleManager::GetInstance()->Render();
		running = ConsoleManager::GetInstance()->GetIsRunnning();
	}

	CPUTicker::Stop();
	Scheduler::Destroy();
	ConsoleManager::Destroy();
	CPUManager::Destroy();
	ProcessManager::Destroy();
	SystemState::Shutdown();

	return 0;
}
