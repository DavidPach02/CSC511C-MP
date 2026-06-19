#include "ProcessList.h"
#include <algorithm>

ProcessList::ProcessList() {
}

void ProcessList::AddProcess(std::shared_ptr<Process> process) {
	if (process != nullptr) {
		processes.push_back(process);
	}
}

bool ProcessList::RemoveProcessByID(int processID) {
	auto it = std::find_if(processes.begin(), processes.end(),
		[processID](const std::shared_ptr<Process>& p) {
			return p->GetID() == processID;
		});
	
	if (it != processes.end()) {
		processes.erase(it);
		return true;
	}
	return false;
}

bool ProcessList::RemoveProcessByName(const std::string& processName) {
	auto it = std::find_if(processes.begin(), processes.end(),
		[&processName](const std::shared_ptr<Process>& p) {
			return p->GetName() == processName;
		});
	
	if (it != processes.end()) {
		processes.erase(it);
		return true;
	}
	return false;
}

void ProcessList::Clear() {
	processes.clear();
}

const std::vector<std::shared_ptr<Process>>& ProcessList::GetAllProcesses() const {
	return processes;
}

std::shared_ptr<Process> ProcessList::GetProcessByID(int processID) const {
	auto it = std::find_if(processes.begin(), processes.end(),
		[processID](const std::shared_ptr<Process>& p) {
			return p->GetID() == processID;
		});
	
	return (it != processes.end()) ? *it : nullptr;
}

std::shared_ptr<Process> ProcessList::GetProcessByName(const std::string& processName) const {
	auto it = std::find_if(processes.begin(), processes.end(),
		[&processName](const std::shared_ptr<Process>& p) {
			return p->GetName() == processName;
		});
	
	return (it != processes.end()) ? *it : nullptr;
}

std::vector<std::shared_ptr<Process>> ProcessList::GetProcessesByStatus(ProcessStatus status) const {
	std::vector<std::shared_ptr<Process>> result;
	for (const auto& process : processes) {
		if (process->GetStatusEnum() == status) {
			result.push_back(process);
		}
	}
	return result;
}

std::vector<std::shared_ptr<Process>> ProcessList::GetProcessesByCoreID(int coreID) const {
	std::vector<std::shared_ptr<Process>> result;
	for (const auto& process : processes) {
		if (process->GetCoreID() == coreID) {
			result.push_back(process);
		}
	}
	return result;
}

int ProcessList::GetProcessCount() const {
	return static_cast<int>(processes.size());
}

int ProcessList::GetRunningProcessCount() const {
	int count = 0;
	for (const auto& process : processes) {
		if (process->GetStatusEnum() == ProcessStatus::Running) {
			count++;
		}
	}
	return count;
}

int ProcessList::GetTerminatedProcessCount() const {
	int count = 0;
	for (const auto& process : processes) {
		if (process->GetStatusEnum() == ProcessStatus::Terminated) {
			count++;
		}
	}
	return count;
}
