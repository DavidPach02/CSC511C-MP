#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>
#include <iostream>

class SymbolTable {
public:
	SymbolTable();
	~SymbolTable() = default;
	void SetVariable(const std::string& name, int value);
	int GetVariable(const std::string& name);
	bool HasVariable(const std::string& name);

	std::string GetTableLogs() const;
private:
	std::unordered_map<std::string, int> table;
};

