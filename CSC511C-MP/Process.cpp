#include "Process.h"

#include <iostream>
#include "TimeUtility.h"
#include "CPUTicker.h"

int Process::delaysPerExec = 0;

std::string StatusToString(ProcessStatus status)
{
	switch (status) {
	case ProcessStatus::Ready:
		return "Ready";
	case ProcessStatus::Running:
		return "Running";
	case ProcessStatus::Paused:
		return "Paused";
	case ProcessStatus::Terminated:
		return "Terminated";
	default:
		return "Unknown";
	}
}

Process::Process() : id(0), name(""), coreID(0), status(ProcessStatus::Ready),
	startTime(""), startDate(""), endTime(""), endDate(""),
	commandCount(0), executedCommandCount(0), logs(""), symTable(new SymbolTable())
{
}

void Process::Initialize(const int processID, const std::string& processName, const int coreID)
{
	id = processID;
	name = processName;
	this->coreID = coreID;
	status = ProcessStatus::Ready;
	symTable = new SymbolTable();
	startTime = "";
	startDate = "";
	endTime = "";
	endDate = "";
	commandCount = 0;
	executedCommandCount = 0;
	commands.clear();
}

void Process::Run()
{
	if (status == ProcessStatus::Ready || status == ProcessStatus::Paused) {
		status = ProcessStatus::Running;
		// Capture start time and date on first run
		if (startTime.empty()) {
			startTime = TimeUtility::GetCurrentTimeString(false, ":");
			startDate = TimeUtility::GetCurrentDateString("/");
		}
	}
}

void Process::Pause()
{
	if (status == ProcessStatus::Running) {
		status = ProcessStatus::Paused;
	}
}

void Process::Resume()
{
	if (status == ProcessStatus::Paused) {
		status = ProcessStatus::Running;
	}
}

void Process::Terminate()
{
	status = ProcessStatus::Terminated;
	// Capture end time and date on termination
	endTime = TimeUtility::GetCurrentTimeString(false, ":");
	endDate = TimeUtility::GetCurrentDateString("/");
}

void Process::PrintInfo() const
{
	std::cout << "Process: " << name << " (ID: " << id << ", Core: " << coreID << ", Status: " << GetStatus() << ")\n";
}

void Process::AddInstruction(std::unique_ptr<Instruction> command)
{
	commands.push_back(std::move(command));
	commandCount++;
}

void Process::ExecuteCommands()
{
	while (HasRemainingCommands()) {
		ExecuteNextCommand();
	}
}

bool Process::ExecuteNextCommand()
{
	if (!HasRemainingCommands()) {
		return false;
	}

	// 
	if (delaysPerExec > 0 && executedCommandCount > 0) {
		const uint64_t targetTick = CPUTicker::GetInstance()->GetCurrentTick()
			+ static_cast<uint64_t>(delaysPerExec);

		// Wait until the target tick is reached
		CPUTicker::GetInstance()->WaitUntilTick(targetTick);
	}

	commands[executedCommandCount]->Execute();
	executedCommandCount++;

	if (!HasRemainingCommands()) {
		std::string finishText = "Finished!";
		this->LogMessage(finishText);
	}

	return HasRemainingCommands();
}

bool Process::HasRemainingCommands() const
{
	return executedCommandCount < commandCount;
}

void Process::SetDelaysPerExec(int delayCycles)
{
	if (delayCycles >= 0) {
		delaysPerExec = delayCycles;
	}
}

int Process::GetDelaysPerExec()
{
	return delaysPerExec;
}

int Process::GetID() const
{
	return id;
}

int Process::GetCoreID() const
{
	return coreID;
}

void Process::SetCoreID(int coreId)
{
	coreID = coreId;
}

std::string Process::GetName() const
{
	return name;
}

std::string Process::GetStatus() const
{
	return StatusToString(status);
}

ProcessStatus Process::GetStatusEnum() const
{
	return status;
}

std::string Process::GetStartTime() const
{
	return startTime;
}

std::string Process::GetStartDate() const
{
	return startDate;
}

std::string Process::GetEndTime() const
{
	return endTime;
}

std::string Process::GetEndDate() const
{
	return endDate;
}

std::string Process::GetLogs() const
{
	return this->logs;
}

SymbolTable* Process::GetSymbolTable() const
{
	return this->symTable;
}

int Process::GetCommandCount() const
{
	return commandCount;
}

int Process::GetExecutedCommandCount() const
{
	return executedCommandCount;
}

void Process::LogMessage(std::string& message) {
	this->logs.append(message + "\n" + this->symTable->GetTableLogs());
}
