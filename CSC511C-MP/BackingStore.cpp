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

	struct BackingStoreState {
		std::mutex mutex;
		std::map<PageKey, std::vector<uint8_t>> pages;

		const std::filesystem::path& FilePath() const {
			static const std::filesystem::path path(BackingStore::FILE_PATH);
			return path;
		}

		static int HexValue(char character) {
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

		std::string ToHex(const std::vector<uint8_t>& pageBytes) const {
			std::ostringstream builder;
			builder << std::hex << std::setfill('0');
			for (const uint8_t byte : pageBytes) {
				builder << std::setw(2) << static_cast<unsigned>(byte);
			}
			return builder.str();
		}

		bool FromHex(const std::string& hexData, size_t frameSizeBytes, std::vector<uint8_t>& outPageBytes) const {
			if (hexData.size() != frameSizeBytes * 2) {
				return false;
			}

			outPageBytes.resize(frameSizeBytes);
			for (size_t index = 0; index < frameSizeBytes; ++index) {
				const int highNibble = HexValue(hexData[index * 2]);
				const int lowNibble = HexValue(hexData[index * 2 + 1]);
				if (highNibble < 0 || lowNibble < 0) {
					return false;
				}
				outPageBytes[index] = static_cast<uint8_t>((highNibble << 4) | lowNibble);
			}
			return true;
		}

		void WriteFile() const {
			const std::filesystem::path parent = FilePath().parent_path();
			if (!parent.empty()) {
				std::error_code error;
				std::filesystem::create_directories(parent, error);
			}

			std::ofstream file(FilePath(), std::ios::out | std::ios::trunc);
			if (!file.is_open()) {
				return;
			}

			file << "# csopesy-backing-store.txt\n";
			file << "# Format: <process_id> <virtual_page> <hex_bytes>\n";
			for (const auto& entry : pages) {
				file << entry.first.first << ' ' << entry.first.second << ' '
					<< ToHex(entry.second) << '\n';
			}
		}

		bool ReadPageFromFile(int processId, uint32_t virtualPage, size_t frameSizeBytes, std::vector<uint8_t>& outPageBytes) const {
			std::ifstream file(FilePath());
			if (!file.is_open()) {
				return false;
			}

			std::string line;
			while (std::getline(file, line)) {
				if (line.empty() || line[0] == '#') {
					continue;
				}

				int fileProcessId = -1;
				uint32_t fileVirtualPage = 0;
				std::string hexData;
				std::istringstream lineStream(line);
				if (!(lineStream >> fileProcessId >> fileVirtualPage >> hexData)) {
					continue;
				}
				if (fileProcessId != processId || fileVirtualPage != virtualPage) {
					continue;
				}

				return FromHex(hexData, frameSizeBytes, outPageBytes);
			}

			return false;
		}
	};

	BackingStoreState& GetStore() {
		static BackingStoreState store;
		return store;
	}
}

void BackingStore::ResetStore() {
	BackingStoreState& store = GetStore();
	std::lock_guard<std::mutex> lock(store.mutex);
	store.pages.clear();
	store.WriteFile();
}

void BackingStore::StoreEvictedPage(int processId, uint32_t virtualPage, const std::vector<uint8_t>& pageBytes) {
	if (pageBytes.empty()) {
		return;
	}

	BackingStoreState& store = GetStore();
	std::lock_guard<std::mutex> lock(store.mutex);
	store.pages[{ processId, virtualPage }] = pageBytes;
	store.WriteFile();
}

bool BackingStore::LoadStoredPage(
	int processId,
	uint32_t virtualPage,
	std::vector<uint8_t>& outPageBytes,
	size_t frameSizeBytes) {
	BackingStoreState& store = GetStore();
	std::lock_guard<std::mutex> lock(store.mutex);

	const PageKey pageKey{ processId, virtualPage };
	const auto cachedPage = store.pages.find(pageKey);
	if (cachedPage != store.pages.end() && cachedPage->second.size() == frameSizeBytes) {
		outPageBytes = cachedPage->second;
		return true;
	}

	if (!store.ReadPageFromFile(processId, virtualPage, frameSizeBytes, outPageBytes)) {
		return false;
	}

	store.pages[pageKey] = outPageBytes;
	return true;
}

void BackingStore::RemovePagesForProcess(int processId) {
	BackingStoreState& store = GetStore();
	std::lock_guard<std::mutex> lock(store.mutex);

	for (auto pageIt = store.pages.lower_bound({ processId, 0 });
		pageIt != store.pages.end() && pageIt->first.first == processId; ) {
		pageIt = store.pages.erase(pageIt);
	}

	store.WriteFile();
}
