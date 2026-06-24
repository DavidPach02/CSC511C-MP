#include "ReportUtilCommand.h"
#include "ProcessReport.h"
#include <fstream>
#include <iostream>

bool ReportUtilCommand::Execute(const std::vector<std::string>& args) const {
	(void)args;

	const std::string outputPath = "../data/csopesy-log.txt";
	std::ofstream logFile(outputPath, std::ios::out | std::ios::trunc);

	if (!logFile.is_open()) {
		std::cerr << "Error: Could not open ../data/csopesy-log.txt for writing.\n";
		return true;
	}

	ProcessReport::WriteSummary(logFile);
	logFile.close();

	std::cout << "Success: Report saved to " << outputPath << "\n";
	return true;
}

std::string ReportUtilCommand::Name() const { return "report-util"; }
std::string ReportUtilCommand::Description() const {
	return "Saves CPU and process summary to ../data/csopesy-log.txt.";
}
