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
    std::cout << "Type '\033[33mexit\033[0m' to quit, '\033[33mclear\033[0m' to clear the screen, '\033[33mhelp\033[0m' to display other commands.\n\n";

    // Create command dispatcher object
    CommandDispatcher dispatcher;
	// Initialize the dispatcher with available commands
	CommandDispatcher::Initialize(dispatcher);

    bool running = true;
	// Main command loop
    while (running) {
		// Prompt the user for input
        std::cout << "Enter a command: ";
        std::string command;
        std::getline(std::cin, command);

		// Dispatch the command and update the running state based on the command's return value
		running = dispatcher.DispatchCommand(command);
    }
    
    return 0;
}

