# Working Of The M2C Compiler

This document is the full technical walkthrough for the repository. It explains:

1. What the project is trying to become.
2. What is actually implemented right now.
3. How the current lexer executable works internally.
4. How the full five-stage compiler pipeline is intended to work.
5. Where the gaps, limitations, and next implementation steps are.

## 1. Big Picture

M2C stands for Morse-to-C. The target idea is:

```text
Morse-inspired source code (.cym2c)
  -> lexical analysis
  -> parsing / AST building
  -> semantic analysis
  -> optional optimization
  -> C code generation
  -> compile generated C with a normal C compiler
```

The repository is already organized around those five phases:

| Phase | Folder | Purpose | Current state |
|---|---|---|---|
| Lexer | `lexer/` | Read source and recognize tokens/Morse text | Implemented and runnable |
| Syntaxer | `syntaxer/` | Turn tokens into an AST | Design notes only |
| Semanter | `semanter/` | Validate meaning, types, and scope | Design notes only |
| Optimizers | `optimizers/` | Improve the validated program | Design notes only |
| Generators | `generators/` | Emit C code | Design notes only |

So the repository already contains the planned pipeline structure, but only the lexer is executable at the moment.

## 2. Repository Layout

```text
m2c compiler/
├── readme.md
├── working.md
├── contribution.md
├── CONTRIBUTING.md
├── m2c_files/
│   ├── test.cym2c
│   └── test2.cym2c
├── lexer/
│   ├── include/
│   │   ├── excptsextra.h
│   │   ├── morse.h
│   │   └── tokens.h
│   ├── source/
│   │   ├── excptsextra.cpp
│   │   ├── main.cpp
│   │   └── morse.cpp
│   ├── scripts/
│   │   └── build.sh
│   ├── compiled/
│   │   └── lexer
│   └── readme.md
├── syntaxer/
│   └── readme.md
├── semanter/
│   └── readme.md
├── optimizers/
│   └── readme.md
└── generators/
    └── readme.md
```

## 3. Two Pipelines To Keep In Mind

There are really two different pipelines to understand in this repo.

### 3.1 The Real Pipeline That Works Today

```text
.cym2c file
  -> fileReader opens file
  -> readFile() scans each line
  -> characters are classified as keyword / separator / parenthesis / quote / buffer text
  -> quoted Morse strings are translated to letters and digits
  -> diagnostics are printed to stdout
  -> end-of-line semicolon check may raise CompileError
```

This is the code path in the current executable.

### 3.2 The Intended Full Compiler Pipeline

```text
.cym2c file
  -> Lexer creates real Token objects
  -> Syntaxer creates AST nodes
  -> Semanter validates scope and types
  -> Optimizers simplify the AST
  -> Generators emit C source
  -> gcc/clang compiles the emitted C
```

This is the architecture described by the phase READMEs and by the project goal.

## 4. Current Executable: Entry Point And Control Flow

The current program starts in [lexer/source/main.cpp](lexer/source/main.cpp).

### 4.1 `main()`

`main()` does only three things:

1. Creates a `fileReader` object.
2. Hardcodes the input file path to `../../m2c_files/test.cym2c`.
3. Calls `readFile()`.

That means the default run path is tied to the sample file unless the code is changed or `openFile()` is wired in.

### 4.2 `fileReader`

`fileReader` is the main lexer driver class. It stores:

- `std::ifstream inputFile`
- `std::string fileName`
- `std::string readLine`
- `int i`
- `int error`
- `int Line`

Important behavior:

- The constructor opens the file immediately.
- If the file cannot be opened, the program prints an error and exits.
- The destructor closes the file and prints `destructor called!!`.

## 5. Current Source Input

The default sample file is [m2c_files/test.cym2c](m2c_files/test.cym2c):

```text
/{
    <".... . .-.. .-.. ---  .... . .-.. .-.. ---  .-- --- .-. .-.. -.. ">;
}


// exit(71);
```

This represents:

- `/` for the main marker
- `{` and `}` for the block
- `< ... >;` for a print-style statement
- Morse inside quotes that the current code translates to `HELLO HELLO WORLD`
- A trailing comment line beginning with `//`

## 6. Current Lexer Phase In Detail

The current implementation is not yet a clean token-stream API. Instead, it behaves like a scanner and logger. The logic sits mostly inside `fileReader::readFile()`.

### 6.1 Read Loop

