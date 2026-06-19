#include <iostream>
#include <string>
#include <cstdlib>
#include <memory>
#include "AppConfig.h"
#include "ConsoleManager.h"
#include "CPUManager.h"
#include "Scheduler.h"
#include "Process.h"
#include "PrintCommand.h"

void InitializeDummyProcesses(int totalCores) {
	// Initialize the Scheduler with FCFS algorithm
	Scheduler::Initialize(totalCores, SchedulingAlgorithm::FCFS);

	// Create 10 processes, each with 100 print commands
	for (int processNum = 1; processNum <= 10; ++processNum) {
		std::string processName = "screen_";
		if (processNum < 10) processName += "0";
		processName += std::to_string(processNum);

		int coreID = (processNum - 1) % totalCores;

		auto process = std::make_shared<Process>(processNum, processName, coreID);

		// Add 100 print commands to each process
		for (int cmdNum = 1; cmdNum <= 100; ++cmdNum) {
			process->AddCommand(std::make_unique<PrintCommand>(*process));
		}

		// Add process to scheduler
		Scheduler::GetInstance()->AddProcess(process);
		
		// Track process in CPUManager for display
		CPUManager::GetInstance()->AddProcess(process);
	}

	// Start the scheduler
	Scheduler::GetInstance()->Start();
}

int main(int argc, char* argv[]){
	AppConfig appConfig = AppConfig::FromConfigFile("../data/config.txt");
	Process::SetArtificialCommandDelayMs(appConfig.GetArtificialCommandDelayMs());

	ConsoleManager::Initialize();
	CPUManager::Initialize(appConfig.GetTotalCores());

	// Initialize dummy processes for testing
	InitializeDummyProcesses(appConfig.GetTotalCores());

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