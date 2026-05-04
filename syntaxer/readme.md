# Syntaxer

The syntaxer is the parser phase of M2C.

Its implementation lives in:

- [include/parser.h](include/parser.h)
- [source/parser.cpp](source/parser.cpp)

## Current Role

The parser consumes:

```cpp
std::vector<Token>
```

and produces:

```cpp
std::vector<Statement>
```

This is a lightweight intermediate representation, not a full AST yet.

## Supported Statement Types

- `PrintString`
- `PrintValue`
- `VariableAssignment`
- `ArithmeticAssignment`

## Supported Syntax

```text
<".... . .-.. .-.. ---">;
<x>;
<42>;

let x = 5;
let y = x;
let z = y + 3;
let q = z / 2;
```

## Validation Performed

- print statements must use `< ... >;`
- assignments must use `let name = ...;`
- arithmetic is limited to a single binary operator
- identifiers must be declared before use
- malformed token sequences raise `CompileError`

## Why This Is Not Yet A Full AST

The current parser was kept intentionally small so the compiler could reach a working end-to-end state quickly.

Right now it optimizes for:

- simple structure
- clear errors
- easy handoff to the generator

instead of full language expressiveness.

## Next Logical Improvements

- replace `std::vector<Statement>` with a real AST
- separate semantic checks into `semanter/`
- support nested expressions
- add operator precedence
- add control-flow parsing
