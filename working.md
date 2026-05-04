# Working Of The M2C Compiler

This document is the full codebase walkthrough for the current repository.

It explains:

1. What each folder contains
2. What data structures move through the compiler
3. How input enters the system
4. How each pipeline stage transforms that input
5. How the generated assembly is linked with the runtime
6. What the current limits and future extension points are

## 1. Codebase Identity

M2C is a Morse-flavored compiler for `.cym2c` source files.

The current implementation is not yet a full traditional compiler with AST, semantic-analysis pass, and optimizer pass. Instead, it is a small modular compiler with a working end-to-end pipeline that looks like this:

```text
User CLI input
  -> root main.cpp
  -> Lexer
  -> std::vector<Token>
  -> Parser
  -> std::vector<Statement>
  -> CodeGenerator
  -> temp.s
  -> runtime/io.s + runtime/math.s
  -> gcc -no-pie
  -> native executable
```

That pipeline is real and runnable today.

## 2. Repository Structure

The active structure is:

```text
m2c compiler/
├── build.sh
├── run.sh
├── main.cpp
├── readme.md
├── working.md
├── contribution.md
├── CONTRIBUTING.md
├── runtime/
│   ├── io.s
│   └── math.s
├── lexer/
│   ├── include/
│   │   ├── excptsextra.h
│   │   ├── lexer.h
│   │   ├── morse.h
│   │   ├── tokenMaker.h
│   │   ├── token_types.h
│   │   └── tokens.h
│   ├── source/
│   │   ├── excptsextra.cpp
│   │   ├── lexer.cpp
│   │   ├── morse.cpp
│   │   └── tokenMaker.cpp
│   ├── compiled/
│   └── readme.md
├── syntaxer/
│   ├── include/
│   │   └── parser.h
│   ├── source/
│   │   └── parser.cpp
│   └── readme.md
├── generators/
│   ├── include/
│   │   └── code_generator.h
│   ├── source/
│   │   └── code_generator.cpp
│   └── readme.md
├── semanter/
│   └── readme.md
├── optimizers/
│   └── readme.md
└── m2c_files/
    ├── test.cym2c
    ├── test2.cym2c
    ├── test_math.cym2c
    └── stress_test.cym2c
```

## 3. Responsibility Split

Each major area has a specific job:

### 3.1 Root Level

- `main.cpp`
  - parses CLI arguments
  - wires phases together
  - invokes the system toolchain
- `build.sh`
  - compiles the compiler itself
- `run.sh`
  - convenience wrapper for running the compiler

### 3.2 `lexer/`

- turns source text into `Token` objects
- decodes Morse inside quoted strings
- performs first-level lexical validation

### 3.3 `syntaxer/`

- turns tokens into a small structured IR
- validates print and assignment syntax
- performs simple declaration-before-use checks

### 3.4 `generators/`

- turns parsed statements into pure GNU assembly
- assigns stack slots for variables
- emits calls into the runtime assembly library

### 3.5 `runtime/`

- reusable hand-written assembly routines
- isolates low-level I/O and arithmetic from the generated `temp.s`

### 3.6 `semanter/` and `optimizers/`

- currently documentation/planning only
- reserved for future semantic analysis and optimization passes

## 4. All Data Nodes In The Pipeline

The current pipeline moves through a small set of core data structures.

### 4.1 CLI Input Node

Defined inside root `main.cpp`:

```cpp
struct CliOptions
{
    std::string inputFileName;
    std::string outputBinaryName;
};
```

This is the first structured data node in the pipeline.

It is produced from:

```text
m2c <input.cym2c> -o <output_binary>
```

### 4.2 Token Node

Defined in [lexer/include/token_types.h](lexer/include/token_types.h).

```cpp
struct Token
{
    TokenType type;
    std::string value;
    int line;
};
```

This is the lexer's output node.

It represents one classified unit of source code.

### 4.3 Operand Node

Defined in [syntaxer/include/parser.h](syntaxer/include/parser.h).

```cpp
struct Operand
{
    TokenType type;
    std::string value;
    int line;
};
```

This is the parser’s value-level node for:

- integer literals
- variable references

### 4.4 Statement Node

