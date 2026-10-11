
#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <stdexcept>
#include <string>
#include <unordered_map>

enum class DataType {
    INT,
    FLOAT,
    BOOLEAN
};

struct VariableInfo {
    DataType type;
    double value;
};

class SymbolTable {
private:
    std::unordered_map<std::string, VariableInfo> variables;

public:
    void declareVariable(const std::string& name, DataType type) {
        auto result = variables.emplace(
            name,
            VariableInfo{type, 0.0}
        );

        if (!result.second) {
            throw std::runtime_error(
                "Variable already declared: " + name
            );
        }
    }

    void setValue(const std::string& name, double value) {
        auto it = variables.find(name);

        if (it == variables.end()) {
            throw std::runtime_error(
                "Undeclared variable: " + name
            );
        }

        it->second.value = value;
    }

    double getValue(const std::string& name) const {
        auto it = variables.find(name);

        if (it == variables.end()) {
            throw std::runtime_error(
                "Undeclared variable: " + name
            );
        }

        return it->second.value;
    }

    DataType getType(const std::string& name) const {
        auto it = variables.find(name);

        if (it == variables.end()) {
            throw std::runtime_error(
                "Undeclared variable: " + name
            );
        }

        return it->second.type;
    }

    bool isDeclared(const std::string& name) const {
        return variables.find(name) != variables.end();
    }
};

#endif
