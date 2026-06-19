#include "PrintCommand.h"

#include "Process.h"
#include "TimeUtility.h"
#include <iostream>
#include <fstream>
#include <sstream>

PrintCommand::PrintCommand(const Process& process) : process(process) {
}

bool PrintCommand::Execute(const std::vector<std::string>& args) const {
	std::ostringstream text;
	
	if (args.empty()) {
		// For testing: log a default message with process and command info
		text << "Hello World from " << process.GetName();
	} else {
		// Normal case: concatenate all arguments
		for (size_t index = 0; index < args.size(); ++index) {
			if (index > 0) {
				text << " ";
			}
			text << args[index];
		}
	}

	AppendToLog(text.str());

	return true;
}

void PrintCommand::AppendToLog(const std::string& text) const {
	// Generate process-specific log file path
	std::string logFilePath = "../data/" + process.GetName() + ".txt";
	
	bool isNewFile = !std::ifstream(logFilePath).good();

	std::ofstream logFile(logFilePath, std::ios::out | std::ios::app);

	if (!logFile.is_open()) {
		std::cerr << "\033[31mError: Could not open log file for writing.\033[0m\n";
		return;
	}

	if (isNewFile) {
		logFile << "Logs:\n";
	}

	std::string date = TimeUtility::GetCurrentDateString("/");
	std::string time = TimeUtility::GetCurrentTimeString(false, ":");
	std::string timestamp = "(" + date + " " + time + ")";

	logFile << timestamp << " Core:" << process.GetCoreID() << " \"" << text << "\"\n";
	logFile.close();
}

std::string PrintCommand::Name() const { return "print"; }
std::string PrintCommand::Description() const { return "Prints text to the console and appends it to the log file."; }
