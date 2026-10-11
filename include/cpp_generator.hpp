
#ifndef CPP_GENERATOR_HPP
#define CPP_GENERATOR_HPP

#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "ast.hpp"
#include "tac.hpp"

class CPPGenerator {
private:
    std::string cppType(DataType type) const {
        switch (type) {
            case DataType::INT: return "int";
            case DataType::FLOAT: return "double";
            case DataType::BOOLEAN: return "bool";
        }

        throw std::runtime_error("Unknown data type");
    }

    void collectDeclarations(
        const std::vector<std::unique_ptr<Stmt>>& statements,
        std::ostringstream& declarations,
        std::unordered_set<std::string>& declared
    ) const {
        for (const auto& statement : statements) {
            if (const auto* declaration =
                    dynamic_cast<const DeclareStmt*>(statement.get())) {
                if (declared.insert(declaration->name).second) {
                    declarations << "    "
                                 << cppType(declaration->type) << " "
                                 << declaration->name << "{};\n";
                }
            } else if (const auto* conditional =
                           dynamic_cast<const IfStmt*>(statement.get())) {
                collectDeclarations(
                    conditional->thenBranch, declarations, declared
                );
                collectDeclarations(
                    conditional->elseBranch, declarations, declared
                );
            } else if (const auto* loop =
                           dynamic_cast<const WhileStmt*>(statement.get())) {
                collectDeclarations(
                    loop->body, declarations, declared
                );
            }
        }
    }

    bool isComputedResult(TACOp op) const {
        switch (op) {
            case TACOp::ADD:
            case TACOp::SUBTRACT:
            case TACOp::MULTIPLY:
            case TACOp::DIVIDE:
            case TACOp::COMPARE_LT:
            case TACOp::COMPARE_GT:
            case TACOp::COMPARE_LE:
            case TACOp::COMPARE_GE:
            case TACOp::COMPARE_EQ:
            case TACOp::COMPARE_NE:
                return true;
            default:
                return false;
        }
    }

public:
    std::string generate(
        const TACProgram& tac,
        const Program& sourceProgram
    ) const {
        std::ostringstream output;
        std::ostringstream declarations;
        std::unordered_set<std::string> declared;

        collectDeclarations(
            sourceProgram.statements, declarations, declared
        );

        for (const auto& instruction : tac.instructions) {
            if (isComputedResult(instruction.op) &&
                !instruction.result.empty() &&
                declared.insert(instruction.result).second) {
                declarations << "    "
                             << cppType(instruction.resultType) << " "
                             << instruction.result << "{};\n";
            }
        }

        output << "#include <iostream>\n\n";
        output << "int main() {\n";
        output << declarations.str();

        for (const auto& instruction : tac.instructions) {
            switch (instruction.op) {
                case TACOp::ASSIGN:
                    output << "    " << instruction.result
                           << " = " << instruction.arg1 << ";\n";
                    break;

                case TACOp::ADD:
                    output << "    " << instruction.result << " = "
                           << instruction.arg1 << " + "
                           << instruction.arg2 << ";\n";
                    break;

                case TACOp::SUBTRACT:
                    output << "    " << instruction.result << " = "
                           << instruction.arg1 << " - "
                           << instruction.arg2 << ";\n";
                    break;

                case TACOp::MULTIPLY:
                    output << "    " << instruction.result << " = "
                           << instruction.arg1 << " * "
                           << instruction.arg2 << ";\n";
                    break;

                case TACOp::DIVIDE:
                    output << "    " << instruction.result << " = "
                           << instruction.arg1 << " / "
                           << instruction.arg2 << ";\n";
                    break;

                case TACOp::COMPARE_LT:
                    output << "    " << instruction.result << " = "
                           << instruction.arg1 << " < "
                           << instruction.arg2 << ";\n";
                    break;

                case TACOp::COMPARE_GT:
                    output << "    " << instruction.result << " = "
                           << instruction.arg1 << " > "
                           << instruction.arg2 << ";\n";
                    break;

                case TACOp::COMPARE_LE:
                    output << "    " << instruction.result << " = "
                           << instruction.arg1 << " <= "
                           << instruction.arg2 << ";\n";
                    break;

                case TACOp::COMPARE_GE:
                    output << "    " << instruction.result << " = "
                           << instruction.arg1 << " >= "
                           << instruction.arg2 << ";\n";
                    break;

                case TACOp::COMPARE_EQ:
                    output << "    " << instruction.result << " = "
                           << instruction.arg1 << " == "
                           << instruction.arg2 << ";\n";
                    break;

                case TACOp::COMPARE_NE:
                    output << "    " << instruction.result << " = "
                           << instruction.arg1 << " != "
                           << instruction.arg2 << ";\n";
                    break;

                case TACOp::PRINT:
                    output << "    std::cout << " << instruction.arg1
                           << " << '\\n';\n";
                    break;

                case TACOp::LABEL:
                    output << instruction.result << ": ;\n";
                    break;

                case TACOp::JUMP:
                    output << "    goto " << instruction.result << ";\n";
                    break;

                case TACOp::JUMP_IF_FALSE:
                    output << "    if (!(" << instruction.arg1
                           << ")) goto " << instruction.result << ";\n";
                    break;
            }
        }

        output << "    return 0;\n";
        output << "}\n";

        return output.str();
    }
};

#endif
