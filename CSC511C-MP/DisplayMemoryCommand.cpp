#include "DisplayMemoryCommand.h"
#include "MemoryManager.h"

bool DisplayMemoryCommand::Execute(const std::vector<std::string>& args) const {
	(void)args;

	MemoryManager* memoryManager = MemoryManager::GetInstance();
	if (memoryManager == nullptr) {
		std::cout << "Memory manager is not initialized." << std::endl;
		return false;
	}
	std::cout << memoryManager->GetMemoryStats() << std::endl;

	return true;
}

std::string DisplayMemoryCommand::Name() const { return "process-smi"; }
std::string DisplayMemoryCommand::Description() const { return "Displays the memory map and statistics of the system."; }