`readFile()`:

1. Resets `Line` to `0`.
2. Reads the source file line by line using `std::getline`.
3. Iterates through each character in the current line.
4. Builds a local `buffer` string for ordinary characters.
5. Prints what it discovers as it goes.

### 6.2 Character Classification Rules

Inside the inner loop, the current character is matched in this order.

#### A. Comment Detection

If the current character is `/` and the next one is also `/`, the rest of the line is treated as a comment:

```cpp
if (current == '/' && readLine[i + 1] == '/')
```

The program prints:

```text
Comment : "// exit(71);"
```

and stops processing that line.

#### B. Separators

The code checks whether the current character exists in `M2C::separatorMap` from [lexer/include/tokens.h](lexer/include/tokens.h).

Current separator map:

```cpp
{
    {"{", "CURL_ST"},
    {"}", "CURL_END"},
    {",", "COMA"},
    {";", "SEMI_COLON"},
    {" ", "SPACE"}
}
```

When a separator is found:

- It prints the separator.
- If `buffer` is not empty, it prints the buffer too.
- Then it clears `buffer`.

Because the space character is listed as a separator, indentation spaces are printed as separators too.

#### C. Keywords

The code checks whether the current character exists in `M2C::keywordsMap`.

Current keyword map:

```cpp
{
    {"/",  "MAIN"},
    {"%",  "FOR"},
    {"%%", "WHILE"},
    {"~",  "IF"},
    {"~~", "ELSE"},
    {"<",  "PRINTST"},
    {">",  "PRINTEND"},
    {"^",  "EXIT"}
}
```

Important detail:

- The map includes multi-character entries such as `%%` and `~~`.
- The active scan checks one character at a time using `std::string(1, current)`.
- So today it can recognize `%` and `~` individually, but it does not yet combine them into `%%` or `~~` during scanning.

That is one of the biggest gaps between the declared token model and the implemented behavior.

#### D. Parentheses

Parentheses are handled separately:

- `(` prints `Open Parenthesis`
- `)` prints `Close Parenthesis`

The current `buffer` content is also printed around parentheses.

#### E. Quoted Strings

When `"` is found:

1. The index is advanced once.
2. The program prints `Found Quote!`
3. It calls `morse_parse(readLine, i)`.

This is the most compiler-like feature already working in the codebase.

#### F. Everything Else

Any other character is appended to `buffer` and printed as `Token : <char>`.

## 7. Morse Translation Subsystem

The Morse-specific code lives in [lexer/include/morse.h](lexer/include/morse.h) and [lexer/source/morse.cpp](lexer/source/morse.cpp).

### 7.1 Morse Maps

There are two main lookup maps.

#### Character Map

`charMap` translates Morse letters to uppercase Latin letters:

```cpp
{".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'}, {".", 'E'}, ...
```

#### Numeric Map

`numericMap` translates Morse digits:

```cpp
{"-----", '0'}, {".----", '1'}, {"..---", '2'}, {"...--", '3'}, {"....-", '4'},
{".....", '5'}, {"-....", '6'}, {"--...", '7'}, {"---..", '8'}, {"----.", '9'}
```

Because these maps store uppercase letters, translated text comes out uppercase.

### 7.2 Helper Functions

The current helpers are:

- `isChar(std::string buffer)`
- `isNum(std::string buffer)`
- `isMorse(std::string buffer)`
- `morseToChar(std::string buffer)`
- `morseToStr(std::string buffer)`
- `morseType(std::string buffer)`
- `morse_parse(std::string& readLine, int& i)`

What they do:

- `isChar` checks `charMap`.
- `isNum` checks `numericMap`.
- `isMorse` returns true if either of the above succeeds.
- `morseToChar` returns the decoded character or `?`.
- `morseToStr` returns a one-character `std::string` or an error string.
- `morseType` reports whether a Morse token is alphabetic, numeric, or unknown.

### 7.3 Shared Parse State

The header also defines a namespace:

```cpp
namespace morseVars {
    static inline std::string translatedToken = "String Not initialized";
    static inline std::string buffer = "";
    static inline std::string wrod = "";
}
```

This is shared mutable state used by `morse_parse()`.

Important notes:

- `wrod` is intentionally spelled that way in the current code.
- The parse state is global to the translation helpers rather than being local to a lexer object.
- That makes the code simple, but it also makes reuse and testing harder.

### 7.4 How `morse_parse()` Works

