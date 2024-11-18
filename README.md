# Umbra Lang Compiler
This repository contains the implementation of the Umbra Lang compiler. The project is structured as follows:

```plaintext
│ umbra-lang/
│
├── src/                    # Source files for the compiler
│   ├── lexer/              # Lexical analyzer implementation
│   ├── parser/             # Syntactic parser 
│   ├── ast/                # Abstract Syntax Tree definitions
│   ├── codegen/            # Code generation for LLVM IR
│   └── main.cpp            # Main entry point of the compiler
│
├── include/                # Public header files
│
├── lib/                    # External libraries
│
├── test/                   # Test files
│   ├── lexer_tests/        # Tests for the lexical analyzer
│   ├── parser_tests/       # Tests for the parser 
│   └── codegen_tests/      # Tests for code generation
│
├── examples/               # Example Umbra Lang programs
│
├── docs/                   # Documentation files
│
├── build/                  # Build output directory
│
├── .vscode/                # VS Code specific settings
│   ├── tasks.json          # Build tasks configuration
│   └── launch.json         # Debugger configuration
│
├── CMakeLists.txt          # CMake configuration file
├── .gitignore              # Git ignore file
└── README.md               # This file
```

## Directory Contents

**src/**: Contains all the source code for the Umbra Lang compiler.

- **lexer/:** Implementation of the lexical analyzer.
- **parser/:** Will contain the syntactic parser (to be implemented).
- **ast/:** Definitions for the Abstract Syntax Tree nodes.
- **codegen/:** Code generation module for producing LLVM IR.
- **main.cpp:** The main entry point of the compiler.


**include/:** Public header files, if the compiler exposes any public API.

**lib/:** External libraries or dependencies, if any are used.

**test/:** Contains all test files to ensure the compiler's correctness.

**examples/:** Sample Umbra Lang programs to demonstrate language features.

**docs/:** Project documentation, language specifications, etc.

**build/:** Directory for build artifacts (not tracked in git).

**.vscode/:** Visual Studio Code configuration for easier development.

**CMakeLists.txt:** CMake build system configuration.

**.gitignore:** Specifies intentionally untracked files to ignore.

**README.md:** Provides an overview of the project (you are here!).

## Getting Started

- [Setup](./SETUP.md)

- [How to contribute](./HOW_TO_CONTRIBUTE.md)


## License
Apache 2.0 "Umbra" 