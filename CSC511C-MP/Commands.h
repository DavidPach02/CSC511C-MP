#pragma once

#include "ICommand.h"
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <cstdlib>

class ExitCommand : public ICommand {
public:
	bool Execute(const std::vector<std::string>& args) const override {
		std::cout << "Shutting down system.";
		Sleep(5);
		return false;
	}
	std::string Name() const override{ return "exit"; }
	std::string Description() const override { return "Shuts down the system."; }
};

class ClearCommand : public ICommand {
public:
	bool Execute(const std::vector<std::string>& args) const override {
		std::system("cls");
		return true;
	}
	std::string Name() const override { return "clear"; }
	std::string Description() const override { return "Clears the screen."; }
};

class InitializeCommand : public ICommand {
public:
	bool Execute(const std::vector<std::string>& args) const override {
		std::cout << Name() + " command recognized. Doing something.\n";
		return true;
	}
	std::string Name() const override { return "initialize"; }
	std::string Description() const override { return "Boots up the OS simulator."; }
};

class ScreenCommand : public ICommand {
public:
	bool Execute(const std::vector<std::string>& args) const override {
		std::cout << Name() + " command recognized. Doing something.\n";
		return true;
	}
	std::string Name() const override { return "screen"; }
	std::string Description() const override { return "Displays all active sessions."; }
};

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

class ReportUtilCommand : public ICommand {
public:
	bool Execute(const std::vector<std::string>& args) const override {
		std::cout << Name() + " command recognized. Doing something.\n";
		return true;
	}
	std::string Name() const override { return "report-util"; }
	std::string Description() const override { return "Takes a snapshot of the active sessions and creates a txt file."; }
};

class HelpCommand : public ICommand {
public:
	bool Execute(const std::vector<std::string>& args) const override {
		//m_dispatcher.PrintHelp();
		std::cout << Name() + " command recognized. Doing something.\n";
		return true;
	}
	std::string Name() const override { return "help"; }
	std::string Description() const override { return "Displays the available commands."; }
private:
	//const CommandDispatcher* m_dispatcher;
};