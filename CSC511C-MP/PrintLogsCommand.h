#pragma once

#include "ICommand.h"
#include <string>
#include <memory>

class Process;

class PrintLogsCommand : public ICommand {
public:
	explicit PrintLogsCommand(std::shared_ptr<Process> process);

	bool Execute(const std::vector<std::string>& args) const override;
	std::string Name() const override;
	std::string Description() const override;

private:
	std::shared_ptr<Process> process;

	void AppendToLog(const std::string& text) const;
};
