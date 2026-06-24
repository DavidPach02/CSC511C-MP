#pragma once

#include "AConsole.h"
#include "InitializeCommand.h"
#include "ClearCommand.h"
#include "HelpCommand.h"

class PrebootScreen : public AConsole {
public:
	PrebootScreen();
	void OnEnabled() override;
	void Update() override;
	void Display() override;
private:
	void PrintPrompt() const;
	bool refreshed = false;
};

