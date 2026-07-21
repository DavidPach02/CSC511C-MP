#include "MemoryLogger.h"

#include "MemoryManager.h"
#include "Process.h"
#include "ProcessManager.h"
#include "SystemState.h"
#include "TimeUtility.h"
#include "MemoryManager.h"
#include "CPUTicker.h"
#include "CPUManager.h"
#include "TextFormatter.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace {
	struct MemorySegment {
		std::string processName;
		size_t lowerAddress;
		size_t upperAddress;
	};

	struct RunningProcessEntry {
		std::string processName;
		int coreId;
	};

	size_t CalculateExternalFragmentationBytes(const std::string& memoryMap) {
		size_t totalFreeBytes = 0;

		for (const char cell : memoryMap) {
			if (cell == '.') {
				++totalFreeBytes;
			}
		}

		return totalFreeBytes;
	}

	std::vector<MemorySegment> BuildLoadedSegments(const std::vector<std::shared_ptr<Process>>& processes, MemoryManager* memoryManager) {
		std::vector<MemorySegment> segments;
		segments.reserve(processes.size());

		for (const auto& process : processes) {
			if (process == nullptr || !process->HasMemoryLoaded()) {
				continue;
			}

			const auto memoryOffset = memoryManager->GetAddressOffset(process->GetMemoryAddress());
			if (!memoryOffset.has_value()) {
				continue;
			}

			const size_t lowerAddress = memoryOffset.value();
			const size_t upperAddress = lowerAddress + process->GetMemoryRequired();
			segments.push_back(MemorySegment{ process->GetName(), lowerAddress, upperAddress });
		}

		std::sort(segments.begin(), segments.end(), [](const MemorySegment& left, const MemorySegment& right) {
			if (left.upperAddress != right.upperAddress) {
				return left.upperAddress > right.upperAddress;
			}
			return left.processName < right.processName;
		});

		return segments;
	}

	std::vector<RunningProcessEntry> BuildRunningProcesses(const std::vector<std::shared_ptr<Process>>& processes) {
		std::vector<RunningProcessEntry> runningProcesses;
		runningProcesses.reserve(processes.size());

		for (const auto& process : processes) {
			if (process == nullptr || process->GetStatusEnum() != ProcessStatus::Running) {
				continue;
			}

			runningProcesses.push_back(RunningProcessEntry{ process->GetName(), process->GetCoreID() });
		}

		std::sort(runningProcesses.begin(), runningProcesses.end(), [](const RunningProcessEntry& left, const RunningProcessEntry& right) {
			if (left.coreId != right.coreId) {
				return left.coreId < right.coreId;
			}
			return left.processName < right.processName;
		});

		return runningProcesses;
	}
}

void MemoryLogger::LogTickSnapshot(uint64_t tick) {
	if (!SystemState::IsInitialized()) {
		return;
	}

	MemoryManager* memoryManager = MemoryManager::GetInstance();
	ProcessManager* processManager = ProcessManager::GetInstance();
	if (memoryManager == nullptr || processManager == nullptr) {
		return;
	}

	const auto& allProcesses = processManager->GetAllProcesses();
	const std::vector<MemorySegment> loadedSegments = BuildLoadedSegments(allProcesses, memoryManager);
	const std::vector<RunningProcessEntry> runningProcesses = BuildRunningProcesses(allProcesses);
	const std::string memoryMap = memoryManager->GetVisualizedMemory();
	const size_t externalFragmentationBytes = CalculateExternalFragmentationBytes(memoryMap);
	const size_t totalMemory = memoryManager->GetTotalMemory();

	const std::filesystem::path outputDirectory = "../output";
	std::error_code directoryError;
	std::filesystem::create_directories(outputDirectory, directoryError);

	const std::filesystem::path outputFilePath = outputDirectory / ("memory_stamp_" + std::to_string(tick) + ".txt");
	std::ofstream outputFile(outputFilePath, std::ios::out | std::ios::trunc);
	if (!outputFile.is_open()) {
		return;
	}

	outputFile << "Timestamp: (" << TimeUtility::GetCurrentDateString("/")
		<< " " << TimeUtility::GetCurrentTimeString(false, ":") << ")\n";
	outputFile << memoryManager->GetMemoryStats();
	outputFile << "Number of processes in memory: " << loadedSegments.size() << "\n";
	outputFile << "Total external fragmentation in bytes: " << externalFragmentationBytes << "\n\n";

	outputFile << "Processes currently running in CPU: " << runningProcesses.size() << "\n";
	if (runningProcesses.empty()) {
		outputFile << "(none)\n\n";
	} else {
		for (const RunningProcessEntry& runningProcess : runningProcesses) {
			outputFile << "- " << runningProcess.processName << " (core " << runningProcess.coreId << ")\n";
		}
		outputFile << "\n";
	}

	outputFile << "----end----- = " << totalMemory << "\n\n";
	for (const MemorySegment& segment : loadedSegments) {
		outputFile << segment.upperAddress << "\n";
		outputFile << segment.processName << "\n";
		outputFile << segment.lowerAddress << "\n\n";
	}
	outputFile << "----start---- = 0\n";
}

