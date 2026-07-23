#pragma once

#include "ProcessList.h"
#include <memory>
#include <string>
#include <vector>

// Registry of all processes. Used by screen -ls, screen -r, and report-util lookups.
class ProcessManager {
public:
	static ProcessManager* GetInstance();
	static void Initialize();
	static void Destroy();

	void AddProcess(std::shared_ptr<Process> process);
	bool RemoveProcessByID(int processID);
	bool RemoveProcessByName(const std::string& processName);
	void Clear();

	const std::vector<std::shared_ptr<Process>>& GetAllProcesses() const;
	std::shared_ptr<Process> GetProcessByID(int processID) const;
	std::shared_ptr<Process> GetProcessByName(const std::string& processName) const;
	std::vector<std::shared_ptr<Process>> GetProcessesByStatus(ProcessStatus status) const;
	std::vector<std::shared_ptr<Process>> GetProcessesByCoreID(int coreID) const;

	int GetProcessCount() const;
	int GetProcessesCountByStatus(ProcessStatus status) const;

private:
	ProcessManager();
	~ProcessManager() = default;
	ProcessManager(const ProcessManager&) = delete;
	ProcessManager& operator=(const ProcessManager&) = delete;

	static ProcessManager* instance;

	std::unique_ptr<ProcessList> processList;
};
