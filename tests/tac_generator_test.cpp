
#include <iostream>
#include "ast.hpp"
#include "tac_generator.hpp"

int main() {
    // Represents the expression: (10 + 20) * 3
    auto expression = std::make_unique<BinaryExpr>(
        BinaryOperator::MULTIPLY,
        new BinaryExpr(
            BinaryOperator::ADD,
            new NumberExpr(10, DataType::INT),
            new NumberExpr(20, DataType::INT)
        ),
        new NumberExpr(3, DataType::INT)
    );

    TACGenerator generator;
    std::string result = generator.generateExpression(*expression);

    for (const auto& instruction : generator.getProgram().instructions) {
        std::cout << instruction.result << " = ";

        switch (instruction.op) {
            case TACOp::ADD:      std::cout << instruction.arg1 << " + " << instruction.arg2; break;
            case TACOp::MULTIPLY: std::cout << instruction.arg1 << " * " << instruction.arg2; break;
            default:              std::cout << "other operation"; break;
        }

        std::cout << '\n';
    }

    std::cout << "Final result stored in: " << result << '\n';
}
