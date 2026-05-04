# Working Of The M2C Compiler

This document explains how the repository works today.

It focuses on the real implementation now present in the codebase:

1. Root-level build and run flow
2. Lexing and token storage
3. Parsing and validation
4. C++ code generation
5. Inline x86 assembly generation for arithmetic
6. Automatic compilation of generated code into an executable

## 1. Big Picture

M2C stands for Morse-to-C, but the current implementation is best described as:

```text
.cym2c source
  -> lexer/tokenizer
  -> parser
  -> validated statement list
  -> C++ code generator
  -> generated .cpp file
  -> g++ system call
  -> final executable
```

So the project is no longer just a lexer prototype. It now performs a small but complete end-to-end compile pipeline.

## 2. Repository Layout

```text
m2c compiler/
├── build.sh
├── run.sh
├── m2c_bin
├── readme.md
├── working.md
├── contribution.md
├── CONTRIBUTING.md
├── m2c_files/
│   ├── test.cym2c
│   ├── test2.cym2c
│   ├── test_math.cym2c
│   └── stress_test.cym2c
├── lexer/
│   ├── include/
│   │   ├── code_generator.h
│   │   ├── excptsextra.h
│   │   ├── morse.h
│   │   ├── parser.h
│   │   ├── tokenMaker.h
│   │   ├── token_types.h
│   │   └── tokens.h
│   ├── source/
│   │   ├── code_generator.cpp
│   │   ├── excptsextra.cpp
│   │   ├── main.cpp
│   │   ├── morse.cpp
│   │   ├── parser.cpp
│   │   └── tokenMaker.cpp
│   ├── compiled/
│   │   └── lexer
│   ├── scripts/
│   │   ├── build.sh
│   │   └── run.sh
│   └── readme.md
├── syntaxer/
│   └── readme.md
├── semanter/
│   └── readme.md
├── optimizers/
│   └── readme.md
└── generators/
    └── readme.md
```

Important note:

- `build.sh` and `run.sh` at the repository root are the active workflow now.
- The older `lexer/scripts/` scripts still exist, but they are no longer the primary entry point.

## 3. What The Compiler Supports Right Now

The current implementation supports a deliberately small MVP language.

### 3.1 Structural Markers

- `/` for the main marker
- `{` and `}` for blocks
- `//` for comments

### 3.2 Print Syntax

Two print forms are supported:

```text
<".... . .-.. .-.. ---">;
<x>;
<42>;
```

Meaning:

- A quoted Morse string is translated by the lexer and later emitted as a string print.
- An identifier or number inside `< >` is emitted as a numeric/value print.

### 3.3 Assignment Syntax

Simple assignment:

```text
let x = 5;
let y = x;
```

Simple arithmetic assignment:

```text
let a = 5 + 3;
let b = a * 2;
let c = b - 4;
let d = c / 2;
```

This is the main arithmetic MVP currently implemented.

## 4. Build And Run Flow

### 4.1 Root Build Script

The active build script is [build.sh](build.sh).

It compiles every `.cpp` file in `lexer/source/`, includes `lexer/include/`, and writes the compiler executable to:

```text
./m2c_bin
```

Equivalent command:

```bash
g++ -std=c++17 lexer/source/*.cpp -I lexer/include -Wall -Wextra -pedantic -o m2c_bin
```

### 4.2 Root Run Script

The active run script is [run.sh](run.sh).

Usage:

```bash
bash run.sh <input.cym2c> [output_binary_name]
```

Examples:

```bash
bash run.sh m2c_files/test.cym2c demo_print
bash run.sh m2c_files/test_math.cym2c demo_math
bash run.sh m2c_files/stress_test.cym2c stress_demo
```

If the second argument is omitted, `run.sh` defaults the output binary name to:

```text
m2c_program
```

### 4.3 CLI Inside The Compiler

Inside [lexer/source/main.cpp](lexer/source/main.cpp), the compiler itself expects:

```text
m2c <input> -o <output_binary_name>
```

So the root script translates the friendlier shell usage into the compiler’s stricter internal CLI.

## 5. High-Level Control Flow

The real control flow today is:

```text
input .cym2c file
  -> FileReader tokenizes source into std::vector<Token>
  -> Parser validates supported statements
  -> Parser returns std::vector<Statement>
  -> CodeGenerator writes <output>_generated.cpp
  -> main.cpp calls g++ with std::system(...)
  -> final executable is produced
```

This all happens in one run of `m2c_bin`.

## 6. Lexing And Token Storage

The lexer driver is implemented inside [lexer/source/main.cpp](lexer/source/main.cpp) as `FileReader`.

### 6.1 Token Model

Tokens are defined in [lexer/include/token_types.h](lexer/include/token_types.h).

Current token categories include:

- `MainMarker`
- `LoopMarker`
- `ConditionalMarker`
- `ExitMarker`
- `LetKeyword`
- `PrintStart`
- `PrintEnd`
- `Separator`
- `BlockStart`
- `BlockEnd`
- `OpenParenthesis`
- `CloseParenthesis`
- `MorseString`
- `Identifier`
- `Number`
- `Assignment`
- `ArithmeticOperator`
- `Comment`