Also defined in [syntaxer/include/parser.h](syntaxer/include/parser.h).

```cpp
struct Statement
{
    StatementType type;
    std::string target;
    Operand firstOperand;
    Operand secondOperand;
    std::string operatorSymbol;
    std::string text;
    int line;
};
```

This is the generator’s main input node.

The compiler does not yet use a full AST class hierarchy, so `Statement` is the current “IR node” for supported language constructs.

### 4.5 Runtime Call Nodes

The generator does not represent runtime calls as C++ classes, but these are still key execution nodes in the final machine-code pipeline:

- `m2c_print`
- `m2c_print_int`
- `m2c_add`
- `m2c_sub`
- `m2c_mul`
- `m2c_div`

These act like stable low-level service nodes that generated programs call at runtime.

## 5. Stage 0: Build Pipeline

Before the compiler can compile user code, it must be built.

### 5.1 `build.sh`

[build.sh](build.sh) compiles:

- `main.cpp`
- every `.cpp` in `lexer/source/`
- every `.cpp` in `syntaxer/source/`
- every `.cpp` in `generators/source/`

and adds include paths for:

- `lexer/include/`
- `syntaxer/include/`
- `generators/include/`

Result:

```text
./m2c_bin
```

### 5.2 `run.sh`

[run.sh](run.sh) is a convenience wrapper:

```bash
bash run.sh m2c_files/test_math.cym2c demo_math
```

This becomes:

```text
./m2c_bin m2c_files/test_math.cym2c -o demo_math
```

## 6. Stage 1: Root Orchestrator

The root pipeline begins in [main.cpp](main.cpp).

### 6.1 Input

Raw process arguments:

```text
argv[0] argv[1] argv[2] argv[3]
```

Expected user-facing format:

```text
m2c <input.cym2c> -o <output_binary>
```

### 6.2 Output

Two concrete outputs are produced:

1. `temp.s`
2. the requested executable name

### 6.3 Main Flow

The control flow in `main.cpp` is:

```cpp
Lexer lexer;
std::vector<Token> tokens = lexer.lex(...);

Parser parser;
std::vector<Statement> statements = parser.parse(tokens);

CodeGenerator generator;
generator.generate(statements, "temp.s");

std::system("gcc temp.s runtime/io.s runtime/math.s -no-pie -o output");
```

So root `main.cpp` is an orchestration node, not a stage that owns language rules.

## 7. Stage 2: Lexer

The lexer implementation lives in:

- [lexer/include/lexer.h](lexer/include/lexer.h)
- [lexer/source/lexer.cpp](lexer/source/lexer.cpp)

### 7.1 Lexer Input

Input type:

```text
std::string fileName
```

The lexer then opens the file with `std::ifstream`.

### 7.2 Lexer Output

Output type:

```cpp
std::vector<Token>
```

### 7.3 Lexer Internal Functions

Important helper functions:

- `lex(...)`
- `tokenizeLine(...)`
- `appendBufferToken(...)`
- `validateLineEnding(...)`
- `isIdentifierToken(...)`
- `isNumberToken(...)`
- `isDivisionOperator(...)`
- `trim(...)`

### 7.4 Lexing Loop

`Lexer::lex(...)`:

1. opens the file
2. reads one line at a time
3. calls `tokenizeLine(...)`
4. appends `Token` objects into one shared vector
5. returns that vector

That means the lexer is a pure producer stage in the pipeline.

### 7.5 Token Recognition Rules

The lexer currently recognizes:

- `//` comments
- `<` and `>` for print delimiters
- `{` and `}` for block markers
- `=` for assignment
- `+`, `-`, `*`, `/` for arithmetic
- `let` as the declaration keyword
- identifiers like `x`, `name`, `_tmp`
- integer literals like `5`, `42`, `15000`
- quoted Morse strings

### 7.6 Overloaded `/`

The character `/` has two meanings:

- main marker
- division operator

The lexer resolves this using local context:

- if `/` sits between valid expression operands, it is tokenized as `ArithmeticOperator`
- otherwise it becomes `MainMarker`

This is implemented in `isDivisionOperator(...)`.

### 7.7 Buffer-Based Token Assembly

