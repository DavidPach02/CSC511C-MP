#include "ProcessManager.h"

ProcessManager* ProcessManager::instance = nullptr;

ProcessManager* ProcessManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ProcessManager();
	}
	return instance;
}

void ProcessManager::Initialize() {
	if (instance == nullptr) {
		instance = new ProcessManager();
	}
}

void ProcessManager::Destroy() {
	delete instance;
	instance = nullptr;
}

ProcessManager::ProcessManager()
	: processList(std::make_unique<ProcessList>()) {
}

void ProcessManager::AddProcess(std::shared_ptr<Process> process) {
	processList->AddProcess(process);
}

bool ProcessManager::RemoveProcessByID(int processID) {
	return processList->RemoveProcessByID(processID);
}

bool ProcessManager::RemoveProcessByName(const std::string& processName) {
	return processList->RemoveProcessByName(processName);
}

void ProcessManager::Clear() {
	processList->Clear();
}

const std::vector<std::shared_ptr<Process>>& ProcessManager::GetAllProcesses() const {
	return processList->GetAllProcesses();
}

std::shared_ptr<Process> ProcessManager::GetProcessByID(int processID) const {
	return processList->GetProcessByID(processID);
}

std::shared_ptr<Process> ProcessManager::GetProcessByName(const std::string& processName) const {
	return processList->GetProcessByName(processName);
}

std::vector<std::shared_ptr<Process>> ProcessManager::GetProcessesByStatus(ProcessStatus status) const {
	return processList->GetProcessesByStatus(status);
}

std::vector<std::shared_ptr<Process>> ProcessManager::GetProcessesByCoreID(int coreID) const {
	return processList->GetProcessesByCoreID(coreID);
}

int ProcessManager::GetProcessCount() const {
	return processList->GetProcessCount();
}

int ProcessManager::GetRunningProcessCount() const {
	return processList->GetRunningProcessCount();
}

int ProcessManager::GetTerminatedProcessCount() const {
	return processList->GetTerminatedProcessCount();
}
