#pragma once

class IHelpDisplayable {
public:
	virtual ~IHelpDisplayable() = default;
	// Returns a string containing the help text for this object. This should include the command name, description, and any relevant usage information.
	virtual void PrintHelp() const = 0;
};