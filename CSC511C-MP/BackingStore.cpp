#include "BackingStore.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {
	using PageKey = std::pair<int, uint32_t>;

	std::mutex backingStoreMutex;
	std::map<PageKey, std::vector<uint8_t>> cachedPages;

	/**
	 * @brief Returns the filesystem path to the backing store file.
	 * @returns Reference to the static path defined by BackingStore::FILE_PATH.
	 */
	const std::filesystem::path& BackingStoreFilePath() {
		static const std::filesystem::path path(BackingStore::FILE_PATH);
		return path;
	}

	/**
	 * @brief Creates the data directory if it does not already exist.
	 * @note Failures are ignored via std::error_code so paging can continue in memory only.
	 */
	void EnsureDataDirectoryExists() {
		const std::filesystem::path parent = BackingStoreFilePath().parent_path();
		if (parent.empty()) {
			return;
		}

		std::error_code error;
		std::filesystem::create_directories(parent, error);
	}

	/**
	 * @brief Encodes raw page bytes as a contiguous lowercase hex string.
	 * @param pageBytes Byte contents of one evicted frame/page.
	 * @returns Hex string with two digits per byte (e.g. "0a1bff").
	 */
	std::string BytesToHex(const std::vector<uint8_t>& pageBytes) {
		std::ostringstream builder;
		builder << std::hex << std::setfill('0');
		for (const uint8_t byte : pageBytes) {
			builder << std::setw(2) << static_cast<unsigned>(byte);
		}
		return builder.str();
	}

	/**
	 * @brief Converts one ASCII hex character to its numeric value.
	 * @param character Hex digit ('0'-'9', 'a'-'f', or 'A'-'F').
	 * @returns Nibble value in [0, 15], or -1 if the character is invalid.
	 */
	int HexDigit(char character) {
		if (character >= '0' && character <= '9') {
			return character - '0';
		}
		if (character >= 'a' && character <= 'f') {
			return character - 'a' + 10;
		}
		if (character >= 'A' && character <= 'F') {
			return character - 'A' + 10;
		}
		return -1;
	}

	/**
	 * @brief Decodes a hex string into a fixed-size byte buffer.
	 * @param hexData Hex-encoded page contents (must be exactly expectedSize * 2 chars).
	 * @param expectedSize Number of bytes to decode (frame size).
	 * @param outPageBytes Output buffer resized and filled on success.
	 * @returns True if decoding succeeds; false on length or invalid-character errors.
	 */
	bool HexToBytes(const std::string& hexData, size_t expectedSize, std::vector<uint8_t>& outPageBytes) {
		if (hexData.size() != expectedSize * 2) {
			return false;
		}

		outPageBytes.resize(expectedSize);
		for (size_t index = 0; index < expectedSize; ++index) {
			const int highNibble = HexDigit(hexData[index * 2]);
			const int lowNibble = HexDigit(hexData[index * 2 + 1]);
			if (highNibble < 0 || lowNibble < 0) {
				return false;
			}
			outPageBytes[index] = static_cast<uint8_t>((highNibble << 4) | lowNibble);
		}
		return true;
	}

	/**
	 * @brief Parses one line from csopesy-backing-store.txt.
	 * @param line Raw text line from the backing store file.
	 * @param processId Output process id read from the line.
	 * @param virtualPage Output virtual page index read from the line.
	 * @param hexData Output hex payload read from the line.
	 * @returns True for data lines; false for empty lines, comments (#), or parse failures.
	 */
	bool ParsePageLine(const std::string& line, int& processId, uint32_t& virtualPage, std::string& hexData) {
		if (line.empty() || line[0] == '#') {
			return false;
		}

		std::istringstream lineStream(line);
		return static_cast<bool>(lineStream >> processId >> virtualPage >> hexData);
	}

	/**
	 * @brief Persists every cached page to disk, replacing the backing store file.
	 * @note Writes header comments plus one line per entry:
	 *       `<process_id> <virtual_page> <hex_bytes>`.
	 */
	void WriteAllPagesToFile() {
		EnsureDataDirectoryExists();

		std::ofstream backingStoreFile(BackingStoreFilePath(), std::ios::out | std::ios::trunc);
		if (!backingStoreFile.is_open()) {
			return;
		}

		backingStoreFile << "# csopesy-backing-store.txt\n";
		backingStoreFile << "# Format: <process_id> <virtual_page> <hex_bytes>\n";
		backingStoreFile << "# Updated when pages are paged out to backing store.\n";

		for (const auto& entry : cachedPages) {
			backingStoreFile << entry.first.first << ' ' << entry.first.second << ' '
				<< BytesToHex(entry.second) << '\n';
		}
	}

	/**
	 * @brief Loads a page from the backing store file when it is not in the in-memory cache.
	 * @param processId Owner process of the requested page.
	 * @param virtualPage Virtual page number within the process address space.
	 * @param expectedFrameSize Required byte length (must match mem-per-frame).
	 * @param outPageBytes Output buffer filled with decoded page bytes on success.
	 * @returns True if the page is found and decoded; false if missing or corrupt.
	 * @note On success, the page is also inserted into cachedPages for faster later reads.
	 */
	bool ReadPageFromFile(int processId, uint32_t virtualPage, size_t expectedFrameSize, std::vector<uint8_t>& outPageBytes) {
		std::ifstream backingStoreFile(BackingStoreFilePath());
		if (!backingStoreFile.is_open()) {
			return false;
		}

		std::string line;
		while (std::getline(backingStoreFile, line)) {
			int fileProcessId = -1;
			uint32_t fileVirtualPage = 0;
			std::string hexData;
			if (!ParsePageLine(line, fileProcessId, fileVirtualPage, hexData)) {
				continue;
			}
			if (fileProcessId != processId || fileVirtualPage != virtualPage) {
				continue;
			}

			if (!HexToBytes(hexData, expectedFrameSize, outPageBytes)) {
				return false;
			}

			cachedPages[{processId, virtualPage}] = outPageBytes;
			return true;
		}

		return false;
	}

	/**
	 * @brief Attempts to read a page from the in-memory cache only.
	 * @param processId Owner process of the requested page.
	 * @param virtualPage Virtual page number within the process address space.
	 * @param expectedFrameSize Required byte length (must match cached page size).
	 * @param outPageBytes Output buffer copied from cache on success.
	 * @returns True if the page exists in cache with the expected size; false otherwise.
	 */
	bool TryReadPageFromCache(int processId, uint32_t virtualPage, size_t expectedFrameSize, std::vector<uint8_t>& outPageBytes) {
		const auto pageIt = cachedPages.find({ processId, virtualPage });
		if (pageIt == cachedPages.end() || pageIt->second.size() != expectedFrameSize) {
			return false;
		}

		outPageBytes = pageIt->second;
		return true;
	}
}

