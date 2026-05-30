#include "ScreenCommand.h"

ScreenCommand::ScreenCommand() {
	//std::unique_ptr<ICommand> createScreenCMD = std::make_unique<CreateScreenCommand>();
	//this->subcommands.emplace(createScreenCMD->Name(), std::move(createScreenCMD));
}

bool ScreenCommand::Execute(const std::vector<std::string>& args) const {
	if (args.empty()) {
		std::cout << "\033[31mUsage:screen -r <process_name>\033[0m\n";
		return true;
	}

	// If users typed -r args then we will execute the create screen command
	if (args[0] == "-r")
	{
		if (args.size() < 2)
		{
			std::cout << "\033[31mUsage: screen -r <process_name>\033[0m\n";
			return true;
		}

		ConsoleManager::GetInstance()->RegisterScreen(std::make_shared<BaseScreen>(args[1]));
	}


	return true;
}

std::string ScreenCommand::Name() const { return "screen"; }
std::string ScreenCommand::Description() const { return "Displays all active sessions."; }