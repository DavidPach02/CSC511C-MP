#include "SystemState.h"

bool SystemState::initialized = false;
AppConfig SystemState::activeConfig = AppConfig::FromConfigFile(CONFIG_FILE_PATH);

bool SystemState::IsInitialized() {
	return initialized;
}

const AppConfig& SystemState::GetConfig() {
	return activeConfig;
}

void SystemState::Initialize(const AppConfig& config) {
	activeConfig = config;
	initialized = true;
}

void SystemState::Shutdown() {
	initialized = false;
}
