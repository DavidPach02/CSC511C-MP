#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

class CPUManager {
public:
	static CPUManager* GetInstance();
	static void Initialize(int totalCores);
	static void Destroy();

	float GetCPUUtilization() const { return this->cpuUtilization; }
	float GetTotalCores() const { return this->totalCores; }
	float GetAvailableCores() const { return this->availableCores; }

	// TODO: Add processes here
	// TODO: Add logic for dispatching them to logical processors

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
};

