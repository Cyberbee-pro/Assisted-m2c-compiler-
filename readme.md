# M2C Compiler

M2C is a Morse-flavored compiler that reads `.cym2c` source, tokenizes it, validates a small language subset, generates GNU assembly, links that assembly with a small runtime library, and produces a native executable.

## Current Pipeline

```text
.cym2c source
  -> Lexer
  -> Parser
  -> GNU assembly generator
  -> runtime/io.s + runtime/math.s
  -> gcc -no-pie
  -> executable
```

## Repository Layout

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
│   └── source/
├── syntaxer/
│   ├── include/
│   └── source/
├── generators/
│   ├── include/
│   └── source/
├── m2c_files/
└── working.md
```

## What Works Today

- Root CLI: `m2c <input.cym2c> -o <output_binary>`
- Root build script: `build.sh`
- Root run script: `run.sh`
- Morse-string translation inside quoted print statements
- Tokenization into `std::vector<Token>`
- Parsing into a small statement IR
- Variable declarations and simple identifier-use validation
- Pure GNU assembly generation
- Runtime-backed printing and arithmetic

## Supported Language Subset

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

## Runtime Library

The compiler now links generated assembly with a small runtime written in assembly:

- [runtime/io.s](runtime/io.s)
  - `m2c_print`
  - `m2c_print_int`
- [runtime/math.s](runtime/math.s)
  - `m2c_add`
  - `m2c_sub`
  - `m2c_mul`
  - `m2c_div`

This keeps the generated output simpler and gives the repository a real assembly standard library.

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

The compiler writes `temp.s` and then links it with the runtime to produce the final binary.

## Working Examples

- [m2c_files/test.cym2c](m2c_files/test.cym2c)
- [m2c_files/test_math.cym2c](m2c_files/test_math.cym2c)

## Documentation

- Full technical walkthrough: [working.md](working.md)
- Contributor guide: [contribution.md](contribution.md)
- Alternate contributor entry point: [CONTRIBUTING.md](CONTRIBUTING.md)
- Lexer notes: [lexer/readme.md](lexer/readme.md)
- Parser notes: [syntaxer/readme.md](syntaxer/readme.md)
- Generator notes: [generators/readme.md](generators/readme.md)
- Semantic-analysis plan: [semanter/readme.md](semanter/readme.md)
- Optimizer plan: [optimizers/readme.md](optimizers/readme.md)
