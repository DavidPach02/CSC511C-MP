#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <mutex>
#include <vector>

// Tracks CPU cores: busy/available state and utilization for screen -ls and report-util.
class CPUManager {
public:
	static CPUManager* GetInstance();
	static void Initialize(int totalCores);
	static void Destroy();

	float GetCPUUtilization() const;
	int GetTotalCores() const;
	int GetAvailableCores() const;

	void AcquireCore(int coreId);
	void ReleaseCore(int coreId);

	std::stringstream GetSnapshotLog() const;
	void DisplaySnapshot() const;

private:
	CPUManager();
	~CPUManager() = default;
	CPUManager(const CPUManager&) = delete;
	CPUManager& operator=(const CPUManager&) = delete;

	void UpdateUtilization();

	static CPUManager* instance;

	mutable std::mutex coreMutex;
	std::vector<bool> coreBusy;
	float cpuUtilization;
	int totalCores;
	int availableCores;
};
