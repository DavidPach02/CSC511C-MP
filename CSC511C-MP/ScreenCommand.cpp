#include "ScreenCommand.h"

ScreenCommand::ScreenCommand() {
	//CreateScreenCommand createScreenCMD = CreateScreenCommand();
	std::unique_ptr<ICommand> createScreenCMD = std::make_unique<CreateScreenCommand>();
	this->subcommands.emplace(createScreenCMD->Argument(), std::move(createScreenCMD));

	std::unique_ptr<ICommand> helpCMD = std::make_unique<HelpCommand>(this);
	this->subcommands.emplace(helpCMD->Argument(), std::move(helpCMD));

	std::unique_ptr<ICommand> displayProcessesCMD = std::make_unique<DisplayProcessesCommand>();
	this->subcommands.emplace(displayProcessesCMD->Argument(), std::move(displayProcessesCMD));

	std::unique_ptr<ICommand> switchScreenCMD = std::make_unique<SwitchScreenCommand>();
	this->subcommands.emplace(switchScreenCMD->Argument(), std::move(switchScreenCMD));

	std::unique_ptr<ICommand> createProcessInstructionCMD = std::make_unique<CreateProcessInstructionCommand>();
	this->subcommands.emplace(createProcessInstructionCMD->Argument(), std::move(createProcessInstructionCMD));
}

bool ScreenCommand::Execute(const std::vector<std::string>& args) const {
	if (args.size() <= 0) {
		std::cout << "\033[31mUsage: screen <args>\033[0m\n";
		return true;
	}

	auto subcommand = this->subcommands.find(args[0]);
	if (subcommand == subcommands.end()) {
		// If the subcommand is not found, print an error message
		std::string argsList = "";
		for (size_t i = 0; i < args.size(); i++) {
			argsList += args[i] + " ";
		}
		std::cout << "\033[31mUnknown command: " << argsList << "\033[0m\n";
		// Return true to continue running the application even if the command is unknown
		return true;
	}

	return subcommand->second->Execute(args);
}

std::string ScreenCommand::Name() const { return "screen"; }
std::string ScreenCommand::Description() const { return "Displays all active sessions."; }

void ScreenCommand::PrintHelp() const {
	std::cout << "\033[34m" << this->Name() << " - " << this->Description() << "\033[0m\n";
	std::cout << "Usage: " << this->Name() << " \033[33m<args>\033[0m\n";
	std::cout << "Available subcommands:\n";
	for (const auto& pair : subcommands) {
		std::cout << "  [ \033[33m" << pair.second->Argument() << "\033[0m ] " << pair.second->Description() << "\n";
	}
}