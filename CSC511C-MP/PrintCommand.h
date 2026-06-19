#pragma once

#include "ICommand.h"
#include <string>

class Process;

class PrintCommand : public ICommand {
public:
	explicit PrintCommand(const Process& process);

	bool Execute(const std::vector<std::string>& args) const override;
	std::string Name() const override;
	std::string Description() const override;

private:
	const Process& process;

	void AppendToLog(const std::string& text) const;
};
