#include "CPUManager.h"
#include <iomanip>
#include <iostream>
#include <mutex>

CPUManager* CPUManager::instance = nullptr;

CPUManager* CPUManager::GetInstance() {
	if (instance == nullptr) {
		instance = new CPUManager();
	}
	return instance;
}

void CPUManager::Initialize(int totalCores) {
	GetInstance()->totalCores = totalCores;
	GetInstance()->availableCores = totalCores;
	GetInstance()->cpuUtilization = 0.0f;
	GetInstance()->coreBusy.assign(totalCores, false);
}

void CPUManager::Destroy() {
	delete instance;
	instance = nullptr;
}

CPUManager::CPUManager()
	: cpuUtilization(0.0f), totalCores(0), availableCores(0) {
}

float CPUManager::GetCPUUtilization() const {
	std::lock_guard<std::mutex> lock(coreMutex);
	return cpuUtilization;
}

int CPUManager::GetTotalCores() const {
	return totalCores;
}

int CPUManager::GetAvailableCores() const {
	std::lock_guard<std::mutex> lock(coreMutex);
	return availableCores;
}

void CPUManager::AcquireCore(int coreId) {
	if (coreId < 0 || coreId >= totalCores) {
		return;
	}

	std::lock_guard<std::mutex> lock(coreMutex);
	if (!coreBusy[coreId]) {
		coreBusy[coreId] = true;
		--availableCores;
		UpdateUtilization();
	}
}

void CPUManager::ReleaseCore(int coreId) {
	if (coreId < 0 || coreId >= totalCores) {
		return;
	}

	std::lock_guard<std::mutex> lock(coreMutex);
	if (coreBusy[coreId]) {
		coreBusy[coreId] = false;
		++availableCores;
		UpdateUtilization();
	}
}

void CPUManager::UpdateUtilization() {
	if (totalCores <= 0) {
		cpuUtilization = 0.0f;
		return;
	}

	const int busyCores = totalCores - availableCores;
	cpuUtilization = (static_cast<float>(busyCores) / static_cast<float>(totalCores)) * 100.0f;
}

std::stringstream CPUManager::GetSnapshotLog() const {
	std::stringstream textLog;
	std::lock_guard<std::mutex> lock(coreMutex);

	textLog << "CPU Utilization: " << std::fixed << std::setprecision(2) << cpuUtilization << "%\n"
		<< "Total Cores: " << totalCores << "\n"
		<< "Available Cores: " << availableCores << "\n";

	return textLog;
}

void CPUManager::DisplaySnapshot() const {
	std::cout << GetSnapshotLog().str();
}
