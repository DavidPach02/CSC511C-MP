#include "DisplayVirtualMemCommand.h"
#include "MemoryManager.h"

bool DisplayVirtualMemCommand::Execute(const std::vector<std::string>& args) const {
	(void)args;

	MemoryManager* memoryManager = MemoryManager::GetInstance();
	if (memoryManager == nullptr) {
		std::cout << "Memory manager is not initialized." << std::endl;
		return false;
	}
	std::cout << memoryManager->GetVirtualMemoryStats() << std::endl;

	return true;
}

std::string DisplayVirtualMemCommand::Name() const { return "vmstat"; }
std::string DisplayVirtualMemCommand::Description() const { return "Displays detailed memory usage stats."; }
