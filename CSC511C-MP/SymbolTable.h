#pragma once

#include <string>
#include <unordered_map>
#include <map>
#include <cstdint>
#include <iostream>

struct DeclaredSymbol {
	DeclaredSymbol() { }
	DeclaredSymbol(std::string name, uint16_t value) : name(std::move(name)), value(value) { }
	std::string name;
	uint16_t value;
};

class SymbolTable {
public:
	SymbolTable(void* baseMemoryAddress, uint16_t lowerAddress, uint16_t allocatedMemory);
	~SymbolTable() = default;
	bool SetVariable(const std::string& name, int value);
	bool SetVariable(const uint16_t memoryAddress, const std::string& name, uint16_t value);
	int GetVariable(const std::string& name);
	int GetVariable(const uint16_t memoryAddress);
	// TODO: Remove this if unused
	bool HasVariable(const std::string& name);

	bool IsFull() const;

	std::string GetTableLogs() const;

protected:
	DeclaredSymbol* FindVariable(const std::string& name);
	void* GetPhysicalAddress(uint16_t virtualAddress) const;
	uint16_t GetFreeMemory() const;

private:
	//std::unordered_map<std::string, int> table;
	std::map<uint16_t, DeclaredSymbol> table;

	void* baseMemoryAddress;
	uint16_t lowerAddress;
	uint16_t higherAddress;
};

