# Generators

The generator phase converts parsed statements into pure GNU assembly.

Its implementation lives in:

- [include/code_generator.h](include/code_generator.h)
- [source/code_generator.cpp](source/code_generator.cpp)

## Current Output

The generator writes:

```text
temp.s
```

This file is then linked with the runtime assembly library by the root orchestrator.

## Input

The generator consumes:

```cpp
std::vector<Statement>
```

from the parser.

## What It Emits

- `.section .rodata`
- string labels for translated Morse literals
- `.text`
- `.globl main`
- stack-frame setup and teardown
- variable storage in the stack frame
- calls to runtime functions instead of fully open-coded I/O and math

## Runtime Calls Used

### I/O

- `m2c_print`
- `m2c_print_int`

### Math

- `m2c_add`
- `m2c_sub`
- `m2c_mul`
- `m2c_div`

These come from:

- [../runtime/io.s](../runtime/io.s)
- [../runtime/math.s](../runtime/math.s)

## Generation Strategy

### String Print

For:

```text
<".... . .-.. .-.. ---">;
```

the generator:

1. creates a string label in `.rodata`
2. loads its address into `%rdi`
3. emits `call m2c_print`

### Value Print

For:

```text
<x>;
```

the generator:

1. loads the value into `%edi`
2. emits `call m2c_print_int`

### Arithmetic

For:

```text
let y = x * 2;
```

the generator:

1. loads operands into `%edi` and `%esi`
2. emits `call m2c_mul`
3. stores `%eax` into the target stack slot

## Current Limits

- one shared output file name: `temp.s`
- no full register allocator
- no optimizer pass yet
- no branching/control-flow codegen yet

## Next Logical Improvements

- unique temp assembly filenames
- assembly output tests
- richer runtime helpers
- control-flow generation
- move from statement IR to AST-based codegen
