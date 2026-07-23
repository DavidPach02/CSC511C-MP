#pragma once

#include <cstdint>
#include <vector>

// Secondary storage for demand paging. When a physical frame is evicted, its bytes are
// saved here (in-memory cache + csopesy-backing-store.txt) and loaded back on page fault.
class BackingStore {
public:
	static constexpr const char* FILE_PATH = "../data/csopesy-backing-store.txt";

	// Wipes the cache and truncates the backing store file. Used on MemoryManager boot.
	static void ResetStore();

	// Page-out: save an evicted frame keyed by (processId, virtualPage).
	static void StoreEvictedPage(int processId, uint32_t virtualPage, const std::vector<uint8_t>& pageBytes);

	// Page-in: load a stored page into outPageBytes (cache first, then disk file).
	static bool LoadStoredPage(
		int processId,
		uint32_t virtualPage,
		std::vector<uint8_t>& outPageBytes,
		size_t expectedFrameSize);

	// Process exit: remove every stored page belonging to processId.
	static void RemovePagesForProcess(int processId);
};
