
#include <iostream>
#include "symbol_table.hpp"

int main() {
    SymbolTable symbols;

    symbols.declareVariable("x");
    symbols.setValue("x", 10);

    std::cout << "x = " << symbols.getValue("x") << '\n';

    try {
        symbols.setValue("z", 20);
    } catch (const std::exception& error) {
        std::cout << "Error: " << error.what() << '\n';
    }

    return 0;
}
