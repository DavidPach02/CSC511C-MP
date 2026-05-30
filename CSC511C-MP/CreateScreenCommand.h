#pragma once

#include "ICommand.h"
#include "ConsoleManager.h"
#include "BaseScreen.h"
#include <iostream>

class CreateScreenCommand : public ICommand {
public:
	CreateScreenCommand() = default;
	bool Execute(const std::vector<std::string>& args) const override;
	std::string Name() const override;
	std::string Description() const override;
	std::string Argument() const override { return "-s"; }
};

