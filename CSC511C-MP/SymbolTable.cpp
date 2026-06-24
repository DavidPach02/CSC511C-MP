#include "SymbolTable.h"

SymbolTable::SymbolTable() {
}

void SymbolTable::SetVariable(const std::string& name, int value) {
	if (HasVariable(name)) {
		table[name] = value;
	}
	else {
		table.emplace(name, value);
	}
}

int SymbolTable::GetVariable(const std::string& name) {
	if (this->HasVariable(name)) {
		return table[name];
	}
	return 0;
}

bool SymbolTable::HasVariable(const std::string& name) {
	return table.find(name) != table.end();
}

std::string SymbolTable::GetTableLogs() const {
	std::string logs = "";

	for (const auto& pair : table) {
		logs += "  [ \033[33m" + pair.first + "\033[0m ] " + std::to_string(pair.second) + "\n";
	}
	return logs;
}
