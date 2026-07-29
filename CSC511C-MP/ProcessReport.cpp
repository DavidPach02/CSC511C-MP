#include "ProcessReport.h"
#include "AppConfig.h"
#include "CPUManager.h"
#include "MemoryManager.h"
#include "ProcessManager.h"
#include "Process.h"
#include "SystemState.h"
#include <algorithm>
#include <iomanip>
#include <tuple>

namespace {
	std::tuple<int, int, int, int, int, int> BuildEndTimestampKey(const Process& process) {
		int month = 0;
		int day = 0;
		int year = 0;
		int hour = 0;
		int minute = 0;
		int second = 0;

		const std::string& endDate = process.GetEndDate();
		const std::string& endTime = process.GetEndTime();

		if (endDate.size() >= 10) {
			month = std::stoi(endDate.substr(0, 2));
			day = std::stoi(endDate.substr(3, 2));
			year = std::stoi(endDate.substr(6, 4));
		}

		if (endTime.size() >= 10) {
			hour = std::stoi(endTime.substr(0, 2));
			minute = std::stoi(endTime.substr(3, 2));
			second = std::stoi(endTime.substr(6, 2));

			const std::string meridiem = endTime.substr(8);
			if (meridiem == "AM") {
				if (hour == 12) {
					hour = 0;
				}
			} else if (meridiem == "PM" && hour != 12) {
				hour += 12;
			}
		}

		return std::make_tuple(year, month, day, hour, minute, second);
	}

	int CountProcessesInMemory(const std::vector<std::shared_ptr<Process>>& processes) {
		MemoryManager* memoryManager = MemoryManager::GetInstance();
		int loadedCount = 0;
		for (const auto& process : processes) {
			if (process != nullptr && memoryManager->IsProcessRegistered(process->GetID())) {
				++loadedCount;
			}
		}
		return loadedCount;
	}
}

std::stringstream ProcessReport::BuildSummary() {
	std::stringstream summary;
	WriteSummary(summary);
	return summary;
}

void ProcessReport::WriteSummary(std::ostream& output) {
	CPUManager* cpuManager = CPUManager::GetInstance();
	MemoryManager* memoryManager = MemoryManager::GetInstance();
	const auto& processes = ProcessManager::GetInstance()->GetAllProcesses();

	output << "CPU Utilization: " << std::fixed << std::setprecision(2)
		<< cpuManager->GetCPUUtilization() << "%\n";
	output << "Cores used: " << (cpuManager->GetTotalCores() - cpuManager->GetAvailableCores()) << "\n";
	output << "Cores available: " << cpuManager->GetAvailableCores() << "\n";

	const size_t usedMemory = memoryManager->GetUsedMemory();
	const size_t totalMemory = memoryManager->GetTotalMemory();
	const double memoryUtilization = totalMemory > 0
		? (static_cast<double>(usedMemory) / static_cast<double>(totalMemory)) * 100.0
		: 0.0;

	output << "Memory usage: " << usedMemory << " / " << totalMemory << " bytes ("
		<< std::fixed << std::setprecision(2) << memoryUtilization << "%)\n";
	output << "Processes in memory: " << CountProcessesInMemory(processes) << "\n";
	output << "---------------------------------------\n";
	output << "Running processes:\n";
	std::vector<std::shared_ptr<Process>> runningProcesses;
	for (const auto& process : processes) {
		if (process->GetStatusEnum() == ProcessStatus::Running) {
			runningProcesses.push_back(process);
		}
	}

	std::sort(runningProcesses.begin(), runningProcesses.end(),
		[](const std::shared_ptr<Process>& left, const std::shared_ptr<Process>& right) {
			if (left->GetCoreID() != right->GetCoreID()) {
				return left->GetCoreID() < right->GetCoreID();
			}

			return left->GetName() < right->GetName();
		});

	for (const auto& process : runningProcesses) {
		output << process->GetName() << " (" << process->GetStartDate() << " "
			<< process->GetStartTime() << ") Core: " << process->GetCoreID()
			<< " " << process->GetExecutedCommandCount() << " / "
			<< process->GetCommandCount() << "\n";
	}

	output << "\nFinished processes:\n";
	std::vector<std::shared_ptr<Process>> finishedProcesses;
	for (const auto& process : processes) {
		if (process->GetStatusEnum() == ProcessStatus::Terminated) {
			finishedProcesses.push_back(process);
		}
	}

	std::sort(finishedProcesses.begin(), finishedProcesses.end(),
		[](const std::shared_ptr<Process>& left, const std::shared_ptr<Process>& right) {
			return BuildEndTimestampKey(*left) < BuildEndTimestampKey(*right);
		});

	for (const auto& process : finishedProcesses) {
		output << process->GetName() << " (" << process->GetEndDate() << " "
			<< process->GetEndTime() << ") Finished "
			<< process->GetCommandCount() << " / "
			<< process->GetCommandCount() << "\n";
	}

	output << "---------------------------------------\n";
}

void ProcessReport::WriteMemoryMap(std::ostream& output) {
	if (!SystemState::IsInitialized()) {
		return;
	}

	const AppConfig& appConfig = SystemState::GetConfig();
	const size_t frameSize = static_cast<size_t>(appConfig.GetMemoryPerFrame());
	MemoryManager* memoryManager = MemoryManager::GetInstance();
	const std::string frameMap = memoryManager->GetFrameMapVisualization();

	std::string byteMap;
	if (frameSize > 0) {
		byteMap.reserve(frameMap.size() * frameSize);
		for (const char frameCell : frameMap) {
			byteMap.append(frameSize, frameCell);
		}
	}

	output << "Memory map (frame pool, # = occupied frame, . = free frame):\n";
	output << "Byte map (" << byteMap.size() << " bytes):\n" << byteMap << "\n";
	if (!frameMap.empty()) {
		output << "Frame map (" << frameMap.size() << " frames, "
			<< frameSize << " bytes each):\n" << frameMap << "\n";
	}
}
