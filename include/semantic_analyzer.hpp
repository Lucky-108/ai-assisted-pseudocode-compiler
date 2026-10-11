
#ifndef SEMANTIC_ANALYZER_HPP
#define SEMANTIC_ANALYZER_HPP

#include "ast.hpp"

class SemanticAnalyzer {
public:
    void analyze(const Program& program) {
        program.analyze();
    }
};

#endif

