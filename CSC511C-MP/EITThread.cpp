#include "EITThread.h"
#include "CPUManager.h"

int EITThread::GenerateThreadId() {
	static int nextThreadId = 1;
	return nextThreadId++;
}

EITThread::EITThread(std::shared_ptr<Process> process, int coreId)
	: threadId(GenerateThreadId()),
	  coreId(coreId),
	  process(std::move(process)),
	  state(EITThreadState::Ready),
	  coreAssigned(false) {
}

EITThread::~EITThread() {
	ReleaseCoreAssignment();
}

int EITThread::GetThreadId() const {
	return threadId;
}

int EITThread::GetCoreId() const {
	return coreId;
}

std::shared_ptr<Process> EITThread::GetProcess() const {
	return process;
}

EITThreadState EITThread::GetState() const {
	return state;
}

bool EITThread::IsFinished() const {
	return state == EITThreadState::Finished
		|| process->GetStatusEnum() == ProcessStatus::Terminated;
}

void EITThread::AssignToCore() {
	if (coreAssigned) {
		return;
	}

	process->SetCoreID(coreId);
	CPUManager::GetInstance()->AcquireCore(coreId);
	coreAssigned = true;
}

void EITThread::ReleaseCoreAssignment() {
	if (!coreAssigned) {
		return;
	}

	CPUManager::GetInstance()->ReleaseCore(coreId);
	coreAssigned = false;
}

void EITThread::EnsureStarted() {
	if (process->GetStatusEnum() == ProcessStatus::Ready) {
		AssignToCore();
		process->Run();
		state = EITThreadState::Running;
	}
}

void EITThread::RunToCompletion() {
	if (process->GetStatusEnum() == ProcessStatus::Sleeping) {
		ReleaseCoreAssignment();
		state = EITThreadState::Ready;
		return;
	}

	EnsureStarted();

	while (process->HasRemainingCommands()) {
		const bool hasRemaining = process->ExecuteNextCommand();

		if (process->GetStatusEnum() == ProcessStatus::Sleeping) {
			ReleaseCoreAssignment();
			state = EITThreadState::Ready;
			return;
		}

		if (!hasRemaining) {
			break;
		}
	}

	if (!process->HasRemainingCommands()) {
		process->Terminate();
		state = EITThreadState::Finished;
	}

	ReleaseCoreAssignment();
}

bool EITThread::ExecuteTimeSlice(int commandCount) {
	if (commandCount <= 0) {
		return !IsFinished() && process->HasRemainingCommands();
	}

	if (process->GetStatusEnum() == ProcessStatus::Sleeping) {
		ReleaseCoreAssignment();
		state = EITThreadState::Ready;
		return true;
	}

	EnsureStarted();

	for (int commandIndex = 0; commandIndex < commandCount; ++commandIndex) {
		const bool hasRemaining = process->ExecuteNextCommand();

		if (process->GetStatusEnum() == ProcessStatus::Sleeping) {
			ReleaseCoreAssignment();
			state = EITThreadState::Ready;
			return true;
		}

		if (!hasRemaining) {
			process->Terminate();
			state = EITThreadState::Finished;
			ReleaseCoreAssignment();
			return false;
		}
	}

	if (!process->HasRemainingCommands()) {
		process->Terminate();
		state = EITThreadState::Finished;
		ReleaseCoreAssignment();
		return false;
	}

	ReleaseCoreAssignment();
	return true;
}
