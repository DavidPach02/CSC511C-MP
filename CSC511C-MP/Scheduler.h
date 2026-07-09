#pragma once

#include "Process.h"
#include <memory>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

enum class SchedulingAlgorithm {
	FCFS,
	RR
};

// CPU scheduler base: ready queue + one std::thread worker per core.
// Workers dequeue processes and hand them to EITThread; subclasses define FCFS vs RR policy.
class Scheduler {
public:
	static Scheduler* GetInstance();
	static void Initialize(int totalCores, SchedulingAlgorithm algorithm, int quantumCommands = 1);
	static void Destroy();

	virtual ~Scheduler();

	void AddProcess(std::shared_ptr<Process> process);
	void Start();
	void Stop();

	virtual SchedulingAlgorithm GetAlgorithm() const = 0;
	virtual std::string GetAlgorithmName() const = 0;
	bool IsRunning() const;

protected:
	explicit Scheduler(int totalCores);

	virtual void RunCore(int coreID) = 0;

	bool DequeueProcess(std::shared_ptr<Process>& process);
	void RequeueProcess(std::shared_ptr<Process> process);
	bool PrepareProcessForExecution(const std::shared_ptr<Process>& process);
	void FinalizeProcess(const std::shared_ptr<Process>& process);

	int totalCores;
	bool running;

	std::queue<std::shared_ptr<Process>> readyQueue;
	std::vector<std::thread> coreThreads;
	std::mutex queueMutex;
	std::condition_variable queueCondition;

private:
	Scheduler(const Scheduler&) = delete;
	Scheduler& operator=(const Scheduler&) = delete;

	void RunWorker(int coreID);

	static Scheduler* instance;
};
