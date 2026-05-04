# Contributing To M2C Compiler

This guide is for contributors working on the current modular compiler.

## Read These First

- [readme.md](readme.md)
- [working.md](working.md)
- the phase README for the area you are editing

## Current Architecture

The active codebase is organized around this data pipeline:

```text
CliOptions
  -> std::vector<Token>
  -> std::vector<Statement>
  -> temp.s
  -> runtime/io.s + runtime/math.s
  -> executable
```

When you modify any stage, think about:

1. what input node it consumes
2. what output node it produces
3. what invariants the next stage assumes

## Best Contribution Areas

### Lexer

- richer token metadata
- more precise diagnostics
- broader Morse coverage

### Syntaxer

- nested expressions
- control-flow parsing
- migration toward a real AST

### Generators

- control-flow assembly emission
- unique temp filenames
- cleaner stack layout management

### Runtime

- comparison helpers
- branching helpers
- safer division/reporting behavior

### Future Stages

- real semantic-analysis pass
- optimization pass

## Required Local Validation

Before treating a change as complete:

1. build the compiler
2. run at least one valid sample
3. if relevant, run one invalid sample and confirm the error path
4. update docs

Suggested commands:

```bash
bash build.sh
bash run.sh m2c_files/test_math.cym2c demo_math
./demo_math
```

## Documentation Expectations

If you change:

- syntax
- token shapes
- statement shapes
- runtime API
- build flow
- link flow

then update:

- [working.md](working.md)
- the relevant phase README
- [readme.md](readme.md) if the top-level user workflow changed

## Review Mindset

Useful contributions usually make one of these better:

- stage boundaries
- correctness
- diagnostics
- runtime reuse
- documentation accuracy

The current project benefits most from changes that reduce ambiguity between:

- what the code actually does
- what the docs say it does