`morse_parse()` walks the source line starting just after an opening quote.

The current logic is roughly:

1. If the next character is another quote, return the previously stored `translatedToken`.
2. Reset `translatedToken` to an empty string.
3. Iterate until the line ends or another quote is reached.
4. Accumulate Morse dots and dashes into `morseVars::buffer`.
5. On a space:
   - If `buffer` is valid Morse, decode it and append the letter/digit to `wrod`.
   - If the next character is another space or a closing quote, commit the current `wrod` into `translatedToken`, then append a space.
6. On a closing quote:
   - Finalize the last pending Morse letter if needed.
   - Return the translated string.

The double-space rule is significant:

- A single space separates Morse letters.
- A double space separates words.

That is why:

```text
.... . .-.. .-.. ---  .-- --- .-. .-.. -..
```

becomes:

```text
HELLO WORLD
```

### 7.5 Output Example

For the sample input in `test.cym2c`, the quoted Morse string is translated to:

```text
HELLO HELLO WORLD
```

This exact behavior was verified by running the current executable.

## 8. Error Handling

Custom compile-time errors are defined in:

- [lexer/include/excptsextra.h](lexer/include/excptsextra.h)
- [lexer/source/excptsextra.cpp](lexer/source/excptsextra.cpp)

### 8.1 `CompileError`

`CompileError` stores:

- message
- source line
- line number
- formatted combined error text

The formatted message looks like:

```text
Compile Error : <message> At Line (<n>) : <source line>
```

### 8.2 Current Validation Rule

At the end of each processed line, `readFile()` performs a simple line-ending validation:

- If the loop reached the line end
- and `buffer` is still non-empty
- and the last relevant character was not `;`
- and the line is not considered properly closed

then a `CompileError` is thrown and caught locally.

This is a lightweight semicolon-style guard, not a full grammar check.

## 9. Build And Run Pipeline

The build script is [lexer/scripts/build.sh](lexer/scripts/build.sh).

Its command is:

```bash
g++ ../source/main.cpp ../source/excptsextra.cpp ../source/morse.cpp -I ../include -o ../compiled/lexer -Wall -Wextra
```

Then it changes into `../compiled/` and runs `./lexer`.

### 9.1 Important Path Detail

This script assumes the current working directory is `lexer/scripts`.

If you run:

```bash
bash lexer/scripts/build.sh
```

from the repository root, the relative paths break. The script works correctly when launched like this:

```bash
cd lexer/scripts
bash build.sh
```

### 9.2 Equivalent Root-Level Build Command

From the repository root, you can compile directly with:

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

## 10. Real Console Output Today

Running the current executable against `m2c_files/test.cym2c` prints output like:

```text
Keyword : "/"
Seperator : " { "
Keyword : "<"
Found Quote!
morse Translation? HELLO HELLO WORLD
Seperator : " } "
Comment : "// exit(71);"
destructor called!!
```

This matters because it shows the current phase boundary:

- The program is reading and classifying source text successfully.
- It is not producing a reusable token list for later compiler stages yet.
- It is not emitting C.

## 11. Intended Full Compiler Pipeline In Detail

The rest of the repository documents a classic compiler architecture. This section ties those pieces together into one complete flow.

### 11.1 Phase 1: Lexer

Expected long-term input:

- raw `.cym2c` source code

Expected long-term output:

- a `std::vector<Token>`

Expected responsibilities:

- recognize delimiters, keywords, operators, identifiers, literals
- decode Morse strings and possibly Morse-based identifiers/numbers
- track line and column positions
- report lexical errors

Current gap:

- `Token` exists in `main.cpp`, but the current code prints discoveries instead of constructing a token stream.

### 11.2 Phase 2: Syntaxer

Planned input:

- token stream from the lexer

Planned output:

- AST rooted at a `Program` node

Planned responsibilities:

- validate statement order and nesting
- parse constructs such as if, while, for, blocks, assignments, and print statements
- preserve source positions for good error messages

The design notes in `syntaxer/readme.md` describe a recursive-descent parser approach and sample AST node shapes.

### 11.3 Phase 3: Semanter

Planned input:

- AST from the syntaxer

Planned output:

- validated and annotated AST

Planned responsibilities:

- symbol table creation
- declaration and use checks
- type checks
- scope checks
- reporting semantic errors

The `semanter/readme.md` notes suggest a visitor-style traversal and a scope stack.

