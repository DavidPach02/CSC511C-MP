#pragma once

#include "Instruction.h"
#include "SymbolTable.h"
#include <cstdint>
#include <vector>
#include <memory>
#include <string>

class Instruction;

enum class ProcessStatus {
	Ready,
	Running,
	Paused,
	Sleeping,
	Terminated
};

// A runnable program: instruction list, status, and timing. Executes via EITThread on a core.
class Process
{
public:
	Process();
	Process(const int processID, const std::string& processName, const size_t memoryRequired, const int coreID = 0);
	~Process();

	void Initialize();
	void Run();
	void Pause();
	void Resume();
	void SetReady();
	void Terminate();

	void AddInstruction(std::unique_ptr<Instruction> command);
	void ExecuteCommands();
	bool ExecuteNextCommand();
	bool HasRemainingCommands() const;
	static void SetDelaysPerExec(int delayCycles);
	static int GetDelaysPerExec();

	void PrintInfo() const;

	int GetID() const;
	int GetCoreID() const;
	void SetCoreID(int coreId);
	std::string GetName() const;
	std::string GetStatus() const;
	ProcessStatus GetStatusEnum() const;
	std::string GetStartTime() const;
	std::string GetStartDate() const;
	std::string GetEndTime() const;
	std::string GetEndDate() const;
	std::string GetLogs() const;
	SymbolTable* GetSymbolTable() const;

	int GetCommandCount() const;
	int GetExecutedCommandCount() const;

	size_t GetMemoryRequired() const;
	void* GetMemoryAddress() const;
	void SetMemoryAddress(void* address);
	bool HasMemoryLoaded() const;

	void SleepForTicks(std::uint8_t duration);
	bool IsSleeping() const;
	bool IsSleepComplete() const;
	void WakeIfReady();
	uint64_t GetWakeTick() const;
	
	void LogMessage(std::string& message);

private:
	int id;
	std::string name;
	size_t memoryRequired;
	void* memoryAddress;
	int coreID;
	ProcessStatus status;
	SymbolTable* symTable;
	std::string startTime;
	std::string startDate;
	std::string endTime;
	std::string endDate;
	int commandCount;
	int executedCommandCount;
	uint64_t wakeTick;

	std::vector<std::unique_ptr<Instruction>> commands;
	static int delaysPerExec;

	std::string logs;
};

