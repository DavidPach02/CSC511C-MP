#include "ReportUtilCommand.h"

bool ReportUtilCommand::Execute(const std::vector<std::string>& args) const {
	//std::cout << Name() + " command recognized. Doing something.\n";
	// Get current screen
	// Make the screen have a log functionality 
	//ConsoleManager::GetInstance()->Get
	
	// Get the path directory
	std::string directoryPath = "./logs";

	// Check if it exists
	if (!std::filesystem::exists(directoryPath)) {
		// Create a new file directory if it doesn't exist
		std::filesystem::create_directory(directoryPath);
	}

	// Get current date and time
	std::string currentDate = TimeUtility::GetCurrentDateString();
	std::string currentTime = TimeUtility::GetCurrentTimeString();
	// Get the file name
	std::string filename = "/process_dump_" + currentDate + "_" + currentTime + ".txt";
	// Get full path name
	std::string fullPath = directoryPath + filename;
	// Initialize the log file
	std::ofstream logFile(fullPath, std::ios::out | std::ios::trunc);

	// Check if the log file is open
	if (logFile.is_open()) {
		// TODO: Get the text from the log
		
		// Write the text in a text file
		logFile << CPUManager::GetInstance()->GetSnapshotLog().str();
		
		// Save it inside the folder
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