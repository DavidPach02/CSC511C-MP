#pragma once

#include "ICommand.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class CommandDispatcher {
public:
	static void Initialize(CommandDispatcher& dispatcher);
	void Register(std::unique_ptr<ICommand> command);
	bool DispatchCommand(const std::string& input) const;
	void PrintHelp() const;

private:
	static std::vector<std::string> Tokenize(const std::string& input);
	std::unordered_map<std::string, std::unique_ptr<ICommand>> m_commands;
};