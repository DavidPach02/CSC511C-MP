#include "CPUManager.h"

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
}
