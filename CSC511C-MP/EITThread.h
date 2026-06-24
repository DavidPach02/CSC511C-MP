#pragma once

#include "Process.h"
#include <memory>

enum class EITThreadState {
	Ready,
	Running,
	Finished
};

// Runs one Process on one CPU core. Used by scheduler workers.
// Assigns/releases cores, starts the process, runs instructions (FCFS: all, RR: slice).
class EITThread {
public:
	EITThread(std::shared_ptr<Process> process, int coreId);
	~EITThread();

	int GetThreadId() const;
	int GetCoreId() const;
	std::shared_ptr<Process> GetProcess() const;
	EITThreadState GetState() const;
	bool IsFinished() const;

	void RunToCompletion();
	bool ExecuteTimeSlice(int commandCount);

private:
	void EnsureStarted();
	void AssignToCore();
	void ReleaseCoreAssignment();

	static int GenerateThreadId();

	int threadId;
	int coreId;
	std::shared_ptr<Process> process;
	EITThreadState state;
	bool coreAssigned;
};