The lexer accumulates ordinary characters into a `buffer`.

When it hits whitespace or punctuation, it flushes that buffer through `appendBufferToken(...)`.

That flush logic classifies the buffer as:

- `LetKeyword`
- `Number`
- `Identifier`

or throws `CompileError` if the buffer does not fit any valid lexical class.

### 7.8 Line Validation

Before tokenization, each line is checked by `validateLineEnding(...)`.

Ignoring comments, non-empty lines must end in:

- `;`
- `{`
- `}`

This catches a large class of malformed input early.

## 8. Morse Translation Subsystem

The Morse layer lives in:

- [lexer/include/morse.h](lexer/include/morse.h)
- [lexer/source/morse.cpp](lexer/source/morse.cpp)

### 8.1 Input

The lexer enters this subsystem when it sees a `"`.

It passes:

- the source line
- the current character index

### 8.2 Output

The subsystem returns a translated `std::string`.

That becomes the `value` field of a `TokenType::MorseString` token.

### 8.3 Internal Data

Morse maps:

- `charMap`
- `numericMap`

Temporary shared state:

- `morseVars::translatedToken`
- `morseVars::buffer`
- `morseVars::wrod`

### 8.4 Morse Parsing Behavior

The current behavior is:

- single spaces separate Morse letters
- double spaces separate words
- letters decode to uppercase Latin characters
- digits decode to ASCII digits

Example:

```text
".... . .-.. .-.. ---"
```

becomes:

```text
HELLO
```

That means the parser and generator never deal with raw Morse strings. They receive already translated text.

## 9. Stage 3: Parser

The parser lives in:

- [syntaxer/include/parser.h](syntaxer/include/parser.h)
- [syntaxer/source/parser.cpp](syntaxer/source/parser.cpp)

### 9.1 Parser Input

```cpp
const std::vector<Token> &
```

### 9.2 Parser Output

```cpp
std::vector<Statement>
```

### 9.3 Parser Internal State

The parser tracks:

- `tokens_`
- `currentIndex_`
- `declaredVariables_`

This means the parser currently carries both syntax state and a small amount of semantic state.

### 9.4 Parser Helper Functions

- `parse(...)`
- `parsePrintStatement()`
- `parseAssignmentStatement()`
- `parseOperand(...)`
- `expect(...)`
- `peek()`
- `advance()`
- `ensureIdentifierIsDeclared(...)`
- `buildLineSnippet(...)`

### 9.5 Top-Level Statement Dispatch

The parser walks token-by-token and decides what to do based on `TokenType`.

Ignored tokens:

- `MainMarker`
- `BlockStart`
- `BlockEnd`
- `Comment`
- `Separator`

Parsed tokens:

- `PrintStart`
- `LetKeyword`

Anything else at the statement level is treated as an unexpected token and becomes a `CompileError`.

### 9.6 Print Statement Node Flow

Input syntax:

```text
<".... . .-.. .-.. ---">;
<x>;
<42>;
```

Parser output:

- `PrintString` node when the value is a `MorseString`
- `PrintValue` node when the value is an identifier or number

Validation:

- `<` must open the statement
- value must exist
- `>` must close the statement
- `;` must terminate it

### 9.7 Assignment Node Flow

Input syntax:

```text
let x = 5;
let y = x;
let z = y + 3;
```

Parser output:

- `VariableAssignment`
- `ArithmeticAssignment`

Validation:

- `let` must appear first
- next token must be an identifier
- next token must be `=`
- right-hand side must be:
  - one operand
  - or operand + operator + operand
- statement must end with `;`

### 9.8 Declaration Check

The parser uses `declaredVariables_` to reject:

- printing undeclared identifiers
- assigning from undeclared identifiers
- arithmetic expressions that reference undeclared identifiers

This is not yet a full semantic-analysis phase, but it gives the current pipeline enough protection to generate meaningful code.

## 10. Stage 4: Generator

The generator lives in:

- [generators/include/code_generator.h](generators/include/code_generator.h)
- [generators/source/code_generator.cpp](generators/source/code_generator.cpp)

### 10.1 Generator Input

```cpp
const std::vector<Statement> &
```

