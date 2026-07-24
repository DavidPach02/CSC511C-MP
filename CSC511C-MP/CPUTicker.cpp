#include "CPUTicker.h"
#include "SystemState.h"
#include "DummyProcessGenerator.h"
#include "MemoryLogger.h"
#include "ProcessManager.h"
#include <chrono>
#include <mutex>
#include <thread>

CPUTicker* CPUTicker::instance = nullptr;

CPUTicker* CPUTicker::GetInstance() {
	if (instance == nullptr) {
		instance = new CPUTicker();
	}
	return instance;
}

void CPUTicker::Start() {
	GetInstance()->running = true;
	// If the ticker thread is not joinable, start it
	if (!GetInstance()->tickerThread.joinable()) {
		GetInstance()->tickerThread = std::thread(&CPUTicker::Run, GetInstance());
	}
}

void CPUTicker::Stop() {
	if (instance == nullptr) {
		return;
	}

	instance->running = false;
	instance->generating = false;
	instance->tickCondition.notify_all();

	if (instance->tickerThread.joinable()) {
		instance->tickerThread.join();
	}
}

CPUTicker::CPUTicker()
	: running(false), generating(false), currentTick(0) {
}

CPUTicker::~CPUTicker() {
	Stop();
}

void CPUTicker::SetGenerationEnabled(bool enabled) {
	generating = enabled;
}

void CPUTicker::StopGeneration() {
	generating = false;
}

bool CPUTicker::IsGenerationEnabled() const {
	return generating;
}

uint64_t CPUTicker::GetCurrentTick() const {
	return currentTick.load();
}

uint64_t CPUTicker::GetIdleTickTime() const {
	return idleTickTime.load();
}

uint64_t CPUTicker::GetActiveTickTime() const {
	return activeTickTime.load();
}

void CPUTicker::WaitUntilTick(uint64_t targetTick) const {
	std::unique_lock<std::mutex> lock(tickMutex);
	tickCondition.wait(lock, [this, targetTick] {
		return currentTick.load() >= targetTick || !running.load();
	});
}

void CPUTicker::OnTick() {
	if (!generating) {
		return;
	}

	const AppConfig& config = SystemState::GetConfig();
	const uint64_t tick = currentTick.load();
	
	// If the tick is 0 or not a multiple of the batch process frequency, return
	if (tick == 0 || tick % static_cast<uint64_t>(config.GetBatchProcessFreq()) != 0) {
		return;
	}

	// Generate a new process
	DummyProcessGenerator::GenerateOne(config);
}

void CPUTicker::Run() {
	while (running) {
		const int tickerDelayMs = SystemState::GetConfig().GetTickerDelayMs();
		const int quantumCycles = SystemState::GetConfig().GetQuantumCycles();

		{
			std::lock_guard<std::mutex> lock(tickMutex);
			++currentTick;

			// Update idle and active tick times
			if (ProcessManager::GetInstance()->GetRunningProcessCount() > 0) {
				activeTickTime++;
			}
			else {
				idleTickTime++;
			}
		}
 
		tickCondition.notify_all();

		OnTick();

		const uint64_t tick = currentTick.load();
		if (quantumCycles > 0 && tick > 0 && tick % static_cast<uint64_t>(quantumCycles) == 0) {
			const uint64_t quantumCycle = tick / static_cast<uint64_t>(quantumCycles);
			//MemoryLogger::LogTickSnapshot(quantumCycle);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(tickerDelayMs));
	}
}
