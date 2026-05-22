#pragma once

#include "ICommand.h" 
#include "CommandDispatcher.h"
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <cstdlib>

class SchedulerStartCommand : public ICommand {
public:
	bool Execute(const std::vector<std::string>& args) const override {
		std::cout << Name() + " command recognized. Doing something.\n";
		return true;
	}
	std::string Name() const override { return "scheduler-start"; }
	std::string Description() const override { return "Starts the scheduler."; }
};

class SchedulerStopCommand : public ICommand {
public:
	bool Execute(const std::vector<std::string>& args) const override {
		std::cout << Name() + " command recognized. Doing something.\n";
		return true;
	}
	std::string Name() const override { return "scheduler-stop"; }
	std::string Description() const override { return "Stops the scheduler."; }
};