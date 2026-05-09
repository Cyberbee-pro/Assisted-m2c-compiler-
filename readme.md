# M2C Compiler

M2C is a modular Morse-flavored compiler that reads `.cym2c` programs, tokenizes them, parses a small statement-level IR, generates GNU assembly, links against an assembly runtime library, and produces a native executable.

## End-To-End Pipeline

```text
CLI input
  -> main.cpp
  -> Lexer
  -> std::vector<Token>
  -> Parser
  -> std::vector<Statement>
  -> CodeGenerator
  -> temp.s
  -> runtime/io.s + runtime/math.s
  -> gcc -no-pie
  -> executable
```

## Main Modules

### Root

- [main.cpp](main.cpp)
- [build.sh](build.sh)
- [run.sh](run.sh)

### Lexer

- [lexer/include/lexer.h](lexer/include/lexer.h)
- [lexer/include/token_types.h](lexer/include/token_types.h)
- [lexer/include/morse.h](lexer/include/morse.h)
- [lexer/source/lexer.cpp](lexer/source/lexer.cpp)
- [lexer/source/morse.cpp](lexer/source/morse.cpp)

### Syntaxer

- [syntaxer/include/parser.h](syntaxer/include/parser.h)
- [syntaxer/source/parser.cpp](syntaxer/source/parser.cpp)

### Generators

- [generators/include/code_generator.h](generators/include/code_generator.h)
- [generators/source/code_generator.cpp](generators/source/code_generator.cpp)

### Runtime

- [runtime/io.s](runtime/io.s)
- [runtime/math.s](runtime/math.s)

## Core Data Nodes

The current pipeline is built around these concrete data shapes:

### `CliOptions`

Produced in `main.cpp` from:

```text
m2c <input.cym2c> -o <output_binary>
```

### `Token`

Produced by the lexer:

```cpp
struct Token
{
    TokenType type;
    std::string value;
    int line;
};
```

### `Operand`

Produced by the parser inside statements.

### `Statement`

The current parsed IR node:

- `PrintString`
- `PrintValue`
- `VariableAssignment`
- `ArithmeticAssignment`

## What Works Today

- root build and run flow
- file-based lexing
- Morse decoding for quoted strings
- token stream production
- parser validation for print and assignment statements
- simple declaration-before-use checks
- pure assembly output
- runtime-backed printing and arithmetic
- automatic assembly/link step

## Supported Syntax

### Print

```text
<".... . .-.. .-.. ---">;
<x>;
<42>;
```

### Assignment

```text
let x = 5;
let y = x;
```

### Arithmetic

```text
let a = 5 + 3;
let b = a * 2;
let c = b - 4;
let d = c / 2;
```

## Build

```bash
bash build.sh
```

This produces:

```text
./m2c_bin
```

## Run

```bash
bash run.sh m2c_files/test_math.cym2c demo_math
```

Or directly:

```bash
./m2c_bin m2c_files/test_math.cym2c -o demo_math
```

## Runtime Assembly Library

The generator links against:

### `runtime/io.s`

- `m2c_print`
- `m2c_print_int`

### `runtime/math.s`

- `m2c_add`
- `m2c_sub`
- `m2c_mul`
- `m2c_div`

This keeps generated assembly smaller and gives the codebase real reusable assembly modules.

## Documentation Map

- deep technical walkthrough: [working.md](working.md)
- contributor guide: [contribution.md](contribution.md)
- alternate contributor entry: [CONTRIBUTING.md](CONTRIBUTING.md)
- lexer internals: [lexer/readme.md](lexer/readme.md)
- parser internals: [syntaxer/readme.md](syntaxer/readme.md)
- generator internals: [generators/readme.md](generators/readme.md)
- semantic-analysis plan: [semanter/readme.md](semanter/readme.md)
- optimizer plan: [optimizers/readme.md](optimizers/readme.md)


