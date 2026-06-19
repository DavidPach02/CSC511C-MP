#pragma once

#include "Process.h"
#include <vector>
#include <memory>
#include <string>

class ProcessList {
public:
	ProcessList();
	~ProcessList() = default;

	// Add and remove processes
	void AddProcess(std::shared_ptr<Process> process);
	bool RemoveProcessByID(int processID);
	bool RemoveProcessByName(const std::string& processName);
	void Clear();

	// Query processes
	const std::vector<std::shared_ptr<Process>>& GetAllProcesses() const;
	std::shared_ptr<Process> GetProcessByID(int processID) const;
	std::shared_ptr<Process> GetProcessByName(const std::string& processName) const;
	std::vector<std::shared_ptr<Process>> GetProcessesByStatus(ProcessStatus status) const;
	std::vector<std::shared_ptr<Process>> GetProcessesByCoreID(int coreID) const;

	// Size and information
	int GetProcessCount() const;
	int GetRunningProcessCount() const;
	int GetTerminatedProcessCount() const;

private:
	std::vector<std::shared_ptr<Process>> processes;
};
