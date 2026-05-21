#pragma once

#include <string>
#include <vector>

class ICommand {
public:
	virtual ~ICommand() = default;
	virtual bool Execute(const std::vector<std::string>& args) const = 0;
	virtual std::string Name() const = 0;
	virtual std::string Description() const = 0;
};
