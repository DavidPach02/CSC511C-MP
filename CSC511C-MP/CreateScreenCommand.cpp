#include "CreateScreenCommand.h"
#include "ProcessManager.h"
#include "Scheduler.h"
#include "CPUTicker.h"

bool CreateScreenCommand::Execute(const std::vector<std::string>& args) const
{
	int argsCount = args.size();
	if (argsCount <= 1 || argsCount > 2)
	{
		std::cout << "\033[31mUsage: screen -s <process_name>\033[0m\n";
		return true;
	}

	const std::string processName = args[1];
	std::shared_ptr<Process> existingProcess = ProcessManager::GetInstance()->GetProcessByName(processName);

	if (existingProcess == nullptr) {
		const AppConfig& config = SystemState::GetConfig();

		if (Scheduler::GetInstance() == nullptr) {
			Scheduler::Initialize(
				config.GetNumCpu(),
				config.GetSchedulerAlgorithm(),
				config.GetQuantumCycles());
		}

		CPUTicker::Start();
		if (!Scheduler::GetInstance()->IsRunning()) {
			Scheduler::GetInstance()->Start();
		}

		if (!DummyProcessGenerator::GenerateOne(config, processName)) {
			std::cout << "\033[31mFailed to create process: " << processName << "\033[0m\n";
			return true;
		}

		std::cout << "Process created successfully: " << processName << "\n";
		
	}

	ConsoleManager::GetInstance()->SwitchScreen(processName);

    return true;
}

std::string CreateScreenCommand::Name() const { return "screen -s"; }
std::string CreateScreenCommand::Description() const { return "Creates a new screen with an attached process."; }
