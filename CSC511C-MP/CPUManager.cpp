#include "CPUManager.h"
#include "Process.h"

CPUManager* CPUManager::instance = nullptr;
CPUManager* CPUManager::GetInstance() {
	if (instance == nullptr) {
		instance = new CPUManager();
	}
	return instance;
}

void CPUManager::Initialize(int totalCores) {
	GetInstance()->totalCores = totalCores;
	GetInstance()->availableCores = 0;
	GetInstance()->cpuUtilization = 100.0;
}

void CPUManager::Destroy() {
	delete instance;
	instance = nullptr;
}

std::stringstream CPUManager::GetSnapshotLog() const {
	std::stringstream textLog;

	textLog << "CPU Utilization: " << std::fixed << std::setprecision(2) << GetInstance()->cpuUtilization << "% \n"
		<< "Total Cores: " << GetInstance()->totalCores << "\n"
		<< "Available Cores: " << GetInstance()->availableCores << "\n";

	return textLog;
}

void CPUManager::DisplaySnapshot() const {
	std::cout << GetInstance()->GetSnapshotLog().str();
}

// TODO: Remove default values
CPUManager::CPUManager() {
	processList = std::make_unique<ProcessList>();
}

void CPUManager::AddProcess(std::shared_ptr<Process> process) {
	processList->AddProcess(process);
}

const std::vector<std::shared_ptr<Process>>& CPUManager::GetAllProcesses() const {
	return processList->GetAllProcesses();
}

std::shared_ptr<Process> CPUManager::GetProcessByID(int processID) const {
	return processList->GetProcessByID(processID);
}

std::shared_ptr<Process> CPUManager::GetProcessByName(const std::string& processName) const {
	return processList->GetProcessByName(processName);
}

std::vector<std::shared_ptr<Process>> CPUManager::GetProcessesByStatus(ProcessStatus status) const {
	return processList->GetProcessesByStatus(status);
}

std::vector<std::shared_ptr<Process>> CPUManager::GetProcessesByCoreID(int coreID) const {
	return processList->GetProcessesByCoreID(coreID);
}

int CPUManager::GetProcessCount() const {
	return processList->GetProcessCount();
}

int CPUManager::GetRunningProcessCount() const {
	return processList->GetRunningProcessCount();
}

int CPUManager::GetTerminatedProcessCount() const {
	return processList->GetTerminatedProcessCount();
}
