# AI-Assisted Pseudocode-to-C++17 Compiler

An AI-assisted compiler that converts natural-language programming instructions into structured pseudocode, translates the pseudocode into C++17, and compares the output of the generated program with the compiler's interpreter.

## Features

- Natural-language normalization using a locally hosted Qwen 2.5 1.5B model through Ollama.
- Lexical analysis using Flex.
- Syntax analysis using GNU Bison.
- Abstract Syntax Tree (AST) construction and semantic analysis.
- Three-Address Code (TAC) generation.
- C++17 source-code generation.
- Compilation using g++ and output comparison.

## Architecture

Natural-language instruction → Python AI normalizer → Ollama/Qwen → pseudocode → Flex lexer → Bison parser → AST and semantic analysis → TAC → C++17 generation → compilation and output verification.

## Requirements

- Ubuntu or WSL2 Ubuntu
- Python 3
- C++17-compatible g++
- Flex
- GNU Bison
- CMake
- Ollama
- Qwen 2.5 1.5B model

## Setup

Install the system dependencies:

```bash
sudo apt update
sudo apt install build-essential flex bison cmake python3
```

Install Ollama and download the model:

```bash
ollama pull qwen2.5:1.5b
```

Ensure the Ollama API is reachable from the environment running the normalizer.

## Build

From the project root:

```bash
mkdir -p build

bison -d --defines=build/parser.tab.h \
    -o build/parser.tab.cpp grammar/parser.y

flex -o build/lexer.cpp grammar/lexer.l

g++ -std=c++17 -Wall -Wextra -pedantic \
    -Iinclude -Ibuild \
    build/parser.tab.cpp build/lexer.cpp \
    -o build/pseudocode_compiler
```

## Run

Start the AI-assisted workflow:

```bash
python3 ai_normalizer.py
```

Enter a natural-language instruction and finish the input by pressing Enter on an empty line.

## Example

Instruction:

> Start x at 1. While x is at most 3, print x and increment x by 1.

Expected output:

```text
1
2
3
Verification passed.
```

## Current Language Support

The implemented pseudocode language supports variable declarations, assignments, arithmetic expressions, printing, conditional statements, loops, and comparison operators as defined by the grammar.

## Verification

The compiler compares the interpreter's output with the output of the generated C++ program. Matching outputs indicate agreement for that execution; this is not a proof of full semantic equivalence or correctness of the original AI interpretation.

## Limitations and Future Work

- Improve AI output completeness and automatic correction.
- Expand the supported language constructs.
- Add broader regression and semantic tests.
- Improve diagnostics and validation.
- Develop a graphical interface.

## References

- Flex: https://github.com/westes/flex
- GNU Bison: https://www.gnu.org/software/bison/
- GCC: https://gcc.gnu.org/
- Ollama: https://ollama.com/
- Qwen2.5: https://github.com/QwenLM/Qwen2.5
