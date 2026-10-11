
#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <memory>
#include <utility>
#include <stdexcept>
#include <string>
#include <vector>


#include "symbol_table.hpp"

class Expr {
public:
    virtual ~Expr() = default;
    virtual double evaluate(const SymbolTable& symbols) const = 0;
    virtual void analyze(const SymbolTable& symbols) const = 0;
    virtual DataType getType(const SymbolTable& symbols) const = 0;
    virtual void print() const = 0;
};


class NumberExpr : public Expr {
public:
    double value;
    DataType type;

    NumberExpr(double value, DataType type)
        : value(value), type(type) {}

    double evaluate(const SymbolTable&) const override {
        return value;
    }

    void analyze(const SymbolTable&) const override {}

    DataType getType(const SymbolTable&) const override {
        return type;
    }

    void print() const override {
        std::cout << value;
    }
};


class VariableExpr : public Expr {
public:
    std::string name;

    explicit VariableExpr(std::string name)
        : name(std::move(name)) {}

    double evaluate(const SymbolTable& symbols) const override {
        return symbols.getValue(name);
    }

    void print() const override {
        std::cout << name;
    }

    void analyze(const SymbolTable& symbols) const override {
        if (!symbols.isDeclared(name)) {
            throw std::runtime_error("Undeclared variable: " + name);
        }
    }

    DataType getType(const SymbolTable& symbols) const override {
        return symbols.getType(name);
    }
};


enum class BinaryOperator {
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    LESS,
    GREATER,
    LESS_EQUAL,
    GREATER_EQUAL,
    EQUAL,
    NOT_EQUAL
};

class BinaryExpr : public Expr {
public:
    BinaryOperator op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    BinaryExpr(BinaryOperator op, Expr* left, Expr* right)
    : op(op), left(left), right(right) {}


    double evaluate(const SymbolTable& symbols) const override {
        double lhs = left->evaluate(symbols);
        double rhs = right->evaluate(symbols);

        switch (op) {
            case BinaryOperator::ADD:
                return lhs + rhs;

            case BinaryOperator::SUBTRACT:
                return lhs - rhs;

            case BinaryOperator::MULTIPLY:
                return lhs * rhs;

            case BinaryOperator::DIVIDE:
                if (rhs == 0) {
                    throw std::runtime_error("Division by zero");
                }

                if (left->getType(symbols) == DataType::INT &&
                    right->getType(symbols) == DataType::INT) {
                    return static_cast<int>(lhs) / static_cast<int>(rhs);
                }

                return lhs / rhs;

            case BinaryOperator::LESS:
                return lhs < rhs;

            case BinaryOperator::GREATER:
                return lhs > rhs;

            case BinaryOperator::LESS_EQUAL:
                return lhs <= rhs;

            case BinaryOperator::GREATER_EQUAL:
                return lhs >= rhs;

            case BinaryOperator::EQUAL:
                return lhs == rhs;

            case BinaryOperator::NOT_EQUAL:
                return lhs != rhs;
        }

        throw std::runtime_error("Unknown binary operator");
    }



    void print() const override {
        const char* symbol = "";

        switch (op) {
            case BinaryOperator::ADD: symbol = "+"; break;
            case BinaryOperator::SUBTRACT: symbol = "-"; break;
            case BinaryOperator::MULTIPLY: symbol = "*"; break;
            case BinaryOperator::DIVIDE: symbol = "/"; break;
            case BinaryOperator::LESS: symbol = "<"; break;
            case BinaryOperator::GREATER: symbol = ">"; break;
            case BinaryOperator::LESS_EQUAL: symbol = "<="; break;
            case BinaryOperator::GREATER_EQUAL: symbol = ">="; break;
            case BinaryOperator::EQUAL: symbol = "=="; break;
            case BinaryOperator::NOT_EQUAL: symbol = "!="; break;
        }

        std::cout << "(" << symbol << " ";
        left->print();
        std::cout << " ";
        right->print();
        std::cout << ")";
    }

    void analyze(const SymbolTable& symbols) const override {
        left->analyze(symbols);
        right->analyze(symbols);
    }

    
    DataType getType(const SymbolTable& symbols) const override {
        switch (op) {
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

        DataType leftType = left->getType(symbols);
        DataType rightType = right->getType(symbols);

        if (leftType == DataType::FLOAT ||
            rightType == DataType::FLOAT) {
            return DataType::FLOAT;
        }

        return DataType::INT;
    }


};

class UnaryExpr : public Expr {
public:
    char op;
    std::unique_ptr<Expr> operand;

    UnaryExpr(char op, Expr* operand)
        : op(op), operand(operand) {}

    double evaluate(const SymbolTable& symbols) const override {
        double value = operand->evaluate(symbols);

        if (op == '-') {
            return -value;
        }

        throw std::runtime_error("Unknown unary operator");
    }

    void print() const override {
        std::cout << "(" << op << " ";
        operand->print();
        std::cout << ")";
    }

    void analyze(const SymbolTable& symbols) const override {
        operand->analyze(symbols);
    }