std::string MemoryLogger::PrintMemoryStats() {
	const size_t totalTableWidth = 80;

	std::stringstream cpuUtilStream;
	cpuUtilStream << std::fixed << std::setprecision(2) << CPUManager::GetInstance()->GetCPUUtilization() << "%";

	std::stringstream memoryUtilStream;
	memoryUtilStream << std::fixed << std::setprecision(2) << MemoryManager::GetInstance()->GetMemoryUtilization() << "%";

	std::stringstream memoryUsageStream;
	memoryUsageStream << MemoryManager::GetInstance()->GetUsedMemory() << " / " << MemoryManager::GetInstance()->GetTotalMemory() << " MiB";

	// Print formatted table here
	std::stringstream previewInfo;
	previewInfo << "+" << TextFormatter::GetPrintedBorder('-', totalTableWidth) << "+\n"
		<< "| " << TextFormatter::GetFormattedCell("PROCESS-SMI V01.01", totalTableWidth / 2 - 1)
		<< TextFormatter::GetFormattedCell("DRIVER VERSION: 01.00", totalTableWidth / 2 - 1, TextFormatter::RIGHT) << " |\n"
		<< "+" << TextFormatter::GetPrintedBorder('-', totalTableWidth) << "+\n"
		<< "| " << TextFormatter::GetFormattedCell("CPU Utilization:", 25)
		<< TextFormatter::GetFormattedCell(cpuUtilStream.str(), totalTableWidth - 27) << " |\n"
		<< "| " << TextFormatter::GetFormattedCell("Memory Utilization:", 25)
		<< TextFormatter::GetFormattedCell(memoryUtilStream.str(), 8)
		<< TextFormatter::GetFormattedCell("Memory Usage:", 20, TextFormatter::RIGHT)
		<< TextFormatter::GetFormattedCell(memoryUsageStream.str(), 25, TextFormatter::RIGHT) << " |\n"
		<< "+" << TextFormatter::GetPrintedBorder('-', totalTableWidth) << "+\n\n";

	const size_t coreIDWidth = 4;
	const size_t processIDWidth = 5;
	const size_t processNameWidth = 52;
	const size_t memoryUsageWidth = 15;
	std::stringstream processHeaderInfo;
	processHeaderInfo << "+" << TextFormatter::GetPrintedBorder('-', totalTableWidth) << "+\n"
		<< "| " << TextFormatter::GetFormattedCell("PROCESSES:", totalTableWidth - 2) << " |\n"
		<< "| " << TextFormatter::GetFormattedCell("Core", coreIDWidth, TextFormatter::CENTER)
		<< TextFormatter::GetFormattedCell("PID", processIDWidth, TextFormatter::CENTER) << " "
		<< TextFormatter::GetFormattedCell("Process Name", processNameWidth, TextFormatter::CENTER) << " "
		<< TextFormatter::GetFormattedCell("Memory Usage", memoryUsageWidth, TextFormatter::CENTER) << " |\n"
		<< "|" << TextFormatter::GetPrintedBorder('=', totalTableWidth) << "|\n";


	std::stringstream processInfo;
	ProcessManager* processManager = ProcessManager::GetInstance();
	const auto& allProcesses = processManager->GetAllProcesses();
	for (const auto& process : allProcesses) {
		if (process->GetStatusEnum() == ProcessStatus::Running) {
			processInfo << "| " << TextFormatter::GetFormattedCell(std::to_string(process->GetCoreID()), coreIDWidth, TextFormatter::RIGHT)
				<< TextFormatter::GetFormattedCell(std::to_string(process->GetID()), processIDWidth, TextFormatter::RIGHT) << " "
				<< TextFormatter::GetFormattedCell(process->GetName(), processNameWidth, TextFormatter::LEFT, true) << " "
				<< TextFormatter::GetFormattedCell(std::to_string(process->GetMemoryRequired()) + " MiB", memoryUsageWidth, TextFormatter::RIGHT) << " |\n";
		}
	}
	processInfo << "+" << TextFormatter::GetPrintedBorder('-', totalTableWidth) << "+\n";

	return previewInfo.str() + processHeaderInfo.str() + processInfo.str();
}

