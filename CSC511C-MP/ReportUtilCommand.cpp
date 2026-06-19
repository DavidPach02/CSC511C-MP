#include "ReportUtilCommand.h"
#include <cerrno>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

namespace {
	bool ensureDirectoryExists(const std::string& directoryPath) {
#ifdef _WIN32
		if (_mkdir(directoryPath.c_str()) == 0) {
			return true;
		}
#else
		if (mkdir(directoryPath.c_str(), 0755) == 0) {
			return true;
		}
#endif
		return errno == EEXIST;
	}
}

bool ReportUtilCommand::Execute(const std::vector<std::string>& args) const {
	const std::string directoryPath = "./logs";

	if (!ensureDirectoryExists(directoryPath)) {
		std::cerr << "Error: Could not create log directory." << std::endl;
		return true;
	}

	std::string currentDate = TimeUtility::GetCurrentDateString();
	std::string currentTime = TimeUtility::GetCurrentTimeString();
	const std::string filename = "process_dump_" + currentDate + "_" + currentTime + ".txt";
	const std::string fullPath = directoryPath + "/" + filename;
	std::ofstream logFile(fullPath, std::ios::out | std::ios::trunc);

	if (logFile.is_open()) {
		logFile << CPUManager::GetInstance()->GetSnapshotLog().str();
		logFile.close();
		std::cout << "Success: Logs exported to " << fullPath << std::endl;
	}
	else {
		std::cerr << "Error: Could not open log file for writing." << std::endl;
	}

	return true;
}

std::string ReportUtilCommand::Name() const { return "report-util"; }
std::string ReportUtilCommand::Description() const { return "Takes a snapshot of the active sessions and creates a txt file."; }
