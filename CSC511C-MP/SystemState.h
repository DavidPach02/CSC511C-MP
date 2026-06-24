#pragma once

#include "AppConfig.h"
#include <string>

// Tracks whether initialize ran and holds the active config from config.txt.
class SystemState {
public:
	static constexpr const char* CONFIG_FILE_PATH = "../data/config.txt";

	static bool IsInitialized();
	static const AppConfig& GetConfig();
	static void Initialize(const AppConfig& config);
	static void Shutdown();

private:
	static bool initialized;
	static AppConfig activeConfig;
};
