#pragma once

#include "AConsole.h"
#include "Process.h"
#include "ExitScreenCommand.h"
#include "ClearCommand.h"
#include "HelpCommand.h"
#include "PrintLogsCommand.h"

class BaseScreen : public AConsole {
public:
	BaseScreen(std::shared_ptr<Process> process);
	void OnEnabled() override;
	void Update() override;
	void Display() override;

private:
	std::shared_ptr<Process> process;

	void PrintProcessInfo() const;
	void DisplayHeader() const;
	//std::shared_ptr<Process> attachedProcess;
	bool refreshed = false;
};

