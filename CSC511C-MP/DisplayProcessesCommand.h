#pragma once

#include "ICommand.h"
#include "CPUManager.h"
#include <iostream>

class DisplayProcessesCommand : public ICommand {
public:
	DisplayProcessesCommand() = default;
	bool Execute(const std::vector<std::string>& args) const override;
	std::string Name() const override;
	std::string Description() const override;
	std::string Argument() const override;
};

