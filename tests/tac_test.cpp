
#include <iostream>
#include "tac.hpp"

int main() {
    TACProgram program;

    // Represents: t1 = 10 + 20
    program.emit(TACOp::ADD, "t1", "10", "20");

    // Represents: x = t1
    program.emit(TACOp::ASSIGN, "x", "t1");

    std::cout << "Number of TAC instructions: "
              << program.instructions.size() << '\n';

    for (const auto& instruction : program.instructions) {
        std::cout << instruction.result << '\n';
    }

    return 0;
}
