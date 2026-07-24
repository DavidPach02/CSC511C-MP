#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

// MO2 swap space: evicted pages live in RAM (cache) and csopesy-backing-store.txt.
// Format per line: <process_id> <virtual_page> <hex_bytes>
class BackingStore {
public:
	static constexpr const char* FILE_PATH = "../data/csopesy-backing-store.txt";

	static void ResetStore();
	static void StoreEvictedPage(int processId, uint32_t virtualPage, const std::vector<uint8_t>& pageBytes);
	static bool LoadStoredPage(
		int processId,
		uint32_t virtualPage,
		std::vector<uint8_t>& outPageBytes,
		size_t frameSizeBytes);
	static void RemovePagesForProcess(int processId);
};
