#pragma once

#include "AConsole.h"

class BaseScreen : public AConsole
{
public:
	// TODO: Add process in 
	BaseScreen(const std::string processName);
	void OnEnabled() override;
	void Process() override;
	void Display() override;

private:
	void PrintProcessInfo() const;
	//std::shared_ptr<Process> attachedProcess;
	bool refreshed = false;
};

