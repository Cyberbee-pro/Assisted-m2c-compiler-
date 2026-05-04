# Contributing To M2C Compiler

This guide reflects the current modular compiler, not the earlier lexer-only prototype.

## Current Project State

M2C is organized into these active areas:

1. `lexer/`
2. `syntaxer/`
3. `generators/`
4. `runtime/`
5. root orchestration in `main.cpp`

Implemented today:

- tokenization
- Morse string decoding
- parsing for a small statement subset
- pure assembly generation
- runtime-backed printing and arithmetic
- native executable output

Still planned:

- richer grammar
- full AST
- separate semantic analyzer
- optimizer phase

## Good Places To Contribute

High-value work now includes:

1. Extend the parser beyond simple binary expressions
2. Move declaration/type checks into `semanter/`
3. Add tests around invalid syntax and runtime behavior
4. Expand the assembly runtime carefully
5. Make temporary output filenames unique
6. Implement control flow for `%`, `%%`, `~`, `~~`, and `^`

## Repository Areas

| Area | Folder | Current role |
|---|---|---|
| Lexer | `lexer/` | File reading, tokenization, Morse decoding |
| Syntaxer | `syntaxer/` | Statement parsing and validation |
| Generators | `generators/` | GNU assembly emission |
| Runtime | `runtime/` | Reusable assembly library for I/O and math |
| Semanter | `semanter/` | Planned future semantic-analysis stage |
| Optimizers | `optimizers/` | Planned future optimization stage |
| Samples | `m2c_files/` | Manual validation inputs |

## Development Workflow

1. Read the relevant phase docs first.
2. Build from the repository root.
3. Run at least one sample input.
4. Update docs when behavior changes.
5. Call out limitations honestly.

## Build And Run

Build:

```bash
bash build.sh
```

Run:

```bash
bash run.sh m2c_files/test_math.cym2c demo_math
```

Or directly:

```bash
./m2c_bin m2c_files/test_math.cym2c -o demo_math
```

## Validation Expectations

There is not yet a formal automated test suite, so contributors should validate changes manually.

Useful checks:

- valid Morse string print
- valid numeric print
- valid arithmetic assignment
- missing semicolon error
- undeclared identifier error
- generated binary output

When you submit a change, mention:

1. which `.cym2c` file you used
2. what command you ran
3. what output or error you observed

## Documentation Expectations

Update docs whenever you change:

- CLI behavior
- supported syntax
- compiler phase responsibilities
- runtime API
- build or link commands
- sample workflows

At minimum, check whether your change also requires edits to:

- [readme.md](readme.md)
- [working.md](working.md)
- the relevant phase README

## Coding Guidelines

- Keep changes scoped and readable.
- Preserve phase boundaries where possible.
- Prefer reusable helpers over duplicating logic across phases.
- Keep assembly runtime code small, explicit, and easy to audit.
- Do not let docs drift from the actual implementation.

## Good First Tasks

- add unique temporary assembly filenames
- expand parser support for nested expressions
- implement a first semantic-analysis pass in `semanter/`
- add runtime helpers for comparisons or control flow
- add more sample `.cym2c` programs
- clean legacy artifacts that no longer match the active design

## Pull Request Checklist

- code builds with `bash build.sh`
- at least one sample input was run
- docs were updated if behavior changed
- limitations are still documented clearly
- new runtime or assembly behavior was verified manually
