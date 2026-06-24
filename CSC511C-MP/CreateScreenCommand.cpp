#include "CreateScreenCommand.h"

bool CreateScreenCommand::Execute(const std::vector<std::string>& args) const
{
	int argsCount = args.size();
	if (argsCount <= 1 || argsCount > 2)
	{
		std::cout << "\033[31mUsage: screen -s <process_name>\033[0m\n";
		return true;
	}

	const AppConfig& config = SystemState::GetConfig();
	DummyProcessGenerator::GenerateOne(config, args[1]);
	ConsoleManager::GetInstance()->SwitchScreen(args[1]);

    return true;
}

std::string CreateScreenCommand::Name() const { return "screen -s"; }
std::string CreateScreenCommand::Description() const { return "Creates a new screen with an attached process."; }
