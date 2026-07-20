#include "SymbolTable.h"

#include <algorithm>

SymbolTable::SymbolTable(void* baseMemoryAddress, uint16_t lowerAddress, uint16_t allocatedMemory)
	: baseMemoryAddress(baseMemoryAddress), lowerAddress(lowerAddress){
	this->higherAddress = lowerAddress + allocatedMemory - 1;

	for (uint16_t address = this->lowerAddress; address <= this->higherAddress; address++) {
		table.emplace(address, DeclaredSymbol("", 0));
	}
	//std::cout << GetTableLogs();
}

bool SymbolTable::SetVariable(const std::string& name, int value) {
	const uint16_t clampedValue = static_cast<uint16_t>(std::clamp(value, 0, 65535));
	DeclaredSymbol* symbol = FindVariable(name);
	//std::cout << "Finding variable" << std::endl;

	// If we found an existing symbol
	if (symbol) {
		//std::cout << name << " found in symbol table." << std::endl;
		// Just set the value
		symbol->value = clampedValue;

		// Find the virtual address of this symbol to write it physically
		for (auto& [address, sym] : this->table) {
			if (&sym == symbol) {
				void* physicalDest = GetPhysicalAddress(address);
				if (physicalDest) {
					*static_cast<uint16_t*>(physicalDest) = clampedValue;
				}
				break;
			}
		}
		return true;
	}
	else {
		// If the symbol table is full, ignore setting
		if (IsFull()) {
			return true;
		}

		//std::cout << name << " not found. Finding a free memory space." << std::endl;
		// If we don't find an existing symbol, find a free memory
		uint16_t freeMemoryAddress = GetFreeMemory();
		// If there's free memory
		if (freeMemoryAddress != 0xFFFF) {
			this->table[freeMemoryAddress] = DeclaredSymbol(name, clampedValue);

			// Write the value to physical host RAM
			void* physicalDest = GetPhysicalAddress(freeMemoryAddress);
			if (physicalDest) {
				*static_cast<uint16_t*>(physicalDest) = clampedValue;
			}
			return true;
		}
	}

	// If we can't set it, then a memory access violation happened.
	return false;
}

bool SymbolTable::SetVariable(const uint16_t memoryAddress, const std::string& name, uint16_t value) {
	auto it = this->table.find(memoryAddress);

	// Found the memory address within the symbol table
	if (it != table.end()) {
		DeclaredSymbol& symbol = it->second;
		symbol.name = name;
		symbol.value = value;

		// Write the value to physical host RAM
		void* physicalDest = GetPhysicalAddress(memoryAddress);
		if (physicalDest) {
			*static_cast<uint16_t*>(physicalDest) = value;
		}

		return true;
	}

	// Memory out of bounds --> memory access violation
	return false;
}

int SymbolTable::GetVariable(const std::string& name) {
	DeclaredSymbol* symbol = FindVariable(name);

	if (symbol != nullptr) {
		return symbol->value;
	}
	return 0;
}

int SymbolTable::GetVariable(const uint16_t memoryAddress) {
	if (memoryAddress >= lowerAddress && memoryAddress <= higherAddress) {
		void* physicalSrc = GetPhysicalAddress(memoryAddress);
		if (physicalSrc != nullptr) {
			// Read the 2-byte value directly from memory
			return *static_cast<uint16_t*>(physicalSrc);
		}
	}
	return 0;
}

bool SymbolTable::HasVariable(const std::string& name) {
	return FindVariable(name);
}

bool SymbolTable::IsFull() const {
	size_t activeVariables = 0;
	for (const auto& [address, symbol] : this->table) {
		if (symbol.name == "") return false;
	}
	return true;
}

std::string SymbolTable::GetTableLogs() const {
	std::string logs = "";

	for (const auto& pair : table) {
		logs += "  [ \033[33m" + std::to_string(pair.first) + "\033[0m ] " + pair.second.name + ": " + std::to_string(pair.second.value) + "\n";
	}
	return logs;
}

DeclaredSymbol* SymbolTable::FindVariable(const std::string& name) {
	for (auto& [address, symbol] : this->table) {
		if (symbol.name == name) {
			return &symbol; // Return a pointer to the symbol in the map
		}
	}
	return nullptr; // Return nullptr if not found
}

void* SymbolTable::GetPhysicalAddress(uint16_t virtualAddress) const {
	if (virtualAddress < lowerAddress || virtualAddress > higherAddress) {
		return nullptr; // Out of bounds safety check
	}

	if (baseMemoryAddress == nullptr) {
		return nullptr;
	}

	// Calculate byte offset: (virtual address - base virtual offset) * 2 bytes per variable
	uint16_t offset = (virtualAddress - lowerAddress) * sizeof(uint16_t);

	// Cast void* to char* to perform byte-wise pointer arithmetic, then offset it
	return static_cast<char*>(this->baseMemoryAddress) + offset;
}

uint16_t SymbolTable::GetFreeMemory() const {
	for (const auto& [address, symbol] : this->table) {
		if (symbol.name == "") {
			return address;
		}
	}

	return 0xFFFF;
}