    DataType getType(const SymbolTable& symbols) const override {
        return operand->getType(symbols);
    }

};

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void execute(SymbolTable& symbols) const = 0;
    virtual void print() const = 0;
    virtual void analyze(SymbolTable& symbols) const = 0;
};


class DeclareStmt : public Stmt {
public:
    std::string name;
    DataType type;

    DeclareStmt(std::string name, DataType type)
        : name(std::move(name)), type(type) {}

    void execute(SymbolTable& symbols) const override {
        symbols.declareVariable(name, type);
    }

    void analyze(SymbolTable& symbols) const override {
        symbols.declareVariable(name, type);
    }

    void print() const override {
        std::cout << "(DECLARE " << name << ")";
    }
};


class AssignStmt : public Stmt {
public:
    std::string name;
    std::unique_ptr<Expr> expression;

    AssignStmt(std::string name, Expr* expression)
        : name(std::move(name)), expression(expression) {}

    void execute(SymbolTable& symbols) const override {
        symbols.setValue(name, expression->evaluate(symbols));
    }

    void print() const override {
        std::cout << "(ASSIGN " << name << " ";
        expression->print();
        std::cout << ")";
    }

    
    void analyze(SymbolTable& symbols) const override {
        if (!symbols.isDeclared(name)) {
            throw std::runtime_error("Undeclared variable: " + name);
        }

        expression->analyze(symbols);

        DataType targetType = symbols.getType(name);
        DataType expressionType = expression->getType(symbols);

        if (targetType == DataType::INT &&
            expressionType == DataType::FLOAT) {
            throw std::runtime_error(
                "Cannot assign FLOAT expression to INT variable: " + name
            );
        }
    }

};

class PrintStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;

    explicit PrintStmt(Expr* expression)
        : expression(expression) {}

    void execute(SymbolTable& symbols) const override {
        std::cout << expression->evaluate(symbols) << '\n';
    }

    void print() const override {
        std::cout << "(PRINT ";
        expression->print();
        std::cout << ")";
    }

    void analyze(SymbolTable& symbols) const override {
        expression->analyze(symbols);
    }
};


class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> thenBranch;
    std::vector<std::unique_ptr<Stmt>> elseBranch;

    IfStmt(
        Expr* condition,
        std::vector<std::unique_ptr<Stmt>> thenBranch,
        std::vector<std::unique_ptr<Stmt>> elseBranch
    )
        : condition(condition),
          thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)) {}

    void analyze(SymbolTable& symbols) const override {
        condition->analyze(symbols);

        if (condition->getType(symbols) != DataType::BOOLEAN) {
            throw std::runtime_error(
                "IF condition must be BOOLEAN"
            );
        }

        for (const auto& statement : thenBranch) {
            statement->analyze(symbols);
        }

        for (const auto& statement : elseBranch) {
            statement->analyze(symbols);
        }
    }

    void execute(SymbolTable& symbols) const override {
        if (condition->evaluate(symbols) != 0) {
            for (const auto& statement : thenBranch) {
                statement->execute(symbols);
            }
        } else {
            for (const auto& statement : elseBranch) {
                statement->execute(symbols);
            }
        }
    }

    void print() const override {
        std::cout << "(IF ";
        condition->print();

        std::cout << " THEN {";
        for (const auto& statement : thenBranch) {
            statement->print();
        }

        std::cout << "} ELSE {";
        for (const auto& statement : elseBranch) {
            statement->print();
        }

        std::cout << "})";
    }
};


class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;

    WhileStmt(
        Expr* condition,
        std::vector<std::unique_ptr<Stmt>> body
    )
        : condition(condition),
          body(std::move(body)) {}

    void analyze(SymbolTable& symbols) const override {
        condition->analyze(symbols);

        if (condition->getType(symbols) != DataType::BOOLEAN) {
            throw std::runtime_error(
                "WHILE condition must be BOOLEAN"
            );
        }

        for (const auto& statement : body) {
            statement->analyze(symbols);
        }
    }

    void execute(SymbolTable& symbols) const override {
        while (condition->evaluate(symbols) != 0) {
            for (const auto& statement : body) {
                statement->execute(symbols);
            }
        }
    }

    void print() const override {
        std::cout << "(WHILE ";
        condition->print();
        std::cout << " DO {";

        for (const auto& statement : body) {
            statement->print();
        }

        std::cout << "})";
    }
};

class Program {
public:
    std::vector<std::unique_ptr<Stmt>> statements;



    void addStatement(std::unique_ptr<Stmt> statement) {
        statements.push_back(std::move(statement));
    }


    void print() const {
        std::cout << "Program AST:\n";

        for (const auto& statement : statements) {
            statement->print();
            std::cout << '\n';
        }
    }

    void execute() const {
        SymbolTable symbols;

        for (const auto& statement : statements) {
            statement->execute(symbols);
        }
    }

    void analyze() const {
        SymbolTable symbols;

        for (const auto& statement : statements) {
            statement->analyze(symbols);
        }
    }
};

#endif