/**
 * @brief Clears all cached pages and resets the backing store file to empty.
 * @note Called when MemoryManager initializes so each simulator boot starts fresh.
 */
void BackingStore::ResetStore() {
	std::lock_guard<std::mutex> lock(backingStoreMutex);
	cachedPages.clear();
	WriteAllPagesToFile();
}

/**
 * @brief Stores one evicted physical frame into the backing store (cache + file).
 * @param processId Process that owned the evicted frame.
 * @param virtualPage Virtual page index that was mapped to the evicted frame.
 * @param pageBytes Raw frame contents written on page-out.
 * @note No-op when pageBytes is empty. Thread-safe via backingStoreMutex.
 */
void BackingStore::StoreEvictedPage(int processId, uint32_t virtualPage, const std::vector<uint8_t>& pageBytes) {
	if (pageBytes.empty()) {
		return;
	}

	std::lock_guard<std::mutex> lock(backingStoreMutex);
	cachedPages[{ processId, virtualPage }] = pageBytes;
	WriteAllPagesToFile();
}

/**
 * @brief Loads a previously stored page on page-in (page fault handling).
 * @param processId Process requesting the page.
 * @param virtualPage Virtual page index to load.
 * @param outPageBytes Output buffer filled with page bytes when found.
 * @param expectedFrameSize Frame size in bytes (mem-per-frame from config).
 * @returns True if the page was found in cache or on disk; false if never stored.
 * @note Lookup order: in-memory cache first, then csopesy-backing-store.txt.
 */
bool BackingStore::LoadStoredPage(
	int processId,
	uint32_t virtualPage,
	std::vector<uint8_t>& outPageBytes,
	size_t expectedFrameSize) {
	std::lock_guard<std::mutex> lock(backingStoreMutex);

	if (TryReadPageFromCache(processId, virtualPage, expectedFrameSize, outPageBytes)) {
		return true;
	}

	return ReadPageFromFile(processId, virtualPage, expectedFrameSize, outPageBytes);
}

/**
 * @brief Removes every backing-store entry belonging to one process.
 * @param processId Process whose evicted pages should be deleted.
 * @note Called on process exit. Updates both cache and backing store file.
 */
void BackingStore::RemovePagesForProcess(int processId) {
	std::lock_guard<std::mutex> lock(backingStoreMutex);

	for (auto pageIt = cachedPages.lower_bound({ processId, 0 });
		pageIt != cachedPages.end() && pageIt->first.first == processId; ) {
		pageIt = cachedPages.erase(pageIt);
	}

	WriteAllPagesToFile();
}
