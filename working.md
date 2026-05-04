# Working Of The M2C Compiler

This document describes the code that actually exists in the repository today.

## 1. Big Picture

M2C is currently a small end-to-end compiler, not just a lexer prototype.

It now performs this flow:

```text
.cym2c source
  -> Lexer
  -> std::vector<Token>
  -> Parser
  -> std::vector<Statement>
  -> GNU assembly generator
  -> temp.s
  -> runtime/io.s + runtime/math.s
  -> gcc -no-pie
  -> final executable
```

## 2. Active Layout

```text
m2c compiler/
├── build.sh
├── run.sh
├── main.cpp
├── runtime/
│   ├── io.s
│   └── math.s
├── lexer/
│   ├── include/
│   │   ├── lexer.h
│   │   ├── token_types.h
│   │   ├── morse.h
│   │   └── excptsextra.h
│   └── source/
│       ├── lexer.cpp
│       ├── morse.cpp
│       ├── excptsextra.cpp
│       └── tokenMaker.cpp
├── syntaxer/
│   ├── include/parser.h
│   └── source/parser.cpp
├── generators/
│   ├── include/code_generator.h
│   └── source/code_generator.cpp
└── m2c_files/
```

## 3. Root Orchestration

The root entrypoint is [main.cpp](main.cpp).

Its job is intentionally small:

1. Parse CLI arguments
2. Invoke the `Lexer`
3. Invoke the `Parser`
4. Invoke the `CodeGenerator`
5. Call `gcc temp.s runtime/io.s runtime/math.s -no-pie -o <output>`

Expected CLI:

```text
m2c <input.cym2c> -o <output_binary>
```

## 4. Root Scripts

### Build

[build.sh](build.sh) compiles:

- `main.cpp`
- all C++ files in `lexer/source/`
- all C++ files in `syntaxer/source/`
- all C++ files in `generators/source/`

and includes:

- `lexer/include/`
- `syntaxer/include/`
- `generators/include/`

Output:

```text
./m2c_bin
```

### Run

[run.sh](run.sh) is a convenience wrapper around `m2c_bin`.

Usage:

```bash
bash run.sh <input.cym2c> [output_binary_name]
```

It forwards to:

```text
./m2c_bin <input> -o <output>
```

## 5. Lexer

The lexer lives in:

- [lexer/include/lexer.h](lexer/include/lexer.h)
- [lexer/source/lexer.cpp](lexer/source/lexer.cpp)

### Responsibilities

- open the source file
- scan line by line
- tokenize characters into `Token` objects
- decode Morse strings inside quotes
- detect malformed end-of-line syntax

### Token Model

Tokens live in [lexer/include/token_types.h](lexer/include/token_types.h).

Current token categories include:

- `MainMarker`
- `LetKeyword`
- `PrintStart`
- `PrintEnd`
- `Separator`
- `BlockStart`
- `BlockEnd`
- `MorseString`
- `Identifier`
- `Number`
- `Assignment`
- `ArithmeticOperator`
- `Comment`

Each token stores:

- type
- value
- line number

### Lexing Details

The lexer recognizes:

- `/` as either the main marker or division operator depending on context
- `{` and `}`
- `//` comments
- `<` and `>`
- `=`
- `+`, `-`, `*`, `/`
- `let`
- identifiers
- integer literals
- quoted Morse strings

Quoted Morse is decoded immediately through [lexer/source/morse.cpp](lexer/source/morse.cpp), so later phases consume translated strings, not raw Morse symbols.

## 6. Parser

The parser lives in:

- [syntaxer/include/parser.h](syntaxer/include/parser.h)
- [syntaxer/source/parser.cpp](syntaxer/source/parser.cpp)

The parser does not build a full AST yet.

Instead, it returns:

```cpp
std::vector<Statement>
```

This is a lightweight intermediate representation for the currently supported language.

### Statement Types

