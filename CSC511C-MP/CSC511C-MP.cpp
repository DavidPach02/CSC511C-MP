#include <iostream>
#include <thread>
#include <chrono>
#include "ConsoleManager.h"
#include "Scheduler.h"
#include "CPUTicker.h"
#include "CPUManager.h"
#include "ProcessManager.h"
#include "MemoryManager.h"
#include "BackingStore.h"
#include "Process.h"
#include "SystemState.h"
#include "PrebootScreen.h"

namespace {
	// Safe only once the scheduler's workers are joined and no process is mid-instruction.
	void FinalizeUnfinishedProcesses() {
		for (const std::shared_ptr<Process>& process : ProcessManager::GetInstance()->GetAllProcesses()) {
			if (process != nullptr && process->GetStatusEnum() != ProcessStatus::Terminated) {
				process->Terminate();
			}
		}

		// Cleared in one pass; releasing per process rewrites the whole store file each time.
		BackingStore::ResetStore();
	}
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	std::shared_ptr<PrebootScreen> prebootScreen = std::make_shared<PrebootScreen>();
	ConsoleManager::GetInstance()->RegisterScreen(prebootScreen, false);
	ConsoleManager::GetInstance()->SwitchScreen(prebootScreen->GetName());

	while (!SystemState::IsInitialized()) {
		ConsoleManager::GetInstance()->Update();
		ConsoleManager::GetInstance()->Render();

		if (prebootScreen->IsExitConsole()) {
			ConsoleManager::Destroy();
			return 0;
		}
	}

	ConsoleManager::GetInstance()->UnregisterScreen(prebootScreen->GetName());
	std::this_thread::sleep_for(std::chrono::seconds(2));
	ConsoleManager::GetInstance()->Initialize();

	bool running = true;
	while (running) {
		ConsoleManager::GetInstance()->Update();
		ConsoleManager::GetInstance()->Render();
		running = ConsoleManager::GetInstance()->GetIsRunnning();
	}

	CPUTicker::Stop();
	Scheduler::Destroy();
	FinalizeUnfinishedProcesses();
	ConsoleManager::Destroy();
	CPUManager::Destroy();
	MemoryManager::Destroy();
	ProcessManager::Destroy();
	SystemState::Shutdown();

	return 0;
}
