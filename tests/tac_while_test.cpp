
#include <iostream>
#include "ast.hpp"
#include "tac_generator.hpp"

int main() {
    // Represents:
    // WHILE (x <= 3) {
    //     PRINT x;
    // }

    std::vector<std::unique_ptr<Stmt>> body;
    body.push_back(
        std::make_unique<PrintStmt>(
            new VariableExpr("x")
        )
    );

    WhileStmt statement(
        new BinaryExpr(
            BinaryOperator::LESS_EQUAL,
            new VariableExpr("x"),
            new NumberExpr(3, DataType::INT)
        ),
        std::move(body)
    );

    TACGenerator generator;
    generator.generateStatement(statement);

    for (const auto& instruction : generator.getProgram().instructions) {
        switch (instruction.op) {
            case TACOp::COMPARE_LE:
                std::cout << instruction.result << " = "
                          << instruction.arg1 << " <= "
                          << instruction.arg2 << '\n';
                break;

            case TACOp::JUMP_IF_FALSE:
                std::cout << "IF_FALSE " << instruction.arg1
                          << " GOTO " << instruction.result << '\n';
                break;

            case TACOp::JUMP:
                std::cout << "GOTO " << instruction.result << '\n';
                break;

            case TACOp::LABEL:
                std::cout << instruction.result << ":\n";
                break;

            case TACOp::PRINT:
                std::cout << "PRINT " << instruction.arg1 << '\n';
                break;

            default:
                break;
        }
    }
}
