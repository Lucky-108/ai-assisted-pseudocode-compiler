
#include <iostream>
#include "ast.hpp"
#include "tac_generator.hpp"

int main() {
    // Represents: x = 10 + 20
    AssignStmt assignment(
        "x",
        new BinaryExpr(
            BinaryOperator::ADD,
            new NumberExpr(10, DataType::INT),
            new NumberExpr(20, DataType::INT)
        )
    );

    TACGenerator generator;
    generator.generateStatement(assignment);

    for (const auto& instruction : generator.getProgram().instructions) {
        std::cout << instruction.result << " = ";

        if (instruction.op == TACOp::ADD) {
            std::cout << instruction.arg1 << " + " << instruction.arg2;
        } else if (instruction.op == TACOp::ASSIGN) {
            std::cout << instruction.arg1;
        }

        std::cout << '\n';
    }
}