- `PrintString`
- `PrintValue`
- `VariableAssignment`
- `ArithmeticAssignment`

### Supported Syntax

```text
<".... . .-.. .-.. ---">;
<x>;
<42>;

let x = 5;
let y = x;
let z = y + 3;
let q = z / 2;
```

### Validation Performed

- print statements must be closed with `>`
- print and assignment statements must end with `;`
- operands must be identifiers or integer literals
- identifiers must be declared before use

The parser is still carrying a small amount of semantic responsibility by tracking declared variables.

## 7. Generator

The generator lives in:

- [generators/include/code_generator.h](generators/include/code_generator.h)
- [generators/source/code_generator.cpp](generators/source/code_generator.cpp)

It generates a pure GNU assembly file:

```text
temp.s
```

### Generated Structure

The output assembly contains:

- `.section .rodata`
- string labels for translated Morse literals
- `.text`
- `.globl main`
- a stack frame in `main`
- calls into the runtime library

### Print Emission

For string prints, the generator emits a string label and:

```asm
leaq str0(%rip), %rdi
call m2c_print
```

For numeric or variable prints, it loads the value into `%edi` and emits:

```asm
call m2c_print_int
```

### Arithmetic Emission

Arithmetic assignments no longer inline the full math implementation.

Instead, the generator loads operands into `%edi` and `%esi`, then calls:

- `m2c_add`
- `m2c_sub`
- `m2c_mul`
- `m2c_div`

The return value arrives in `%eax` and is written back to the target variable slot on the stack.

## 8. Runtime Assembly Library

The runtime is stored in:

- [runtime/io.s](runtime/io.s)
- [runtime/math.s](runtime/math.s)

This is now part of the actual repository architecture, not just generated output.

### `runtime/io.s`

Provides:

- `m2c_print`
- `m2c_print_int`

`m2c_print` calls `puts@PLT`.

`m2c_print_int` calls `printf@PLT` with a built-in integer format string.

### `runtime/math.s`

Provides:

- `m2c_add`
- `m2c_sub`
- `m2c_mul`
- `m2c_div`

`m2c_div` includes basic zero-division protection by returning `0` if the divisor is zero.

## 9. Error Handling

Compile-time errors are reported through:

- [lexer/include/excptsextra.h](lexer/include/excptsextra.h)
- [lexer/source/excptsextra.cpp](lexer/source/excptsextra.cpp)

`CompileError` is used for:

- invalid tokens
- missing semicolons
- malformed print statements
- malformed assignments
- use of undeclared identifiers
- unexpected parser tokens

Message shape:

```text
Compile Error : <message> At Line (<n>) : <line text>
```

## 10. Examples

### Morse Print

Input:

```text
<".... . .-.. .-.. ---">;
```

Result:

```text
HELLO
```

### Arithmetic

Input:

```text
let x = 5 + 3;
let y = x * 2;
<x>;
<y>;
```

Result:

```text
8
16
```

## 11. Current Limits

The compiler is real, but still intentionally narrow.

Current limits:

- no full AST hierarchy yet
- no separate semantic-analysis phase yet
- no optimizer implementation yet
- no nested expression parser
- no operator precedence beyond single binary expressions
- no full control-flow implementation for `%`, `%%`, `~`, `~~`, `^`
- one shared intermediate output file: `temp.s`

That last point means parallel compiler runs can race each other.

## 12. Next Useful Steps

The most valuable next improvements are:

1. Replace `std::vector<Statement>` with a real AST
2. Split semantic checks out of the parser and into `semanter/`
3. Expand expression parsing beyond one operator
4. Add control-flow grammar for the existing markers
5. Make the temporary assembly filename unique per build
6. Add formal tests for parser failures and runtime behavior

## 13. Summary

Today, M2C is a modular compiler that:

- tokenizes `.cym2c`
- parses a small Morse-aware language subset
- generates pure GNU assembly
- links against an assembly runtime
- produces runnable native executables
