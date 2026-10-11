
#include <iostream>
#include "ast.hpp"
#include "tac_generator.hpp"

int main() {
    // Represents:
    // IF (5 > 2) {
    //     PRINT 1;
    // } ELSE {
    //     PRINT 0;
    // }

    std::vector<std::unique_ptr<Stmt>> thenBranch;
    thenBranch.push_back(
        std::make_unique<PrintStmt>(
            new NumberExpr(1, DataType::INT)
        )
    );

    std::vector<std::unique_ptr<Stmt>> elseBranch;
    elseBranch.push_back(
        std::make_unique<PrintStmt>(
            new NumberExpr(0, DataType::INT)
        )
    );

    IfStmt statement(
        new BinaryExpr(
            BinaryOperator::GREATER,
            new NumberExpr(5, DataType::INT),
            new NumberExpr(2, DataType::INT)
        ),
        std::move(thenBranch),
        std::move(elseBranch)
    );

    TACGenerator generator;
    generator.generateStatement(statement);

    for (const auto& instruction : generator.getProgram().instructions) {
        switch (instruction.op) {
            case TACOp::COMPARE_GT:
                std::cout << instruction.result << " = "
                          << instruction.arg1 << " > "
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
