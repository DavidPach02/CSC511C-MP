#pragma once

#include "ICommand.h"

class CreateProcessInstructionCommand : public ICommand {
public:
	CreateProcessInstructionCommand() = default;
	bool Execute(const std::vector<std::string>& args) const override;
	std::string Name() const override;
	std::string Description() const override;
	std::string Argument() const override { return "-c"; }
};

