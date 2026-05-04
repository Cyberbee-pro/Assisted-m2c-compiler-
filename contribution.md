# Contributing To M2C Compiler

This guide is for contributors who want to improve the M2C compiler in a way that matches the real state of the repository.

## Project Reality First

M2C is organized as a five-stage compiler:

1. Lexer
2. Syntaxer
3. Semanter
4. Optimizers
5. Generators

Right now, only the lexer stage is implemented as runnable code. The later stages exist as documented design targets in their phase `readme.md` files.

Before starting, read:

- [readme.md](readme.md) for the short project overview
- [working.md](working.md) for the full pipeline and implementation walkthrough

## Best Places To Contribute

The highest-value work at the moment is:

1. Improve the lexer so it returns a real token stream instead of printing diagnostics only.
2. Add support for multi-character tokens such as `%%` and `~~`.
3. Move shared token structures into a reusable header.
4. Start the syntaxer with a minimal parser for blocks, print statements, and literals.
5. Expand tests and sample `.cym2c` programs.
6. Tighten build instructions and developer workflow.

If you want a first contribution with low risk, start in `lexer/`.

## Repository Areas

| Area | Folder | Current state |
|---|---|---|
| Core working implementation | `lexer/` | Active code |
| Parser design | `syntaxer/` | Documentation only |
| Semantic analysis design | `semanter/` | Documentation only |
| Optimization design | `optimizers/` | Documentation only |
| Code generation design | `generators/` | Documentation only |
| Sample input files | `m2c_files/` | Active examples |

## Development Workflow

1. Read the relevant phase documentation before changing code.
2. Check the current behavior locally.
3. Make focused changes.
4. Re-run the build and sample input.
5. Update documentation when behavior changes.

## Building The Current Code

Run the existing build script from its own directory:

```bash
cd lexer/scripts
bash build.sh
```

Or build from the repository root:

```bash
g++ lexer/source/main.cpp lexer/source/excptsextra.cpp lexer/source/morse.cpp \
  -I lexer/include \
  -o lexer/compiled/lexer \
  -Wall -Wextra
```

Then run:

```bash
./lexer/compiled/lexer
```

## Coding Guidelines

- Keep changes small and understandable.
- Preserve the current folder split by compiler stage.
- Prefer shared headers for data that will be needed by later phases.
- Add comments only where the logic is genuinely non-obvious.
- Keep docs aligned with the actual implementation, not just the intended architecture.

## Documentation Expectations

Update docs when you change:

- build commands
- language syntax
- token shapes
- compiler phase responsibilities
- sample input/output behavior

At minimum, check whether your change should also update:

- [readme.md](readme.md)
- [working.md](working.md)
- the phase-specific `readme.md`

## Testing Expectations

There is not yet a formal automated test suite, so contributors should verify behavior by running the sample programs in `m2c_files/`.

When you submit a change:

1. State which file you used for validation.
2. Describe the expected output.
3. Mention any current limitations that still remain.

Helpful manual checks include:

- a valid print statement
- comments starting with `//`
- malformed lines missing a semicolon
- Morse strings with one word and multiple words
- keyword and separator recognition

## Good First Tasks

- Replace debug printing with `Token` collection.
- Add line and column metadata to tokens.
- Detect `%%` as a single while token.
- Detect `~~` as a single else token.
- Move `Token` into `lexer/include/`.
- Make the input filename configurable instead of hardcoded.
- Add more `.cym2c` examples.

## Pull Request Checklist

Before opening a PR, make sure:

- the code builds locally
- the sample program still runs
- the docs match the new behavior
- the change is scoped to a clear problem
- limitations or follow-up work are called out honestly

## Final Advice

The biggest contribution you can make right now is reducing the gap between:

- the documented five-stage compiler design
- and the current lexer-only implementation

Anything that makes the lexer reusable by a parser will move the entire project forward.