### 10.2 Generator Output

```text
temp.s
```

### 10.3 Generator Internal Data Nodes

The generator builds:

- `OffsetMap`
- string-label table
- stack-size value

#### `OffsetMap`

Maps variable names to stack-frame offsets:

```text
x -> 4
y -> 8
z -> 12
```

This is how generated assembly knows where each local variable lives.

#### String Label Table

Each `PrintString` statement gets a label:

```text
str0
str1
str2
```

These labels are emitted into `.rodata`.

### 10.4 Generator Helper Functions

- `generate(...)`
- `escapeAssemblyString(...)`
- `buildVariableOffsets(...)`
- `stackSizeForVariableCount(...)`
- `memoryOperand(...)`
- `emitLoadOperandToRegister(...)`
- `emitStoreFromEax(...)`
- `emitPrintValue(...)`
- `emitArithmeticCall(...)`

### 10.5 Assembly File Shape

The generator emits:

1. `.section .rodata`
2. string constants
3. `.text`
4. `.globl main`
5. function prologue
6. statement-by-statement assembly
7. `movl $0, %eax`
8. `leave`
9. `ret`

### 10.6 Variable Storage Model

Variables are stored on the stack relative to `%rbp`.

Example memory operand:

```asm
-4(%rbp)
```

This means the generated assembly is using a simple stack-frame local-variable model.

### 10.7 Print Generation

#### String Print

For a `PrintString` statement:

1. allocate a string label in `.rodata`
2. load the label address into `%rdi`
3. `call m2c_print`

#### Value Print

For a `PrintValue` statement:

1. load the integer value into `%edi`
2. `call m2c_print_int`

### 10.8 Arithmetic Generation

For `ArithmeticAssignment`:

1. load first operand into `%edi`
2. load second operand into `%esi`
3. choose the runtime call based on operator
4. store `%eax` into the target variable stack slot

Operator mapping:

- `+` -> `m2c_add`
- `-` -> `m2c_sub`
- `*` -> `m2c_mul`
- `/` -> `m2c_div`

### 10.9 Assignment Without Arithmetic

For `VariableAssignment`:

1. load source operand into `%eax`
2. store `%eax` into the target slot

So simple assignment never needs a runtime call.

## 11. Stage 5: Runtime Assembly Library

The runtime lives in:

- [runtime/io.s](runtime/io.s)
- [runtime/math.s](runtime/math.s)

This is the lowest-level implementation layer owned by the repository.

### 11.1 `runtime/io.s`

Exports:

- `m2c_print`
- `m2c_print_int`

#### `m2c_print`

Input:

- `%rdi` = pointer to string

Behavior:

- calls `puts@PLT`

Output:

- printed string

#### `m2c_print_int`

Input:

- `%edi` = integer value

Behavior:

- moves `%edi` into `%esi`
- loads format string into `%rdi`
- clears `%eax` for varargs ABI rules
- calls `printf@PLT`

Output:

- printed integer followed by newline

### 11.2 `runtime/math.s`

Exports:

- `m2c_add`
- `m2c_sub`
- `m2c_mul`
- `m2c_div`

Input convention:

- `%edi` = left operand
- `%esi` = right operand

Output convention:

- `%eax` = result

#### `m2c_add`

```asm
movl %edi, %eax
addl %esi, %eax
ret
```

#### `m2c_sub`

```asm
movl %edi, %eax
subl %esi, %eax
ret
```

#### `m2c_mul`

```asm
movl %edi, %eax
imull %esi, %eax
ret
```

#### `m2c_div`

```asm
testl %esi, %esi
je .Ldiv_zero
movl %edi, %eax
cltd
idivl %esi
ret
```

If the divisor is zero, the runtime returns `0`.

That is the current safety behavior.

## 12. Stage 6: Toolchain Link Step

After `temp.s` is generated, root `main.cpp` builds a shell command:

```text
gcc temp.s runtime/io.s runtime/math.s -no-pie -o <output_binary>
```

This is where:

- generated assembly
- runtime assembly
- system C runtime / libc glue

come together into one executable.

This final system call is the bridge between compiler pipeline and operating-system toolchain.

## 13. Full I/O Pipeline

