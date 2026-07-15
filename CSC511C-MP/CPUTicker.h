#pragma once

#include <atomic>
#include <cstdint>
#include <thread>
#include <mutex>
#include <condition_variable>

// Global simulation clock (background std::thread). Increments currentTick each frame.
// Drives batch-process-freq spawning (OnTick) and delays-per-exec waits (WaitUntilTick).
class CPUTicker {
public:
	static CPUTicker* GetInstance();
	static void Start();
	static void Stop();

	void SetGenerationEnabled(bool enabled);
	void StopGeneration();
	bool IsGenerationEnabled() const;

	uint64_t GetCurrentTick() const;
	uint64_t GetIdleTickTime() const;
	uint64_t GetActiveTickTime() const;
	void WaitUntilTick(uint64_t targetTick) const;

private:
	CPUTicker();
	~CPUTicker();
	CPUTicker(const CPUTicker&) = delete;
	CPUTicker& operator=(const CPUTicker&) = delete;

	void Run();
	void OnTick();

	static CPUTicker* instance;

	std::thread tickerThread;
	std::atomic<bool> running;
	std::atomic<bool> generating;
	std::atomic<uint64_t> currentTick;

	std::atomic<uint64_t> idleTickTime;
	std::atomic<uint64_t> activeTickTime;

	mutable std::mutex tickMutex;
	mutable std::condition_variable tickCondition;
};
