#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>
#include "ProcessList.h"

class Process;

class CPUManager {
public:
	static CPUManager* GetInstance();
	static void Initialize(int totalCores);
	static void Destroy();

	float GetCPUUtilization() const { return this->cpuUtilization; }
	float GetTotalCores() const { return this->totalCores; }
	float GetAvailableCores() const { return this->availableCores; }

	void AddProcess(std::shared_ptr<Process> process);
	const std::vector<std::shared_ptr<Process>>& GetAllProcesses() const;

	// Query methods delegated to ProcessList
	std::shared_ptr<Process> GetProcessByID(int processID) const;
	std::shared_ptr<Process> GetProcessByName(const std::string& processName) const;
	std::vector<std::shared_ptr<Process>> GetProcessesByStatus(ProcessStatus status) const;
	std::vector<std::shared_ptr<Process>> GetProcessesByCoreID(int coreID) const;
	int GetProcessCount() const;
	int GetRunningProcessCount() const;
	int GetTerminatedProcessCount() const;

	std::stringstream GetSnapshotLog() const;
	void DisplaySnapshot() const;
private:
	CPUManager();
	~CPUManager() = default;
	CPUManager(const CPUManager&) = delete;
	CPUManager& operator=(const CPUManager&) = delete;
	static CPUManager* instance;

	float cpuUtilization;
	int totalCores;
	int availableCores;
	std::unique_ptr<ProcessList> processList;
};

