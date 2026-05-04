# Contributing To M2C Compiler

This file mirrors [contribution.md](contribution.md).

## Project Reality

M2C is currently a modular compiler with:

- a lexer in `lexer/`
- a parser in `syntaxer/`
- an assembly generator in `generators/`
- an assembly runtime in `runtime/`
- a root orchestrator in `main.cpp`

It is no longer just a lexer prototype.

## Build

```bash
bash build.sh
```

## Run

```bash
bash run.sh m2c_files/test_math.cym2c demo_math
```

Or:

```bash
./m2c_bin m2c_files/test_math.cym2c -o demo_math
```

## Best Contribution Areas

- parser expansion
- semantic-analysis separation
- optimizer implementation
- runtime assembly growth
- better testing
- safer temporary-file handling

## Contribution Rules

- keep changes scoped
- validate behavior locally
- update docs when behavior changes
- document remaining limits honestly

## Before Opening A PR

- build the compiler
- run a sample input
- note what you tested
- update affected docs
