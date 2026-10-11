
%code requires {
    #include <memory>
    #include <string>
    #include "ast.hpp"
    #include<vector>
    struct NumberLiteral {
        double value;
        DataType type;
    };
}

%{
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <utility>
#include <string>
#include "verifier.hpp"
#include "cpp_generator.hpp"
#include <fstream>
#include "ast.hpp"
#include "semantic_analyzer.hpp"
#include "tac_generator.hpp"

int yylex(void);
void yyerror(const char *message);

std::unique_ptr<Program> programRoot;
%}

%union {
    double number;
    std::string* identifier;
    Expr* expr;
    Stmt* stmt;
    NumberLiteral* numberLiteral;
    std::vector<std::unique_ptr<Stmt>>* block;
}

%token PRINT INT FLOAT IF ELSE WHILE
%token EQ NE LE GE LT GT
%token <numberLiteral> NUMBER
%token <identifier> IDENTIFIER

%type <expr> expression
%type <stmt> statement
%type <block> statement_list block

%destructor { delete $$; } <expr> <stmt> <identifier> <block>


%nonassoc EQ NE LT GT LE GE
%left '+' '-'
%left '*' '/'
%precedence UMINUS


%%


program:
    statement_list
    {
        programRoot = std::make_unique<Program>();

        for (auto& stmt : *$1) {
            programRoot->addStatement(std::move(stmt));
        }

        delete $1;
    }
;

block:
    '{' statement_list '}'
    {
        $$ = $2;
    }
;

statement_list:
    %empty
    {
        $$ = new std::vector<std::unique_ptr<Stmt>>();
    }
    | statement_list statement
    {
        $1->emplace_back($2);
        $$ = $1;
    }
;



statement:
    INT IDENTIFIER ';'
    {
        $$ = new DeclareStmt(*$2, DataType::INT);
        delete $2;
    }
    | FLOAT IDENTIFIER ';'
    {
        $$ = new DeclareStmt(*$2, DataType::FLOAT);
        delete $2;
    }
    | IDENTIFIER '=' expression ';'
    {
        $$ = new AssignStmt(*$1, $3);
        delete $1;
    }
    | PRINT expression ';'
    {
        $$ = new PrintStmt($2);
    }
    | IF '(' expression ')' block ELSE block
    {
        $$ = new IfStmt(
            $3,
            std::move(*$5),
            std::move(*$7)
        );

        delete $5;
        delete $7;
    }
    | WHILE '(' expression ')' block
    {
        $$ = new WhileStmt(
            $3,
            std::move(*$5)
        );

        delete $5;
    }

;

expression:
   
    
    NUMBER {
        $$ = new NumberExpr(
            $1->value,
            $1->type
        );
        delete $1;
    }
    | IDENTIFIER
    {
        $$ = new VariableExpr(*$1);
        delete $1;
    }
    | expression '+' expression
    {
        $$ = new BinaryExpr(BinaryOperator::ADD, $1, $3);
    }
    | expression '-' expression
    {
        $$ = new BinaryExpr(BinaryOperator::SUBTRACT, $1, $3);
    }
    | expression '*' expression
    {
        $$ = new BinaryExpr(BinaryOperator::MULTIPLY, $1, $3);
    }
    | expression '/' expression
    {
        $$ = new BinaryExpr(BinaryOperator::DIVIDE, $1, $3);
    }
    | expression LT expression
    {
        $$ = new BinaryExpr(BinaryOperator::LESS, $1, $3);
    }
    | expression GT expression
    {
        $$ = new BinaryExpr(BinaryOperator::GREATER, $1, $3);
    }
    | expression LE expression
    {
        $$ = new BinaryExpr(BinaryOperator::LESS_EQUAL, $1, $3);
    }
    | expression GE expression
    {
        $$ = new BinaryExpr(BinaryOperator::GREATER_EQUAL, $1, $3);
    }
    | expression EQ expression
    {
        $$ = new BinaryExpr(BinaryOperator::EQUAL, $1, $3);
    }
    | expression NE expression
    {
        $$ = new BinaryExpr(BinaryOperator::NOT_EQUAL, $1, $3);
    }
;

%%

void yyerror(const char *message) {
    std::fprintf(stderr, "Error: %s\n", message);
}


int main() {
    if (yyparse() != 0) {
        return 1;
    }

    if (programRoot) {
        SemanticAnalyzer analyzer;

        try {
            analyzer.analyze(*programRoot);
        } catch (const std::exception& error) {
            yyerror(error.what());
            return 1;
        }

        TACGenerator generator;
        generator.generateProgram(*programRoot);

        std::cout << "\nThree-Address Code (TAC):\n";
        generator.printTAC();
        
        CPPGenerator cppGenerator;

        std::string generatedCpp =
            cppGenerator.generate(generator.getProgram(), *programRoot);

        std::ofstream cppFile("build/generated_program.cpp");

        if (!cppFile) {
            std::cerr << "Error: Could not create generated C++ file.\n";
            return 1;
        }

        cppFile << generatedCpp;
        cppFile.close();

        CppVerifier verifier;

        if (!verifier.compile(
                "build/generated_program.cpp",
                "build/generated_program")) {
            return 1;
        }

        std::cout << "\nGenerated C++17 code saved to "
                << "build/generated_program.cpp\n";


        programRoot->print();

        
        std::cout << "\nOutput:\n";

        std::ostringstream interpreterOutput;
        std::streambuf* originalBuffer =
            std::cout.rdbuf(interpreterOutput.rdbuf());

        try {
            programRoot->execute();
        } catch (const std::exception& error) {
            std::cout.rdbuf(originalBuffer);
            std::cerr << "Interpreter error: " << error.what() << '\n';
            return 1;
        }

        std::cout.rdbuf(originalBuffer);

        const std::string expectedOutput = interpreterOutput.str();
        std::cout << expectedOutput;

        if (!verifier.compareOutput(
                "build/generated_program",
                expectedOutput)) {
            return 1;
        }
    }
    return 0;
}


