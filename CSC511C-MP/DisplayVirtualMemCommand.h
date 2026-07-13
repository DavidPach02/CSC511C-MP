#pragma once

#include "ICommand.h"

class DisplayVirtualMemCommand : public ICommand {
public:
	DisplayVirtualMemCommand() = default;
	bool Execute(const std::vector<std::string>& args) const override;
	std::string Name() const override;
	std::string Description() const override;
};

