#include "Process.h"

#include <iostream>
#include <chrono>
#include <thread>
#include "TimeUtility.h"

int Process::artificialCommandDelayMs = 50;

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
	commandCount(0), executedCommandCount(0)
{
}

void Process::Initialize(const int processID, const std::string& processName, const int coreID)
{
	id = processID;
	name = processName;
	this->coreID = coreID;
	status = ProcessStatus::Ready;
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

void Process::AddCommand(std::unique_ptr<ICommand> command)
{
	commands.push_back(std::move(command));
	commandCount++;
}

void Process::ExecuteCommands()
{
	for (const auto& command : commands) {
		command->Execute({});
		std::this_thread::sleep_for(std::chrono::milliseconds(artificialCommandDelayMs)); // Simulate time taken to execute each command.
		executedCommandCount++;
	}
}

void Process::SetArtificialCommandDelayMs(int delayMs)
{
	if (delayMs >= 0) {
		artificialCommandDelayMs = delayMs;
	}
}

int Process::GetArtificialCommandDelayMs()
{
	return artificialCommandDelayMs;
}

int Process::GetID() const
{
	return id;
}

int Process::GetCoreID() const
{
	return coreID;
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

int Process::GetCommandCount() const
{
	return commandCount;
}

int Process::GetExecutedCommandCount() const
{
	return executedCommandCount;
}
