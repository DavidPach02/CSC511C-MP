#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

class Process;

struct DeclaredSymbol {
	DeclaredSymbol() = default;
	DeclaredSymbol(std::string name, uint16_t value) : name(std::move(name)), value(value) { }
	std::string name;
	uint16_t value;
};

// Symbol table metadata with values stored in the process symbol segment (page 0) via MemoryManager.
class SymbolTable {
public:
	explicit SymbolTable(const std::shared_ptr<Process>& process);
	~SymbolTable() = default;

	bool SetVariable(const std::string& name, int value);
	bool SetVariable(uint16_t memoryAddress, const std::string& name, uint16_t value);
	int GetVariable(const std::string& name);
	int GetVariable(uint16_t memoryAddress);
	bool HasVariable(const std::string& name);
	bool IsFull() const;
	std::string GetTableLogs() const;

protected:
	DeclaredSymbol* FindVariable(const std::string& name);
	uint16_t GetFreeMemory() const;
	uint16_t GetByteAddressForSlot(uint16_t slotIndex) const;
	bool ReadSlotValue(uint16_t slotIndex, uint16_t& outValue) const;
	bool WriteSlotValue(uint16_t slotIndex, uint16_t value);

private:
	std::weak_ptr<Process> processRef;
	std::map<uint16_t, DeclaredSymbol> table;
	uint16_t lowerAddress;
	uint16_t higherAddress;
};
