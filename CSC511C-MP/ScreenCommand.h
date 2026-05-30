#pragma once

#include "ICommand.h"
#include "IHelpDisplayable.h"
#include "CreateScreenCommand.h"
#include "HelpCommand.h"
#include <iostream>
#include <string>
#include <unordered_map>

class ScreenCommand : public ICommand, public IHelpDisplayable {
public:
	ScreenCommand();
	bool Execute(const std::vector<std::string>& args) const override;
	std::string Name() const override;
	std::string Description() const override;

	void PrintHelp() const override;

private:
	std::unordered_map<std::string, std::unique_ptr<ICommand>> subcommands;
};