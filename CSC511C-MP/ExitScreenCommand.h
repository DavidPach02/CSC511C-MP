#pragma once

#include "ICommand.h"
#include "ConsoleManager.h"

class ExitScreenCommand : public ICommand {
public:
	bool Execute(const std::vector<std::string>& args) const override;
	std::string Name() const override;
	std::string Description() const override;
};

