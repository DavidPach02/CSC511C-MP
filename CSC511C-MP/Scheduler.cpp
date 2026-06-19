#include "Scheduler.h"

Scheduler* Scheduler::instance = nullptr;

Scheduler* Scheduler::GetInstance() {
	if (instance == nullptr) {
		instance = new Scheduler();
	}
	return instance;
}

void Scheduler::Initialize(int totalCores, SchedulingAlgorithm algorithm) {
	GetInstance()->totalCores = totalCores;
	GetInstance()->algorithm = algorithm;
	GetInstance()->running = false;
}

void Scheduler::Destroy() {
	if (instance != nullptr) {
		instance->Stop();
	}
	delete instance;
	instance = nullptr;
}

Scheduler::Scheduler() : algorithm(SchedulingAlgorithm::FCFS), totalCores(0), running(false) {
}

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
		coreThreads.emplace_back(&Scheduler::RunCore, this, coreID);
	}
}

void Scheduler::Stop() {
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		running = false;
	}
	queueCondition.notify_all();

	for (std::thread& coreThread : coreThreads) {
		if (coreThread.joinable()) {
			coreThread.join();
		}
	}

	coreThreads.clear();
}

void Scheduler::RunCore(int coreID) {
	RunFCFS(coreID);
}

void Scheduler::RunFCFS(int coreID) {
	while (true) {
		std::shared_ptr<Process> process;

		{
			std::unique_lock<std::mutex> lock(queueMutex);
			queueCondition.wait(lock, [this] {
				return !readyQueue.empty() || !running;
			});

			if (!running && readyQueue.empty()) {
				return;
			}

			process = readyQueue.front();
			readyQueue.pop();
            
		}

		process->Run();
		// Execute all commands attached to the process in order
		process->ExecuteCommands();

		process->Terminate();
	}
}

SchedulingAlgorithm Scheduler::GetAlgorithm() const {
	return algorithm;
}

std::string Scheduler::GetAlgorithmName() const {
	return "First Come, First Served (FCFS)";
}

bool Scheduler::IsRunning() const {
	return running;
}
