# M2C Compiler

M2C is a Morse-code-flavored compiler project whose long-term goal is to translate `.cym2c` source files into valid C. The repository already contains a working lexer executable and design notes for the remaining compiler stages.

## Current Status

The project is split into five compiler phases:

| Phase | Folder | Status | What it does today |
|---|---|---|---|
| Lexer | `lexer/` | Implemented | Reads `.cym2c` input, recognizes separators and keywords, translates Morse inside quoted strings, prints token/debug information |
| Syntaxer | `syntaxer/` | Planned | Parser and AST stage described in docs only |
| Semanter | `semanter/` | Planned | Semantic analysis stage described in docs only |
| Optimizers | `optimizers/` | Planned | Optimization stage described in docs only |
| Generators | `generators/` | Planned | C code generation stage described in docs only |

If you want the complete technical walkthrough, read [working.md](working.md). That document explains both the real code path that works now and the intended end-to-end compiler pipeline.

## Pipeline Overview

The target architecture is:

```text
.cym2c source
  -> lexer
  -> syntaxer
  -> semanter
  -> optimizers
  -> generators
  -> C output
```

The current executable stops after the lexer-style inspection stage and prints its findings to standard output.

## What The Current Lexer Recognizes

The lexer code currently understands:

- Main marker: `/`
- Loop and control markers: `%`, `~`, `^`
- Print delimiters: `<` and `>`
- Separators: `{`, `}`, `,`, `;`, and spaces
- Parentheses: `(` and `)`
- Comments beginning with `//`
- Morse sequences inside quoted strings such as `".... . .-.. .-.. ---"`

Morse letters and digits are mapped in [lexer/include/morse.h](lexer/include/morse.h). Keyword and separator metadata live in [lexer/include/tokens.h](lexer/include/tokens.h).

## Quick Start

### Build

The checked-in build script uses paths relative to `lexer/scripts`, so run it from that directory:

```bash
cd lexer/scripts
bash build.sh
```

If you prefer compiling from the repository root, this equivalent command also works:

```bash
g++ lexer/source/main.cpp lexer/source/excptsextra.cpp lexer/source/morse.cpp \
  -I lexer/include \
  -o lexer/compiled/lexer \
  -Wall -Wextra
```

### Run

After building, run:

```bash
./lexer/compiled/lexer
```

The current `main()` function is hardcoded to read:

```text
m2c_files/test.cym2c
```

## Example

Input file:

```text
/{
    <".... . .-.. .-.. ---  .... . .-.. .-.. ---  .-- --- .-. .-.. -.. ">;
}
```

Current lexer output includes:

```text
Keyword : "/"
Keyword : "<"
Found Quote!
morse Translation? HELLO HELLO WORLD
```

This shows the implemented behavior clearly: the program is already translating Morse inside strings, but it is not yet building an AST or generating C output.

## Repository Map

```text
m2c compiler/
├── readme.md
├── working.md
├── contribution.md
├── CONTRIBUTING.md
├── m2c_files/
├── lexer/
│   ├── include/
│   ├── source/
│   ├── scripts/
│   ├── compiled/
│   └── readme.md
├── syntaxer/
├── semanter/
├── optimizers/
└── generators/
```

## Important Notes

- The lexer currently prints diagnostics rather than returning a reusable token vector to later phases.
- `Token` is defined in [lexer/source/main.cpp](lexer/source/main.cpp) but is not yet used to power a parser interface.
- Multi-character keywords such as `%%` and `~~` are declared in the maps, but the active character-by-character scan does not yet combine them into single tokens.
- The later compiler stages are documented, not implemented.

## Documentation

- Project deep dive: [working.md](working.md)
- Contribution guide: [contribution.md](contribution.md)
- Alternate contribution entry point: [CONTRIBUTING.md](CONTRIBUTING.md)
- Lexer notes: [lexer/readme.md](lexer/readme.md)
- Syntaxer notes: [syntaxer/readme.md](syntaxer/readme.md)
- Semanter notes: [semanter/readme.md](semanter/readme.md)
- Optimizer notes: [optimizers/readme.md](optimizers/readme.md)
- Generator notes: [generators/readme.md](generators/readme.md)

## Where To Contribute Next

The most useful next steps are:

1. Convert the lexer's printed output into a real token stream API.
2. Implement the syntaxer around those tokens and define the first AST nodes.
3. Add semantic checks for identifiers, scope, and value categories.
4. Emit C from a validated AST.

The contribution workflow and coding expectations are documented in [contribution.md](contribution.md).