Here is the complete I/O story from start to finish.

### 13.1 Input Boundary

User provides:

- input filename
- output binary name

### 13.2 File I/O Boundary

The lexer opens the `.cym2c` file and reads raw text lines.

### 13.3 Structured Data Boundary

The lexer converts text into:

```cpp
std::vector<Token>
```

### 13.4 Parsed IR Boundary

The parser converts tokens into:

```cpp
std::vector<Statement>
```

### 13.5 Generated File Boundary

The generator converts statements into:

```text
temp.s
```

### 13.6 Link Boundary

The system toolchain combines:

- `temp.s`
- `runtime/io.s`
- `runtime/math.s`

into the final binary.

### 13.7 Runtime Output Boundary

When the final executable runs:

- `m2c_print` and `m2c_print_int` perform actual console output
- math helpers compute values and return them back into the generated program flow

## 14. Example End-to-End Trace

Take this source:

```text
/{
let x = 5 + 3;
let y = x * 2;
<x>;
<y>;
<".... . .-.. .-.. ---">;
}
```

### 14.1 Lexer Output

Conceptually:

```text
MainMarker("/")
BlockStart("{")
LetKeyword("let")
Identifier("x")
Assignment("=")
Number("5")
ArithmeticOperator("+")
Number("3")
Separator(";")
...
MorseString("HELLO")
...
```

### 14.2 Parser Output

Conceptually:

```text
ArithmeticAssignment(target="x", first=5, op="+", second=3)
ArithmeticAssignment(target="y", first=x, op="*", second=2)
PrintValue(x)
PrintValue(y)
PrintString("HELLO")
```

### 14.3 Generator Output

Conceptually:

```asm
movl $5, %edi
movl $3, %esi
call m2c_add
movl %eax, -4(%rbp)
...
call m2c_print_int
...
call m2c_print
```

### 14.4 Runtime Output

At program execution time, the final binary prints:

```text
8
16
HELLO
```

## 15. Error Pipeline

Errors can emerge at multiple levels.

### 15.1 CLI Errors

Handled in `main.cpp`:

- invalid argument count
- missing `-o`

### 15.2 File I/O Errors

Handled in `Lexer::lex(...)`:

- file could not be opened

### 15.3 Lexical Errors

Handled in `lexer.cpp`:

- invalid token buffer
- malformed line endings

### 15.4 Parse Errors

Handled in `parser.cpp`:

- malformed print statement
- malformed assignment
- undeclared identifier use
- unexpected token sequence

### 15.5 Code Generation / Link Errors

Handled in:

- `CodeGenerator::generate(...)`
- root `main.cpp`

Examples:

- cannot open output `.s` file
- `gcc` returns non-zero

## 16. Current Limits

The codebase is modular and real, but still intentionally small.

Important limits:

- no full AST class hierarchy yet
- no separate semantic-analysis pass yet
- no optimizer implementation yet
- no nested expression grammar
- arithmetic limited to one binary operator per assignment
- control-flow markers are not fully parsed into executable control flow yet
- `temp.s` is shared across runs, so parallel invocations can race
- Morse decoding currently targets quoted strings, not a full Morse-everywhere language

## 17. Real Extension Points

The codebase is already structured so future work can plug into clear boundaries.

### 17.1 Lexer Extension Points

- add column tracking
- add richer token categories
- support more literal types

### 17.2 Parser Extension Points

- replace `std::vector<Statement>` with a full AST
- add precedence-aware expression parsing
- parse loops and conditionals

### 17.3 Semanter Extension Points

- move declaration checks out of parser
- add symbol tables
- add type rules

### 17.4 Optimizer Extension Points

- constant folding
- dead-code elimination
- runtime-call reduction

### 17.5 Generator / Runtime Extension Points

- unique temp file names
- more runtime helpers
- comparison helpers
- control-flow labels and branches

## 18. Summary

Today, the M2C codebase works as a modular pipeline with these concrete stage boundaries:

- raw CLI args
- `CliOptions`
- `std::vector<Token>`
- `std::vector<Statement>`
- `temp.s`
- runtime assembly calls
- final native executable

That is the most important mental model for understanding the current repository.