Each token stores:

- token type
- raw or translated value
- line number

### 6.2 What The Lexer Actually Does

`FileReader::readFile()`:

1. Opens the `.cym2c` file
2. Reads it line by line
3. Tokenizes each line character by character
4. Appends tokens into `std::vector<Token>`
5. Returns that token vector to the parser

This is a major architectural change from the earlier lexer-only version that printed diagnostics directly to stdout.

### 6.3 Lexing Rules

The lexer currently recognizes:

- comments using `//`
- block markers `{` and `}`
- print delimiters `<` and `>`
- assignment `=`
- arithmetic operators `+`, `-`, `*`, `/`
- `let`
- identifiers like `x`, `total`, `_tmp`
- integer literals like `5`, `42`, `15000`
- Morse strings inside quotes

### 6.4 Division Versus Main Marker

The character `/` is overloaded:

- it is the main-program marker in the language
- it is also the arithmetic division operator

The lexer resolves this with context:

- if `/` appears between expression operands, it becomes `ArithmeticOperator`
- otherwise it becomes `MainMarker`

This is implemented in `isDivisionOperator(...)` inside [lexer/source/main.cpp](lexer/source/main.cpp).

### 6.5 Morse Translation

Quoted Morse strings are decoded immediately during lexing using [lexer/source/morse.cpp](lexer/source/morse.cpp).

Example:

```text
<".... . .-.. .-.. ---">;
```

becomes a token like:

```text
TokenType::MorseString value="HELLO"
```

The parser and generator never need to decode Morse again. They only consume the translated token value.

## 7. Parsing And Validation

The parser lives in:

- [lexer/include/parser.h](lexer/include/parser.h)
- [lexer/source/parser.cpp](lexer/source/parser.cpp)

### 7.1 Parser Output

The parser does not build a full AST yet.

Instead, it returns:

```cpp
std::vector<Statement>
```

This is a lightweight intermediate representation for the currently supported language subset.

### 7.2 Statement Types

The current parser supports four statement kinds:

- `PrintString`
- `PrintValue`
- `VariableAssignment`
- `ArithmeticAssignment`

### 7.3 Print Validation

The parser validates print forms such as:

```text
<".... . .-.. .-.. ---">;
<x>;
<42>;
```

It enforces:

- `<` must start the statement
- the next token must be a Morse string, identifier, or number
- `>` must close the print
- `;` must terminate the statement

If a print statement is malformed, the parser throws `CompileError`.

### 7.4 Assignment Validation

The parser validates:

```text
let x = 5;
let y = x;
let z = y + 3;
```

It enforces:

- `let` must begin the declaration
- an identifier must follow
- `=` must appear next
- the right-hand side must be either:
  - one operand, or
  - operand operator operand
- the statement must end with `;`

### 7.5 Declaration Checks

The parser also tracks declared identifiers with a simple set.

That means:

- using an identifier in a print before it is declared is rejected
- using an identifier on the right-hand side before declaration is rejected

This is a small semantic check embedded in the current parser layer.

## 8. Code Generation

The generator lives in:

- [lexer/include/code_generator.h](lexer/include/code_generator.h)
- [lexer/source/code_generator.cpp](lexer/source/code_generator.cpp)

### 8.1 Generator Input

The generator consumes:

```cpp
std::vector<Statement>
```

### 8.2 Generator Output

It writes a standalone C++ source file named:

```text
<output_binary_name>_generated.cpp
```

Example:

- output binary: `demo_math`
- generated source: `demo_math_generated.cpp`

### 8.3 Generated File Structure

Each generated file contains:

```cpp
#include <iostream>

int main()
{
    ...
    return 0;
}
```

### 8.4 Print Emission

String print:

```text
<".... . .-.. .-.. ---">;
```

emits:

```cpp
std::cout << "HELLO" << std::endl;
```

Value print:

```text
<x>;
```

emits:

```cpp
std::cout << x << std::endl;
```

### 8.5 Simple Assignment Emission

Input:

```text
let x = 5;
let y = x;
```

emits:

```cpp
int x = 5;
int y = x;
```

or reassignment if the variable already exists later in the generated function.

## 9. Inline Assembly Arithmetic

This is the most distinctive part of the current generator.

Arithmetic assignments do not emit normal C++ arithmetic expressions.

Instead, they emit GCC inline x86 assembly using `__asm__`.

### 9.1 Addition

Input:

```text
let x = 5 + 3;
```

Generates code in this style:

```cpp
int x = 0;
int __m2c_left_0 = 5;
int __m2c_right_0 = 3;
__asm__ (
    "movl %1, %%eax;"
    "addl %2, %%eax;"
    "movl %%eax, %0;"
    : "=r" (x)
    : "r" (__m2c_left_0), "r" (__m2c_right_0)
    : "%eax"
);
```

### 9.2 Subtraction

