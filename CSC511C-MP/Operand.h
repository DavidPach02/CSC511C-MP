#pragma once
#include <string>
#include <cctype>
#include <memory>
#include "Process.h"

struct Operand {
    std::string rawValue;

    // Checks if the string is entirely digits
    bool IsLiteral() const {
        if (rawValue.empty()) return false;
        for (char const& c : rawValue) {
            if (!std::isdigit(c)) return false;
        }
        return true;
    }

    // Resolves the actual uint16_t value at runtime
    uint16_t Resolve(std::shared_ptr<Process> process) const {
        if (IsLiteral()) {
            return static_cast<uint16_t>(std::stoul(rawValue));
        }
        // If it's not a digit literal, look it up in the symbol table!
        return process->GetSymbolTable()->GetVariable(rawValue);
    }
};
