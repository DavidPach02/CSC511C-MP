#pragma once

#include "CommandDispatcher.h"
#include <string>

class AConsole {
public:
	AConsole(const std::string processName);
	virtual ~AConsole() = default;

	std::string GetName() const;

	virtual void OnEnabled() = 0;
	virtual void Display() = 0; // Draw the screen
	virtual void Update() = 0; // Input or process algo

protected:
	std::unique_ptr<CommandDispatcher> commandDispatcher;
	std::string name;
};