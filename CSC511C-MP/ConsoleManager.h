#pragma once

#include "AConsole.h"
#include "BaseScreen.h"
#include "CommandDispatcher.h"
#include <unordered_map>

const std::string MAIN_CONSOLE_NAME = "MAIN_CONSOLE_NAME";

class BaseScreen;
class AConsole;

class ConsoleManager
{
public:
	static ConsoleManager* GetInstance();
	static void Initialize();
	static void Destroy();

	void Process();
	void Render();

	void RegisterScreen(const std::shared_ptr<BaseScreen> screen, bool announce = true);
	void UnregisterScreen(const std::string& name);
	void SwitchScreen(const std::string& name);
	bool HasScreen(const std::string& name) const;
	bool IsOnMainConsole() const;

	void ReturnToPreviousScreen();
	void Exit();

	bool GetIsRunnning() const;

private:
	ConsoleManager();
	~ConsoleManager() = default;
	ConsoleManager(const ConsoleManager&) = delete;
	ConsoleManager& operator=(const ConsoleManager&) = delete;
	static ConsoleManager* instance;

	std::unordered_map<std::string, std::shared_ptr<AConsole>> screenTable;

	std::shared_ptr<AConsole> currentConsole;
	std::shared_ptr<AConsole> previousConsole;
	CommandDispatcher commandDispatcher;

	bool running = true;
};

