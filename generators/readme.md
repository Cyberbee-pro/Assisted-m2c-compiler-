# Generators

The generator phase turns parsed statements into pure GNU assembly.

It is the stage that bridges high-level parsed intent to the low-level runtime API.

## Files

- [include/code_generator.h](include/code_generator.h)
- [source/code_generator.cpp](source/code_generator.cpp)

## Input And Output

### Input

```cpp
const std::vector<Statement> &
```

### Output

```text
temp.s
```

## Generator Pipeline

```text
std::vector<Statement>
  -> buildVariableOffsets(...)
  -> assign string labels
  -> emit .rodata
  -> emit .text
  -> emit main prologue
  -> emit statement assembly
  -> emit epilogue
  -> temp.s
```

## Internal Data Nodes

### `OffsetMap`

```cpp
using OffsetMap = std::unordered_map<std::string, int>;
```

Maps variable names to stack offsets.

Example:

```text
x -> 4
y -> 8
z -> 12
```

### String Label Table

Each string print is assigned a label like:

- `str0`
- `str1`
- `str2`

These labels live in `.rodata`.

## Generated Assembly Shape

The generated file has this structure:

1. `.section .rodata`
2. string constants
3. `.text`
4. `.globl main`
5. stack-frame setup
6. statement code
7. `movl $0, %eax`
8. `leave`
9. `ret`

## Stack Model

The generator uses a simple frame-pointer-based local-variable layout:

```asm
-4(%rbp)
-8(%rbp)
-12(%rbp)
```

This makes variable addressing deterministic and easy to document.

## Runtime Call Boundary

The generator does not directly emit libc calls for normal program logic anymore.

Instead it emits runtime calls.

### String Output

```asm
leaq str0(%rip), %rdi
call m2c_print
```

### Integer Output

```asm
movl -4(%rbp), %edi
call m2c_print_int
```

### Arithmetic

```asm
movl -4(%rbp), %edi
movl $2, %esi
call m2c_mul
movl %eax, -8(%rbp)
```

## Important Helper Functions

### `escapeAssemblyString(...)`

Escapes string payloads for `.string` emission.

### `buildVariableOffsets(...)`

Assigns each declared variable a stable stack slot.

### `stackSizeForVariableCount(...)`

Computes aligned stack-frame allocation size.

### `memoryOperand(...)`

Formats stack-slot references like:

```asm
-4(%rbp)
```

### `emitLoadOperandToRegister(...)`

Loads:

- integer literals
- stack-stored identifiers

into a chosen 32-bit register.

### `emitStoreFromEax(...)`

Writes arithmetic or assignment results into the target stack slot.

### `emitPrintValue(...)`

Emits the runtime integer print path.

### `emitArithmeticCall(...)`

Loads `%edi` / `%esi`, calls the correct runtime math function, and stores `%eax`.

## Current Limits

- fixed temporary output file name
- no optimizer pass before generation
- no control-flow code generation yet
- no register allocation beyond simple helper emission
