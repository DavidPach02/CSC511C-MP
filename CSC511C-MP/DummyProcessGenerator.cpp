#include "DummyProcessGenerator.h"
#include "AppConfig.h"
#include "Scheduler.h"
#include "ProcessManager.h"
#include "ConsoleManager.h"
#include "BaseScreen.h"
#include "Process.h"
#include "PrintInstruction.h"
#include "DeclareVariableInstruction.h"
#include "AddInstruction.h"
#include "SubtractInstruction.h"
#include "ForInstruction.h"
#include "SleepInstruction.h"
#include <memory>
#include <random>
#include <string>

int DummyProcessGenerator::nextProcessId = 1;
std::vector<std::string> DummyProcessGenerator::createdScreenNames;

std::string DummyProcessGenerator::MakeProcessName(int processId) {
	if (processId < 10) {
		return "p0" + std::to_string(processId);
	}
	return "p" + std::to_string(processId);
}

void DummyProcessGenerator::Reset() {
	ConsoleManager* consoleManager = ConsoleManager::GetInstance();
	for (const std::string& screenName : createdScreenNames) {
		consoleManager->UnregisterScreen(screenName);
	}

	createdScreenNames.clear();
	nextProcessId = 1;
}

bool DummyProcessGenerator::GenerateOne(const AppConfig& appConfig) {
	if (Scheduler::GetInstance() == nullptr) {
		return false;
	}

	std::random_device randomDevice;
	std::mt19937 generator(randomDevice());
	std::uniform_int_distribution<int> commandCountDistribution(
		appConfig.GetMinInstructions(),
		appConfig.GetMaxInstructions());

	const int commandCount = commandCountDistribution(generator);
	const std::string processName = MakeProcessName(nextProcessId);

	auto process = std::make_shared<Process>(nextProcessId, processName);
	++nextProcessId;

	// TODO: Randomize instructions
	for (int commandIndex = 0; commandIndex < commandCount; ++commandIndex) {
		process->AddInstruction(std::make_unique<DeclareVariableInstruction>(process, "Sample" + std::to_string(commandIndex), commandIndex));
		process->AddInstruction(std::make_unique<AddInstruction>(process, "x", "1", "2"));
		process->AddInstruction(std::make_unique<SleepInstruction>(process, 2));
		process->AddInstruction(std::make_unique<AddInstruction>(process, "y", "Sample" + std::to_string(commandIndex), "1"));
		process->AddInstruction(std::make_unique<SubtractInstruction>(process, "z", "x", "y"));
		process->AddInstruction(std::make_unique<PrintInstruction>(process, "Hello World from " + process->GetName() + "!"));
		
		std::vector<std::unique_ptr<Instruction>> forLoopInstructions;
		std::unique_ptr<PrintInstruction> forPrint1 = std::make_unique<PrintInstruction>(process, "For Loop Statement A");
		forLoopInstructions.push_back(std::move(forPrint1));
		std::unique_ptr<PrintInstruction> forPrint2 = std::make_unique<PrintInstruction>(process, "For Loop Statement B");
		forLoopInstructions.push_back(std::move(forPrint2));
		
		process->AddInstruction(std::make_unique<ForInstruction>(process, std::move(forLoopInstructions), "z"));
	}

	std::shared_ptr<BaseScreen> baseScreen = std::make_shared<BaseScreen>(process);
	// Register the screen for the process
	ConsoleManager::GetInstance()->RegisterScreen(baseScreen, false);
	// Add the screen name to the list of created screen names
	createdScreenNames.push_back(processName);
	// Add the process to the scheduler
	Scheduler::GetInstance()->AddProcess(process);
	// Add the process to the process manager
	ProcessManager::GetInstance()->AddProcess(process);

	return true;
}
