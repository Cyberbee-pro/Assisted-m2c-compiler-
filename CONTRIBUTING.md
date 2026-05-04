# Contributing To M2C Compiler

This file mirrors [contribution.md](contribution.md) so contributors can use either entry point.

## Project Reality First

M2C is organized as a five-stage compiler:

1. Lexer
2. Syntaxer
3. Semanter
4. Optimizers
5. Generators

Only the lexer is implemented as runnable code right now. The remaining stages are documented targets.

Start here:

- [readme.md](readme.md)
- [working.md](working.md)

## Best Places To Contribute

The most useful work right now is:

1. Turn the lexer into a real token-stream producer.
2. Add support for multi-character tokens such as `%%` and `~~`.
3. Move shared token structures into headers.
4. Start a minimal syntaxer implementation.
5. Add more sample programs and validation steps.

## Build And Run

Run the existing script from `lexer/scripts`:

```bash
cd lexer/scripts
bash build.sh
```

Or compile from the repository root:

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

## Contribution Rules

- Keep changes scoped and readable.
- Update docs when behavior changes.
- Validate the sample `.cym2c` input after code changes.
- Be explicit about current limitations.

## Good First Tasks

- collect tokens instead of printing only
- add token position data
- support `%%` and `~~`
- make input filename configurable
- add more example source files

## Pull Request Checklist

- code builds
- sample input runs
- docs were updated if needed
- limitations are documented honestly

The repository will benefit most from changes that help the lexer feed a future parser cleanly.
