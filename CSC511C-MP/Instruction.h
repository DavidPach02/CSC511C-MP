#pragma once

#include "Process.h"
#include "TimeUtility.h"
#include <string>
#include <memory>

class Process;

class Instruction {
public:
	Instruction(std::shared_ptr<Process> process);
	virtual ~Instruction() = default;

	virtual void Execute() = 0;
	std::string GetExecutionDate() const;
	std::string GetExecutionTime() const;
	int GetExecutionCore() const;

protected:
	std::shared_ptr<Process> process;
	std::string executionDate;
	std::string executionTime;
	int executionCore;

	void LogExecution();
};

