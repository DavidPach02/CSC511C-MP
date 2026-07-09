#include "Scheduler.h"
#include "FCFSScheduler.h"
#include "RRScheduler.h"
#include "MemoryManager.h"

Scheduler* Scheduler::instance = nullptr;

Scheduler* Scheduler::GetInstance() {
	return instance;
}

void Scheduler::Initialize(int totalCores, SchedulingAlgorithm algorithm, int quantumCommands) {
	Destroy();

	switch (algorithm) {
	case SchedulingAlgorithm::RR:
		instance = new RRScheduler(totalCores, quantumCommands);
		break;
	case SchedulingAlgorithm::FCFS:
	default:
		instance = new FCFSScheduler(totalCores);
		break;
	}
}

void Scheduler::Destroy() {
	if (instance != nullptr) {
		instance->Stop();
		delete instance;
		instance = nullptr;
	}
}

Scheduler::Scheduler(int totalCores)
	: totalCores(totalCores), running(false) {
}

Scheduler::~Scheduler() = default;

void Scheduler::AddProcess(std::shared_ptr<Process> process) {
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		readyQueue.push(process);
	}
	queueCondition.notify_one();
}

void Scheduler::Start() {
	if (running) {
		return;
	}

	running = true;

	for (int coreID = 0; coreID < totalCores; ++coreID) {
		coreThreads.emplace_back(&Scheduler::RunWorker, this, coreID);
	}
}

void Scheduler::Stop() {
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		running = false;
		while (!readyQueue.empty()) {
			readyQueue.pop();
		}
	}
	queueCondition.notify_all();

	for (std::thread& coreThread : coreThreads) {
		if (coreThread.joinable()) {
			coreThread.join();
		}
	}

	coreThreads.clear();
}

bool Scheduler::IsRunning() const {
	return running;
}

void Scheduler::RunWorker(int coreID) {
	RunCore(coreID);
}

bool Scheduler::DequeueProcess(std::shared_ptr<Process>& process) {
	std::unique_lock<std::mutex> lock(queueMutex);
	queueCondition.wait(lock, [this] {
		return !readyQueue.empty() || !running;
	});

	if (!running && readyQueue.empty()) {
		return false;
	}

	process = readyQueue.front();
	readyQueue.pop();
	return true;
}

void Scheduler::RequeueProcess(std::shared_ptr<Process> process) {
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		if (!running) {
			return;
		}
		readyQueue.push(process);
	}
	queueCondition.notify_one();
}

bool Scheduler::PrepareProcessForExecution(const std::shared_ptr<Process>& process) {
	return MemoryManager::GetInstance()->TryAllocateForProcess(process);
}

void Scheduler::FinalizeProcess(const std::shared_ptr<Process>& process) {
	MemoryManager::GetInstance()->ReleaseProcessMemory(process);
}
