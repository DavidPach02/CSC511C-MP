#include "SymbolTable.h"
#include "MemoryManager.h"
#include "Process.h"

#include <algorithm>

SymbolTable::SymbolTable(const std::shared_ptr<Process>& process)
	: processRef(process), lowerAddress(0), higherAddress(31) {
	for (uint16_t address = lowerAddress; address <= higherAddress; ++address) {
		table.emplace(address, DeclaredSymbol("", 0));
	}
}

uint16_t SymbolTable::GetByteAddressForSlot(uint16_t slotIndex) const {
	return static_cast<uint16_t>(slotIndex * sizeof(uint16_t));
}

bool SymbolTable::ReadSlotValue(uint16_t slotIndex, uint16_t& outValue) const {
	const std::shared_ptr<Process> process = processRef.lock();
	if (process == nullptr) {
		return false;
	}

	const MemoryAccessResult accessResult = MemoryManager::GetInstance()->ReadProcessMemory(
		process, GetByteAddressForSlot(slotIndex), outValue);
	return accessResult == MemoryAccessResult::Success;
}

bool SymbolTable::WriteSlotValue(uint16_t slotIndex, uint16_t value) {
	const std::shared_ptr<Process> process = processRef.lock();
	if (process == nullptr) {
		return false;
	}

	const MemoryAccessResult accessResult = MemoryManager::GetInstance()->WriteProcessMemory(
		process, GetByteAddressForSlot(slotIndex), value);
	return accessResult == MemoryAccessResult::Success;
}

bool SymbolTable::SetVariable(const std::string& name, int value) {
	const uint16_t clampedValue = static_cast<uint16_t>(std::clamp(value, 0, 65535));
	DeclaredSymbol* symbol = FindVariable(name);

	if (symbol != nullptr) {
		symbol->value = clampedValue;
		for (auto& [slotIndex, entry] : table) {
			if (&entry == symbol) {
				if (!WriteSlotValue(slotIndex, clampedValue)) {
					return false;
				}
				break;
			}
		}
		return true;
	}

	if (IsFull()) {
		return true;
	}

	const uint16_t freeSlotIndex = GetFreeMemory();
	if (freeSlotIndex == 0xFFFF) {
		return false;
	}

	table[freeSlotIndex] = DeclaredSymbol(name, clampedValue);
	return WriteSlotValue(freeSlotIndex, clampedValue);
}

bool SymbolTable::SetVariable(const uint16_t memoryAddress, const std::string& name, uint16_t value) {
	auto slotIt = table.find(memoryAddress);
	if (slotIt == table.end()) {
		return false;
	}

	slotIt->second.name = name;
	slotIt->second.value = value;
	return WriteSlotValue(memoryAddress, value);
}

int SymbolTable::GetVariable(const std::string& name) {
	for (auto& [slotIndex, entry] : table) {
		if (entry.name == name) {
			uint16_t storedValue = 0;
			if (ReadSlotValue(slotIndex, storedValue)) {
				entry.value = storedValue;
				return storedValue;
			}
			return entry.value;
		}
	}
	return 0;
}

int SymbolTable::GetVariable(const uint16_t memoryAddress) {
	if (memoryAddress < lowerAddress || memoryAddress > higherAddress) {
		return 0;
	}

	uint16_t storedValue = 0;
	if (ReadSlotValue(memoryAddress, storedValue)) {
		return storedValue;
	}
	return 0;
}

bool SymbolTable::HasVariable(const std::string& name) {
	return FindVariable(name) != nullptr;
}

bool SymbolTable::IsFull() const {
	for (const auto& [address, symbol] : table) {
		if (symbol.name.empty()) {
			return false;
		}
	}
	return true;
}

std::string SymbolTable::GetTableLogs() const {
	std::string logs;
	for (const auto& pair : table) {
		logs += "  [ \033[33m" + std::to_string(pair.first) + "\033[0m ] "
			+ pair.second.name + ": " + std::to_string(pair.second.value) + "\n";
	}
	return logs;
}

DeclaredSymbol* SymbolTable::FindVariable(const std::string& name) {
	for (auto& [address, symbol] : table) {
		if (symbol.name == name) {
			return &symbol;
		}
	}
	return nullptr;
}

uint16_t SymbolTable::GetFreeMemory() const {
	for (const auto& [address, symbol] : table) {
		if (symbol.name.empty()) {
			return address;
		}
	}
	return 0xFFFF;
}
