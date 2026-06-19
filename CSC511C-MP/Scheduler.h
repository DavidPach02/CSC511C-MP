#pragma once

#include "Process.h"
#include <memory>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

enum class SchedulingAlgorithm {
	FCFS	// First Come, First Served
};

class Scheduler {
public:
	static Scheduler* GetInstance();
	static void Initialize(int totalCores, SchedulingAlgorithm algorithm);
	static void Destroy();

	void AddProcess(std::shared_ptr<Process> process);
	void Start();
	void Stop();

	SchedulingAlgorithm GetAlgorithm() const;
	std::string GetAlgorithmName() const;
	bool IsRunning() const;

private:
	Scheduler();
	~Scheduler() = default;
	Scheduler(const Scheduler&) = delete;
	Scheduler& operator=(const Scheduler&) = delete;
	static Scheduler* instance;

	void RunCore(int coreID);
	void RunFCFS(int coreID);

	SchedulingAlgorithm algorithm;
	int totalCores;
	bool running;

	std::queue<std::shared_ptr<Process>> readyQueue;
	std::vector<std::thread> coreThreads;
	std::mutex queueMutex;
	std::condition_variable queueCondition;
};
