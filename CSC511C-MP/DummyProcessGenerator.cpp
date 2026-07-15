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
#include <vector>

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

bool DummyProcessGenerator::GenerateOne(const AppConfig& appConfig, const std::string& customName, const size_t memoryRequired) {
	if (Scheduler::GetInstance() == nullptr) {
		return false;
	}

	std::mt19937 generator(std::random_device{}());
	std::uniform_int_distribution<int> commandCountDistribution(
		appConfig.GetMinInstructions(), appConfig.GetMaxInstructions());
	std::uniform_int_distribution<int> instructionTypeDistribution(0, 5);
	std::uniform_int_distribution<int> valueDistribution(0, 20);

	const int commandCount = commandCountDistribution(generator);
	const std::string processName = customName == "" ? MakeProcessName(nextProcessId) : customName;

	size_t memorySize = memoryRequired > 0 ? memoryRequired : appConfig.GetMemoryPerProcess();
	auto process = std::make_shared<Process>(nextProcessId, processName, memorySize);
	++nextProcessId;

	// Variables are used to store the values of the variables declared in the process
	std::vector<std::string> variables;

	for (int commandIndex = 0; commandIndex < commandCount; ++commandIndex) {
		const std::string variableName = "variableName" + std::to_string(commandIndex);
		// Add the variable name to the list of variables
		variables.push_back(variableName);

		// Generate a random operand from the list of variables or a random literal
		auto randomOperand = [&]() -> std::string {
			// If there are variables and the random number is even, pick a random variable
			if (!variables.empty() && valueDistribution(generator) % 2 == 0) {
				std::uniform_int_distribution<size_t> variablePick(0, variables.size() - 1);
				return variables[variablePick(generator)];
			}
			// If there are no variables or the random number is odd, pick a random literal
			return std::to_string(valueDistribution(generator));
		};

		switch (instructionTypeDistribution(generator)) {
		case 0:
			process->AddInstruction(std::make_unique<DeclareVariableInstruction>(
				process, variableName, static_cast<uint16_t>(valueDistribution(generator))));
			break;
		case 1:
			process->AddInstruction(std::make_unique<AddInstruction>(
				process, variableName, randomOperand(), randomOperand()));
			break;
		case 2:
			process->AddInstruction(std::make_unique<SubtractInstruction>(
				process, variableName, randomOperand(), randomOperand()));
			break;
		case 3: {
			std::string message = "Hello world from " + process->GetName() + "!";
			std::string varName = "";
			std::uniform_int_distribution<int> randIndexGen(0, commandIndex);
			std::uniform_int_distribution<int> randMessageGen(0, 1);

			int randIndex = randIndexGen(generator);

			if (randMessageGen(generator) == 1) {
				varName = "variableName" + std::to_string(randIndex);
				message = "Retrieved " + varName + " = %i";
			}

			process->AddInstruction(std::make_unique<PrintInstruction>(
				process, message, varName));
			break;
		}
		case 4: {
			std::vector<std::unique_ptr<Instruction>> forBody;
			forBody.push_back(std::make_unique<PrintInstruction>(process, "For loop Statement A"));
			forBody.push_back(std::make_unique<PrintInstruction>(process, "For loop Statement B"));
			process->AddInstruction(std::make_unique<ForInstruction>(
				process, std::move(forBody), randomOperand()));
			break;
		}
		case 5:
			process->AddInstruction(std::make_unique<SleepInstruction>(process, randomOperand()));
			break;
		default:
			break;
		}
	}

	std::shared_ptr<BaseScreen> baseScreen = std::make_shared<BaseScreen>(process);
	ConsoleManager::GetInstance()->RegisterScreen(baseScreen, false);
	createdScreenNames.push_back(processName);
	// Add the process to the scheduler
	Scheduler::GetInstance()->AddProcess(process);
	ProcessManager::GetInstance()->AddProcess(process);

	return true;
}
