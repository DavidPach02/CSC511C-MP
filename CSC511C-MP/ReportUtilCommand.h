#pragma once

#include "ICommand.h"
#include "ConsoleManager.h"
#include "TimeUtility.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

class ReportUtilCommand : public ICommand {
public:
	bool Execute(const std::vector<std::string>& args) const override;
	std::string Name() const override;
	std::string Description() const override;
};