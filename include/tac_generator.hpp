
#ifndef TAC_GENERATOR_HPP
#define TAC_GENERATOR_HPP

#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "ast.hpp"
#include "tac.hpp"

class TACGenerator {
private:
    TACProgram program;
    int tempCounter = 0;
    int labelCounter = 0;

    std::unordered_map<std::string, DataType> variableTypes;

    std::string newTemp() {
        return "t" + std::to_string(++tempCounter);
    }

    std::string newLabel() {
        return "L" + std::to_string(++labelCounter);
    }

    std::string numberToString(double value, DataType type) {
        std::ostringstream output;

        if (type == DataType::INT) {
            output << static_cast<long long>(value);
        } else {
            output << value;
            if (output.str().find('.') == std::string::npos &&
                output.str().find('e') == std::string::npos &&
                output.str().find('E') == std::string::npos) {
                output << ".0";
            }
        }

        return output.str();
    }

    void collectVariableTypes(
        const std::vector<std::unique_ptr<Stmt>>& statements
    ) {
        for (const auto& statement : statements) {
            if (const auto* declaration =
                    dynamic_cast<const DeclareStmt*>(statement.get())) {
                variableTypes[declaration->name] = declaration->type;
            } else if (const auto* conditional =
                           dynamic_cast<const IfStmt*>(statement.get())) {
                collectVariableTypes(conditional->thenBranch);
                collectVariableTypes(conditional->elseBranch);
            } else if (const auto* loop =
                           dynamic_cast<const WhileStmt*>(statement.get())) {
                collectVariableTypes(loop->body);
            }
        }
    }

    DataType inferType(const Expr& expr) const {
        if (const auto* number = dynamic_cast<const NumberExpr*>(&expr)) {
            return number->type;
        }

        if (const auto* variable =
                dynamic_cast<const VariableExpr*>(&expr)) {
            auto it = variableTypes.find(variable->name);

            if (it == variableTypes.end()) {
                throw std::runtime_error(
                    "Unknown variable type: " + variable->name
                );
            }

            return it->second;
        }

        if (const auto* unary = dynamic_cast<const UnaryExpr*>(&expr)) {
            return inferType(*unary->operand);
        }

        if (const auto* binary = dynamic_cast<const BinaryExpr*>(&expr)) {
            switch (binary->op) {
                case BinaryOperator::LESS:
                case BinaryOperator::GREATER:
                case BinaryOperator::LESS_EQUAL:
                case BinaryOperator::GREATER_EQUAL:
                case BinaryOperator::EQUAL:
                case BinaryOperator::NOT_EQUAL:
                    return DataType::BOOLEAN;

                default:
                    break;
            }

            DataType leftType = inferType(*binary->left);
            DataType rightType = inferType(*binary->right);

            if (leftType == DataType::FLOAT ||
                rightType == DataType::FLOAT) {
                return DataType::FLOAT;
            }

            return DataType::INT;
        }

        throw std::runtime_error("Cannot infer expression type");
    }

    void generateBlock(
        const std::vector<std::unique_ptr<Stmt>>& statements
    ) {
        for (const auto& statement : statements) {
            generateStatement(*statement);
        }
    }

public:
    std::string generateExpression(const Expr& expr) {
        if (const auto* number =
                dynamic_cast<const NumberExpr*>(&expr)) {
            return numberToString(number->value, number->type);
        }

        if (const auto* variable =
                dynamic_cast<const VariableExpr*>(&expr)) {
            return variable->name;
        }

        if (const auto* unary =
                dynamic_cast<const UnaryExpr*>(&expr)) {
            std::string operand = generateExpression(*unary->operand);
            std::string result = newTemp();

            program.emit(
                TACOp::SUBTRACT,
                result,
                "0",
                operand,
                inferType(expr)
            );

            return result;
        }

        if (const auto* binary =
                dynamic_cast<const BinaryExpr*>(&expr)) {
            std::string left = generateExpression(*binary->left);
            std::string right = generateExpression(*binary->right);
            std::string result = newTemp();

            TACOp operation = TACOp::ADD;

            switch (binary->op) {
                case BinaryOperator::ADD:
                    operation = TACOp::ADD;
                    break;
                case BinaryOperator::SUBTRACT:
                    operation = TACOp::SUBTRACT;
                    break;
                case BinaryOperator::MULTIPLY:
                    operation = TACOp::MULTIPLY;
                    break;
                case BinaryOperator::DIVIDE:
                    operation = TACOp::DIVIDE;
                    break;
                case BinaryOperator::LESS:
                    operation = TACOp::COMPARE_LT;
                    break;
                case BinaryOperator::GREATER:
                    operation = TACOp::COMPARE_GT;
                    break;
                case BinaryOperator::LESS_EQUAL:
                    operation = TACOp::COMPARE_LE;
                    break;
                case BinaryOperator::GREATER_EQUAL:
                    operation = TACOp::COMPARE_GE;
                    break;
                case BinaryOperator::EQUAL:
                    operation = TACOp::COMPARE_EQ;
                    break;
                case BinaryOperator::NOT_EQUAL:
                    operation = TACOp::COMPARE_NE;
                    break;
            }

            program.emit(
                operation,
                result,
                left,
                right,
                inferType(expr)
            );

            return result;
        }

        throw std::runtime_error(
            "Unsupported expression in TAC generator"
        );
    }

