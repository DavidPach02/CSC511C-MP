#include <iostream>
#include <string>
#include <Windows.h>
#include <cstdlib>
#include "Commands.h"
#include "CommandDispatcher.h"
#include "ICommand.h"

int main()
{
    std::cout << "\n";
    std::cout << "====================================================================================\n\n";
    std::cout << "  ,ad8888ba,    ad88888ba     ,ad8888ba,   8888888888    88      88    ,ad8888ba,   \n";
    std::cout << " d8\"'    `\"8b  d8\"     \"8b   d8\"'    `\"8b  88          ,d88    ,d88   d8\"'    `\"8b\n";
    std::cout << "d8'            Y8,          d8'            88  ____  888888  888888  d8'\n";
    std::cout << "88             `Y8aaaaa,    88             88a8PPPP8b,   88      88  88\n";
    std::cout << "88               `\"\"\"\"\"8b,  88             PP\"     `8b   88      88  88\n";
    std::cout << "Y8,                    `8b  Y8,                     d8   88      88  Y8,\n";
    std::cout << " Y8a.    .a8P  Y8a     a8P   Y8a.    .a8P  Y8a     a8P   88      88   Y8a.    .a8P  \n";
    std::cout << "  `\"Y8888Y\"'    \"Y88888P\"     `\"Y8888Y\"'    \"Y88888P\"    88      88    `\"Y8888Y\"'\n\n";
    std::cout << "====================================================================================\n";
    std::cout << "Welcome to CSC511C Command Line!\n";
    std::cout << "Type 'exit' to quit, 'clear' to clear the screen.\n\n";

    CommandDispatcher dispatcher;
	dispatcher.Register(std::make_unique<ExitCommand>());
	dispatcher.Register(std::make_unique<ClearCommand>());
	dispatcher.Register(std::make_unique<InitializeCommand>());
	dispatcher.Register(std::make_unique<ScreenCommand>());
	dispatcher.Register(std::make_unique<SchedulerStartCommand>());
	dispatcher.Register(std::make_unique<SchedulerStopCommand>());
	dispatcher.Register(std::make_unique<HelpCommand>());
	dispatcher.Register(std::make_unique<ReportUtilCommand>());

    bool running = true;
    while (running) {
        std::cout << "Enter a command: ";
        std::string command;
        std::getline(std::cin, command);

		running = dispatcher.DispatchCommand(command);
		std::cout << "\n";
    }
    
    return 0;
}

