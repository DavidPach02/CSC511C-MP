#include "CreateScreenCommand.h"

bool CreateScreenCommand::Execute(const std::vector<std::string>& args) const
{
	if (args.empty())
	{
		std::cout << "\033[31mUsage: screen -r <process_name>\033[0m\n";
		return true;
	}

	std::string screenName = args[0];
	ConsoleManager::GetInstance()->RegisterScreen(std::make_shared<BaseScreen>(screenName));

    return true;
}

std::string CreateScreenCommand::Name() const { return "screen -r"; }
std::string CreateScreenCommand::Description() const { return "Creates a new screen with an attached process."; }