    void generateStatement(const Stmt& stmt) {
        if (dynamic_cast<const DeclareStmt*>(&stmt)) {
            return;
        }

        if (const auto* assignment =
                dynamic_cast<const AssignStmt*>(&stmt)) {
            std::string value =
                generateExpression(*assignment->expression);

            auto it = variableTypes.find(assignment->name);

            if (it == variableTypes.end()) {
                throw std::runtime_error(
                    "Unknown variable type: " + assignment->name
                );
            }

            program.emit(
                TACOp::ASSIGN,
                assignment->name,
                value,
                "",
                it->second
            );

            return;
        }

        if (const auto* print =
                dynamic_cast<const PrintStmt*>(&stmt)) {
            std::string value =
                generateExpression(*print->expression);

            program.emit(
                TACOp::PRINT,
                "",
                value,
                "",
                inferType(*print->expression)
            );

            return;
        }

        if (const auto* conditional =
                dynamic_cast<const IfStmt*>(&stmt)) {
            std::string condition =
                generateExpression(*conditional->condition);

            std::string elseLabel = newLabel();
            std::string endLabel = newLabel();

            program.emit(
                TACOp::JUMP_IF_FALSE, elseLabel, condition
            );

            generateBlock(conditional->thenBranch);
            program.emit(TACOp::JUMP, endLabel);
            program.emit(TACOp::LABEL, elseLabel);
            generateBlock(conditional->elseBranch);
            program.emit(TACOp::LABEL, endLabel);

            return;
        }

        if (const auto* loop =
                dynamic_cast<const WhileStmt*>(&stmt)) {
            std::string startLabel = newLabel();
            std::string endLabel = newLabel();

            program.emit(TACOp::LABEL, startLabel);

            std::string condition =
                generateExpression(*loop->condition);

            program.emit(
                TACOp::JUMP_IF_FALSE, endLabel, condition
            );

            generateBlock(loop->body);
            program.emit(TACOp::JUMP, startLabel);
            program.emit(TACOp::LABEL, endLabel);

            return;
        }

        throw std::runtime_error(
            "Unsupported statement in TAC generator"
        );
    }

    void generateProgram(const Program& sourceProgram) {
        program = TACProgram{};
        tempCounter = 0;
        labelCounter = 0;
        variableTypes.clear();

        collectVariableTypes(sourceProgram.statements);

        for (const auto& statement : sourceProgram.statements) {
            generateStatement(*statement);
        }
    }

    const TACProgram& getProgram() const {
        return program;
    }

    void printTAC() const {
        for (const auto& instruction : program.instructions) {
            switch (instruction.op) {
                case TACOp::ASSIGN:
                    std::cout << instruction.result << " = "
                              << instruction.arg1 << '\n';
                    break;
                case TACOp::ADD:
                    std::cout << instruction.result << " = "
                              << instruction.arg1 << " + "
                              << instruction.arg2 << '\n';
                    break;
                case TACOp::SUBTRACT:
                    std::cout << instruction.result << " = "
                              << instruction.arg1 << " - "
                              << instruction.arg2 << '\n';
                    break;
                case TACOp::MULTIPLY:
                    std::cout << instruction.result << " = "
                              << instruction.arg1 << " * "
                              << instruction.arg2 << '\n';
                    break;
                case TACOp::DIVIDE:
                    std::cout << instruction.result << " = "
                              << instruction.arg1 << " / "
                              << instruction.arg2 << '\n';
                    break;
                case TACOp::COMPARE_LT:
                    std::cout << instruction.result << " = "
                              << instruction.arg1 << " < "
                              << instruction.arg2 << '\n';
                    break;
                case TACOp::COMPARE_GT:
                    std::cout << instruction.result << " = "
                              << instruction.arg1 << " > "
                              << instruction.arg2 << '\n';
                    break;
                case TACOp::COMPARE_LE:
                    std::cout << instruction.result << " = "
                              << instruction.arg1 << " <= "
                              << instruction.arg2 << '\n';
                    break;
                case TACOp::COMPARE_GE:
                    std::cout << instruction.result << " = "
                              << instruction.arg1 << " >= "
                              << instruction.arg2 << '\n';
                    break;
                case TACOp::COMPARE_EQ:
                    std::cout << instruction.result << " = "
                              << instruction.arg1 << " == "
                              << instruction.arg2 << '\n';
                    break;
                case TACOp::COMPARE_NE:
                    std::cout << instruction.result << " = "
                              << instruction.arg1 << " != "
                              << instruction.arg2 << '\n';
                    break;
                case TACOp::PRINT:
                    std::cout << "PRINT " << instruction.arg1 << '\n';
                    break;
                case TACOp::LABEL:
                    std::cout << instruction.result << ":\n";
                    break;
                case TACOp::JUMP:
                    std::cout << "GOTO " << instruction.result << '\n';
                    break;
                case TACOp::JUMP_IF_FALSE:
                    std::cout << "IF_FALSE " << instruction.arg1
                              << " GOTO " << instruction.result << '\n';
                    break;
            }
        }
    }
};

#endif
