#pragma once

#include "BaseScreen.h"

class MainConsole : public AConsole
{
public:
	MainConsole();
	MainConsole(const std::string processName);
	void OnEnabled() override;
	void Process() override;
	void Display() override;

private:
	void PrintHeader() const;
	bool refreshed = false;
};

