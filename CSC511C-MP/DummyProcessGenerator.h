#pragma once

#include <string>
#include <vector>

class AppConfig;

// Creates dummy processes (p01, p02, …) when called by CPUTicker during scheduler-start.
// Builds Process + print commands, registers screen, adds to Scheduler and ProcessManager.
class DummyProcessGenerator {
public:
	static bool GenerateOne(const AppConfig& appConfig, const std::string& customName = "", const size_t memoryRequired = 0);
	static bool GenerateOneWithInstruction(const AppConfig& appConfig, const std::string& customName, const size_t memoryRequired, const std::string& instructionsArgument);
	static void Reset();

private:
	static std::string MakeProcessName(int processId);

	static int nextProcessId;
	static std::vector<std::string> createdScreenNames;
};