### 11.4 Phase 4: Optimizers

Planned input:

- semantically valid AST

Planned output:

- optimized AST with the same meaning

Planned responsibilities:

- constant folding
- dead-code elimination
- algebraic simplification
- loop simplification
- common subexpression elimination

This phase is optional by design.

### 11.5 Phase 5: Generators

Planned input:

- AST from the optimizer, or directly from the semanter if optimization is skipped

Planned output:

- valid C source code

Planned responsibilities:

- emit headers such as `#include <stdio.h>`
- declare variables
- convert print constructs into `printf(...)`
- emit control flow in valid C syntax
- generate `main()` and return statements

The generator documentation already sketches an output-oriented class around a `std::stringstream`.

## 12. Language Metadata In The Repo

The repo already defines the basic token vocabulary in [lexer/include/tokens.h](lexer/include/tokens.h).

### 12.1 Keywords

| Raw token | Meaning |
|---|---|
| `/` | `MAIN` |
| `%` | `FOR` |
| `%%` | `WHILE` |
| `~` | `IF` |
| `~~` | `ELSE` |
| `<` | `PRINTST` |
| `>` | `PRINTEND` |
| `^` | `EXIT` |

### 12.2 Separators

| Raw token | Meaning |
|---|---|
| `{` | `CURL_ST` |
| `}` | `CURL_END` |
| `,` | `COMA` |
| `;` | `SEMI_COLON` |
| ` ` | `SPACE` |

### 12.3 Category Map

`categoryMap` groups keywords and separators under broad classes like `KEYWORD` and `SEPARATOR`. This is useful groundwork for a real tokenization API.

## 13. Limitations In The Current Implementation

Nothing should be hidden here, so this section is explicit.

### 13.1 What Works

- Opening and reading `.cym2c` files
- Scanning line by line
- Recognizing comments
- Recognizing single-character separators and keywords
- Handling parentheses separately
- Translating Morse inside quotes
- Raising a simple compile error for some malformed line endings

### 13.2 What Does Not Exist Yet

- no parser
- no AST
- no semantic analyzer
- no optimizer
- no C generator
- no generated `.c` file output
- no integration with `gcc` or `clang` for final binaries

### 13.3 Implementation Gaps Inside The Lexer

- `Token` is declared but not populated into a vector for downstream phases.
- Multi-character operators such as `%%` and `~~` are defined in the map but not lexed as combined tokens.
- Input is hardcoded in `main()`.
- The scanner mixes lexing, printing, and validation in a single routine.
- Morse parsing depends on global namespace state.
- Morse translation currently yields uppercase output only.
- Morse decoding is only used inside quoted strings, not yet as a full identifier/number token strategy across the language.
- Space characters are emitted as separators, which creates noisy output during indentation.
- The line-ending validation is heuristic and not a real grammar check.

## 14. Recommended Next Engineering Steps

If the goal is to turn this repo into a full compiler, the cleanest order is:

1. Move the `Token` struct into a shared header and add line/column metadata.
2. Refactor `readFile()` so it builds and returns `std::vector<Token>`.
3. Add proper handling for multi-character tokens such as `%%`, `~~`, `==`, `\<`, and `\>`.
4. Separate Morse decoding from console printing.
5. Implement the syntaxer as the first consumer of the token stream.
6. Define a minimal AST for blocks, print statements, literals, identifiers, assignments, and control flow.
7. Add semantic checks only after the parser is stable.
8. Generate a small subset of C first, then grow coverage.

## 15. Example Of The Future Full Flow

A plausible finished flow would look like this:

```text
Source (.cym2c)
    /{
        <".... ..">;
    }

Lexer output
    MAIN
    CURL_ST
    PRINTST
    STRING("HI")
    PRINTEND
    SEMI_COLON
    CURL_END

Syntaxer output
    Program
      Block
        PrintStatement("HI")

Semanter output
    Program marked valid

Generator output
    #include <stdio.h>
    int main() {
        printf("HI");
        return 0;
    }
```

That is the architectural direction the existing folder layout already suggests.

## 16. Final Summary

Today, M2C is best understood as:

- a working lexer prototype
- a Morse string translator
- a repository scaffold for a full compiler
- a project with clear next steps, but not yet a finished end-to-end translator

For day-to-day contributor guidance, use [contribution.md](contribution.md). For the short project overview, use [readme.md](readme.md).