Uses:

```text
subl
```

### 9.3 Multiplication

Uses:

```text
imull
```

### 9.4 Division

Uses:

```text
cltd
idivl
```

and clobbers both `%eax` and `%edx`.

### 9.5 Temporary Operands

For each arithmetic statement, the generator creates temporary C++ locals like:

```cpp
__m2c_left_0
__m2c_right_0
```

These hold the evaluated operands and are then passed into the inline assembly block.

That keeps the assembly emission simple and readable for presentation/demo purposes.

## 10. Automatic Executable Compilation

After generating the intermediate `.cpp` file, [lexer/source/main.cpp](lexer/source/main.cpp) builds the final executable automatically.

The sequence is:

1. derive `<output_binary_name>_generated.cpp`
2. write that file
3. build a shell-safe `g++` command string
4. call `std::system(...)`
5. fail if the compiler returns a non-zero code

Effective command shape:

```bash
g++ -std=c++17 <generated_cpp> -o <output_binary_name>
```

So the user experience is:

```text
run the compiler once
  -> get generated C++
  -> get final executable
```

## 11. Error Handling

Custom compile-time errors use [lexer/include/excptsextra.h](lexer/include/excptsextra.h) and [lexer/source/excptsextra.cpp](lexer/source/excptsextra.cpp).

`CompileError` is used for:

- malformed line endings
- invalid tokens
- bad print syntax
- bad assignment syntax
- undeclared identifier use
- unexpected parser tokens

Error messages follow this shape:

```text
Compile Error : <message> At Line (<n>) : <line text>
```

This is still simple, but it gives a usable compile-style error surface for demos.

## 12. Example Inputs

### 12.1 Morse Print

From [m2c_files/test.cym2c](m2c_files/test.cym2c):

```text
/{
<".... . .-.. .-.. ---  .... . .-.. .-.. ---  .-- --- .-. .-.. -.. ">;
<".... . .-.. .-.. ---   .-- --- .-. .-.. -..">;
<"....">;
}
```

Produces generated C++ that prints translated strings such as:

```text
HELLO HELLO WORLD
HELLO WORLD
H
```

### 12.2 Arithmetic Demo

From [m2c_files/test_math.cym2c](m2c_files/test_math.cym2c):

```text
/{
let x = 5 + 3;
let y = x * 2;
<x>;
<y>;
<".... . .-.. .-.. ---">;
}
```

Produces output:

```text
8
16
HELLO
```

### 12.3 Stress Test

From [m2c_files/stress_test.cym2c](m2c_files/stress_test.cym2c):

- Morse string output
- chained arithmetic
- multiplication
- subtraction
- division
- switching back and forth between strings and arithmetic

This file is intended as the current end-to-end confidence test.

## 13. What Is Implemented Versus Planned

### 13.1 Implemented Today

- root-level build and run workflow
- strict compiler CLI
- file reading and tokenization
- token storage in `std::vector<Token>`
- Morse string translation
- parser for a small statement subset
- declaration/use validation for that subset
- C++ generation
- inline assembly generation for arithmetic
- automatic final executable compilation

### 13.2 Still Planned Or Incomplete

- full AST hierarchy
- separate semantic-analysis phase
- optimizer phase
- control-flow parsing for `%`, `%%`, `~`, `~~`, `^`
- richer expressions
- type system
- scoped blocks beyond the current lightweight handling
- function support
- full language specification

## 14. Known Limitations

The current compiler is functional, but intentionally narrow.

Important limitations:

- It supports only a small MVP syntax.
- Arithmetic is limited to simple binary expressions such as `a + b`.
- No operator precedence parser exists yet.
- No nested arithmetic expressions like `a + b * c`.
- No floating-point values.
- No explicit semantic-analysis module yet.
- The parser is still acting as both syntax and part of semantics.
- Legacy `lexer/scripts/` files remain in the repo and may confuse new contributors.
- `tokenMaker.cpp` still builds with old warnings, though it is not part of the new pipeline path.

## 15. Recommended Next Steps

The most useful next engineering steps are:

1. Move from `std::vector<Statement>` to a real AST.
2. Split semantic checks out of `Parser` into the `semanter/` phase.
3. Define grammar for control flow markers `%`, `%%`, `~`, `~~`, and `^`.
4. Add nested expression parsing and precedence handling.
5. Decide whether the project should ultimately target generated C or generated C++.
6. Clean up or retire legacy lexer-only artifacts that no longer represent the active architecture.
7. Expand tests around invalid arithmetic, invalid division cases, and malformed mixed Morse/math programs.

## 16. Final Summary

Today, M2C is no longer just a Morse-aware lexer.

It is now a small working compiler pipeline that:

- tokenizes `.cym2c` source
- validates a minimal print-and-assignment language
- translates Morse strings
- emits C++ code
- uses inline x86 assembly for arithmetic
- invokes `g++`
- produces runnable executables

For the project overview, see [readme.md](readme.md). For contributor workflow, see [contribution.md](contribution.md).
