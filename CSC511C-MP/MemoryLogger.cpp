#include "MemoryLogger.h"

#include "MemoryManager.h"
#include "Process.h"
#include "ProcessManager.h"
#include "SystemState.h"
#include "TimeUtility.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {
	struct MemorySegment {
		std::string processName;
		size_t lowerAddress;
		size_t upperAddress;
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

	const std::vector<MemorySegment> loadedSegments = BuildLoadedSegments(processManager->GetAllProcesses(), memoryManager);
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
	outputFile << "Number of processes in memory: " << loadedSegments.size() << "\n";
	outputFile << "Total external fragmentation in bytes: " << externalFragmentationBytes << "\n\n";

	outputFile << "----end----- = " << totalMemory << "\n\n";
	for (const MemorySegment& segment : loadedSegments) {
		outputFile << segment.upperAddress << "\n";
		outputFile << segment.processName << "\n";
		outputFile << segment.lowerAddress << "\n\n";
	}
	outputFile << "----start---- = 0\n";
}
