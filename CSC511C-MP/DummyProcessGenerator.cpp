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
#include "ReadInstruction.h"
#include "WriteInstruction.h"
#include "InstructionBuilder.h"
#include "MemoryManager.h"
#include <memory>
#include <random>
#include <sstream>
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
	std::uniform_int_distribution<int> instructionTypeDistribution(0, 7);
	std::uniform_int_distribution<int> valueDistribution(0, 20);

	const int commandCount = commandCountDistribution(generator);
	const std::string processName = customName == "" ? MakeProcessName(nextProcessId) : customName;

	const size_t memorySize = memoryRequired > 0
		? memoryRequired
		: appConfig.RollSchedulerProcessMemory();
	auto process = std::make_shared<Process>(nextProcessId, processName, memorySize);
	++nextProcessId;
	MemoryManager::GetInstance()->RegisterProcess(process->GetID(), memorySize);
	process->InitializeSymbolTable(process);

	// Variables are used to store the values of the variables declared in the process
	std::vector<std::string> variables;

	auto randomMemoryAddress = [&]() -> std::string {
		if (memorySize < 2) {
			return "0x0";
		}

		const size_t maxAddress = memorySize - 2;
		std::uniform_int_distribution<size_t> addressDistribution(0, maxAddress);
		const size_t address = addressDistribution(generator);

		std::ostringstream addressBuilder;
		addressBuilder << "0x" << std::hex << std::uppercase << address;
		return addressBuilder.str();
	};

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
		case 6:
			process->AddInstruction(std::make_unique<WriteInstruction>(
				process, randomMemoryAddress(), randomOperand()));
			break;
		case 7:
			process->AddInstruction(std::make_unique<ReadInstruction>(
				process, variableName, randomMemoryAddress()));
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

bool DummyProcessGenerator::GenerateOneWithInstruction(
	const AppConfig& appConfig, const std::string& customName, 
	const size_t memoryRequired, const std::string& instructionsArgument) {

	const std::string processName = customName == "" ? MakeProcessName(nextProcessId) : customName;

	const size_t memorySize = memoryRequired > 0
		? memoryRequired
		: static_cast<size_t>(appConfig.GetMinMemoryPerProcess());
	auto process = std::make_shared<Process>(nextProcessId, processName, memorySize);

	// TEST-FULL: screen -c p01 64 "DECLARE varA 10; DECLARE varB 5; ADD varA varA varB; WRITE 0x500 varA; READ varC 0x500; SLEEP 4; FOR(5, PRINT('Hello, World'), ADD varA varA varB); PRINT('Result: %i', varA); PRINT('Result: %i', varC);"
	// TEST-A: screen -c p01 64 "DECLARE varA 10; DECLARE varB 5; ADD varA varA varB; FOR(5, PRINT('varA: %i', varA), ADD varA varA varB);"
	if (!InstructionBuilder::BuildInstructionsFromString(instructionsArgument, process)){
		return false;
	}

	// If successfully created increment the next process ID
	++nextProcessId;

	MemoryManager::GetInstance()->RegisterProcess(process->GetID(), memorySize); // Register the process with the memory manager
	process->InitializeSymbolTable(process); // Initialize the symbol table for the process
	std::shared_ptr<BaseScreen> baseScreen = std::make_shared<BaseScreen>(process);
	ConsoleManager::GetInstance()->RegisterScreen(baseScreen, false);
	createdScreenNames.push_back(processName);
	// Add the process to the scheduler
	Scheduler::GetInstance()->AddProcess(process);
	ProcessManager::GetInstance()->AddProcess(process);

	return true;
}


