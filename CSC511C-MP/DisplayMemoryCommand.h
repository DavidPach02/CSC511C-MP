#pragma once

#include "ICommand.h"

class DisplayMemoryCommand : public ICommand {
public:
	DisplayMemoryCommand() = default;
	bool Execute(const std::vector<std::string>& args) const override;
	std::string Name() const override;
	std::string Description() const override;
};

