#pragma once

#include "ICommand.h"
#include <vector>
#include <memory>
#include <string>

enum class ProcessStatus {
	Ready,
	Running,
	Paused,
	Terminated
};

class Process
{
public:
	Process();
	Process(const int processID, const std::string& processName, const int coreID = 0)
		: id(processID), name(processName), coreID(coreID), status(ProcessStatus::Ready),
		  startTime(""), startDate(""), endTime(""), endDate(""), 
		  commandCount(0), executedCommandCount(0) {}
	~Process() = default;

	void Initialize(const int processID, const std::string& processName, const int coreID = 0);
	void Run();
	void Pause();
	void Resume();
	void Terminate();

	void AddCommand(std::unique_ptr<ICommand> command);
	void ExecuteCommands();
	static void SetArtificialCommandDelayMs(int delayMs);
	static int GetArtificialCommandDelayMs();

	void PrintInfo() const;

	int GetID() const;
	int GetCoreID() const;
	std::string GetName() const;
	std::string GetStatus() const;
	ProcessStatus GetStatusEnum() const;
	std::string GetStartTime() const;
	std::string GetStartDate() const;
	std::string GetEndTime() const;
	std::string GetEndDate() const;
	int GetCommandCount() const;
	int GetExecutedCommandCount() const;

private:
	int id;
	std::string name;
	int coreID;
	ProcessStatus status;
	std::string startTime;
	std::string startDate;
	std::string endTime;
	std::string endDate;
	int commandCount;
	int executedCommandCount;
	std::vector<std::unique_ptr<ICommand>> commands;
	static int artificialCommandDelayMs;
};