std::string MemoryLogger::PrintVirtualMemoryStats() {
	const size_t totalTableWidth = 50;

	std::stringstream virtualMemoryInfo;
	virtualMemoryInfo << TextFormatter::GetFormattedCell(std::to_string(MemoryManager::GetInstance()->GetTotalMemory()), 12, TextFormatter::RIGHT) << " " << TextFormatter::GetFormattedCell("MiB Total Memory", 38) << "\n"
		<< TextFormatter::GetFormattedCell(std::to_string(MemoryManager::GetInstance()->GetUsedMemory()), 12, TextFormatter::RIGHT) << " " << TextFormatter::GetFormattedCell("MiB Used Memory", 38) << "\n"
		<< TextFormatter::GetFormattedCell(std::to_string(MemoryManager::GetInstance()->GetFreeMemory()), 12, TextFormatter::RIGHT) << " " << TextFormatter::GetFormattedCell("MiB Free Memory", 38) << "\n"
		<< TextFormatter::GetFormattedCell(std::to_string(ProcessManager::GetInstance()->GetRunningProcessCount()), 12, TextFormatter::RIGHT) << " " << TextFormatter::GetFormattedCell("Active Processes", 38) << "\n"
		<< TextFormatter::GetFormattedCell(std::to_string(ProcessManager::GetInstance()->GetInactiveProcessCount()), 12, TextFormatter::RIGHT) << " " << TextFormatter::GetFormattedCell("Inactive Processes", 38) << "\n"
		<< TextFormatter::GetFormattedCell(std::to_string(CPUTicker::GetInstance()->GetIdleTickTime()), 12, TextFormatter::RIGHT) << " " << TextFormatter::GetFormattedCell("Idle CPU Ticks", 38) << "\n"
		<< TextFormatter::GetFormattedCell(std::to_string(CPUTicker::GetInstance()->GetActiveTickTime()), 12, TextFormatter::RIGHT) << " " << TextFormatter::GetFormattedCell("Active CPU Ticks", 38) << "\n"
		<< TextFormatter::GetFormattedCell(std::to_string(CPUTicker::GetInstance()->GetCurrentTick()), 12, TextFormatter::RIGHT) << " " << TextFormatter::GetFormattedCell("Total CPU Ticks", 38) << "\n"
		<< TextFormatter::GetFormattedCell(std::to_string(MemoryManager::GetInstance()->GetPagesPagedIn()), 12, TextFormatter::RIGHT) << " " << TextFormatter::GetFormattedCell("Pages paged in", 38) << "\n"
		<< TextFormatter::GetFormattedCell(std::to_string(MemoryManager::GetInstance()->GetPagesPagedOut()), 12, TextFormatter::RIGHT) << " " << TextFormatter::GetFormattedCell("Pages paged out", 38) << "\n";

	return virtualMemoryInfo.str();
}
