# Compiler-for-a-custom-language

# Compiler and Interpreter System

## Overview
This project implements a custom programming language compiler and interpreter system that translates and executes simple imperative programs. The system demonstrates fundamental concepts of compiler construction through a three-phase approach: lexical analysis, parsing with intermediate representation generation, and execution.

## Features

### Language Support
- Variables and arithmetic operations (+, -, *, /)
- Control flow structures (if, while, for, switch-case)
- Input/output operations
- Comparison operators (>, <, !=)
- Basic memory management

### System Components

#### 1. Lexical Analyzer (lexer.cc/h)
- Breaks input code into tokens
- Handles keywords, identifiers, numbers, and operators
- Manages line numbers and error detection
- Supports token stream peeking

#### 2. Parser (project3.cc)
- Implements recursive descent parsing
- Generates intermediate representation
- Handles variable declarations
- Processes control structures and expressions
- Creates instruction list for execution

#### 3. Executor (execute.cc/h)
- Interprets the intermediate representation
- Manages program memory and variable storage
- Handles arithmetic operations
- Processes control flow instructions
- Manages input/output operations

## Building and Running

### Prerequisites
- C++ compiler (supporting C++11 or later)
- Make build system

### Build Instructions
```bash
g++ lexer.cc inputbuf.cc execute.cc main.cc
```

### Running the Compiler
```bash
./compiler < input_file
```
## Implementation Details

### Memory Management
- Uses array-based storage for program memory
- Supports up to 1000 variable allocations
- Implements variable tracking using symbol table

### Error Handling
- Syntax error detection during parsing
- Runtime error checking during execution
- Input validation

## Educational Value
This project serves as:
- A practical example of compiler design principles
- A learning tool for language implementation concepts
- A foundation for exploring advanced compiler concepts
- A demonstration of program execution models

## Future Improvements
- Implementation of optimization passes
- Extended error reporting
- Support for additional language features
- Implementation of a symbol table using hash table for better performance

## Contributing
Feel free to submit issues and enhancement requests!

## License
[MIT License](LICENSE)

## Contact
For any questions or feedback, please open an issue in the repository.

---
*Note: This project was developed as part of a compiler design course and serves educational purposes.*
