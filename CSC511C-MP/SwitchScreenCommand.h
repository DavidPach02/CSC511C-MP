#pragma once

#include "ICommand.h"
#include "ConsoleManager.h"
#include <iostream>

class SwitchScreenCommand : public ICommand {
public:
	SwitchScreenCommand() = default;
	bool Execute(const std::vector<std::string>& args) const override;
	std::string Name() const override;
	std::string Description() const override;
	std::string Argument() const override { return "-r"; }
};
