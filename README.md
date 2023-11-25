# Project 3 CSE 340 Fall 2023 - Parser

## Overview
This project, authored by Caleb Lamoreaux, is part of the CSE 340 course at ASU for Fall 2023. The primary focus of the project is the implementation of a Parser class in C++.

## Description
The Parser class is designed to parse a specific programming language and generate an intermediate representation for further processing. It leverages additional components such as `execute.h` and `lexer.h` for execution and lexical analysis.

## Key Components
- **Program Structure**: The `Program` struct holds essential program information during compile-time and aids in generating intermediate representations.
- **Expression Handling**: The `Expression` struct simplifies handling arithmetic expressions, while the `Condition` struct is used for conditional expressions.
- **InstructionList**: This struct is crucial for efficient instruction handling, allowing for O(N) append operations.
- **Parsing Methods**: The Parser class includes methods for parsing different sections of a program, such as variable sections, statements, expressions, and control structures like loops and conditionals.

## Requirements
- C++ Compiler
- Standard C++ Libraries
- Custom headers: `execute.h`, `lexer.h`

## Installation and Usage
Ensure that all the required files and libraries are included in the project directory. Compile the project using a C++ compiler and run the executable to parse the input files.

## Contributing
This project is a part of an academic course and is primarily intended for educational purposes. Contributions are not sought at this time.

## License
This project is licensed under the terms of the Arizona State University academic program.

## Contact
For queries regarding this project, please contact Caleb Lamoreaux at [caleblamro@gmail.com].