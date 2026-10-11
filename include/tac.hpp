
#ifndef TAC_HPP
#define TAC_HPP

#include <string>
#include <vector>
#include "symbol_table.hpp"

enum class TACOp {
    ASSIGN, ADD, SUBTRACT, MULTIPLY, DIVIDE,
    COMPARE_LT, COMPARE_GT, COMPARE_LE, COMPARE_GE,
    COMPARE_EQ, COMPARE_NE,
    LABEL, JUMP, JUMP_IF_FALSE, PRINT
};

struct TACInstruction {
    TACOp op;
    std::string result;
    std::string arg1;
    std::string arg2;
    DataType resultType = DataType::INT;
};

class TACProgram {
public:
    std::vector<TACInstruction> instructions;

    void emit(
        TACOp op,
        const std::string& result = "",
        const std::string& arg1 = "",
        const std::string& arg2 = "",
        DataType resultType = DataType::INT
    ) {
        instructions.push_back(
            {op, result, arg1, arg2, resultType}
        );
    }
};

#endif
