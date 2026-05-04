# Lexer

The lexer is the first real transformation stage in the compiler.

Its job is to convert file text into a typed token stream that later phases can consume deterministically.

## Files

- [include/lexer.h](include/lexer.h)
- [include/token_types.h](include/token_types.h)
- [include/morse.h](include/morse.h)
- [include/excptsextra.h](include/excptsextra.h)
- [source/lexer.cpp](source/lexer.cpp)
- [source/morse.cpp](source/morse.cpp)
- [source/excptsextra.cpp](source/excptsextra.cpp)

## Input And Output

### Input

```cpp
std::string fileName
```

### Output

```cpp
std::vector<Token>
```

## Core Token Node

```cpp
struct Token
{
    TokenType type;
    std::string value;
    int line;
};
```

This is the lexer's data output node.

## Token Categories

The current lexer can produce:

- `MainMarker`
- `LoopMarker`
- `ConditionalMarker`
- `ExitMarker`
- `LetKeyword`
- `PrintStart`
- `PrintEnd`
- `Separator`
- `BlockStart`
- `BlockEnd`
- `OpenParenthesis`
- `CloseParenthesis`
- `MorseString`
- `Identifier`
- `Number`
- `Assignment`
- `ArithmeticOperator`
- `Comment`

## Pipeline Inside The Lexer

The main call is:

```cpp
Lexer::lex(const std::string &fileName)
```

Its internal pipeline is:

```text
filename
  -> std::ifstream
  -> line-by-line read loop
  -> tokenizeLine(...)
  -> appendBufferToken(...)
  -> std::vector<Token>
```

## Important Helper Functions

### `trim(...)`

Used to normalize line-ending validation logic.

### `isIdentifierToken(...)`

Checks whether a buffered token matches:

- leading alpha or underscore
- remaining alnum or underscore

### `isNumberToken(...)`

Checks whether a buffered token is an integer literal.

### `nextNonWhitespaceIndex(...)` and `previousNonWhitespaceIndex(...)`

These help distinguish:

- `/` as a main marker
- `/` as a division operator

### `isDivisionOperator(...)`

Implements that context-sensitive `/` logic.

### `appendBufferToken(...)`

Classifies the buffered text as:

- `let`
- number
- identifier

or throws `CompileError`.

### `validateLineEnding(...)`

Rejects non-empty, non-comment lines that do not end in:

- `;`
- `{`
- `}`

## Morse Subsystem

The lexer delegates quoted Morse decoding to [source/morse.cpp](source/morse.cpp).

### Maps

- `charMap`
- `numericMap`

### Runtime State

The Morse parser currently uses shared namespace state:

- `translatedToken`
- `buffer`
- `wrod`

### Morse Pipeline

```text
quoted Morse text
  -> morse_parse(...)
  -> letter-by-letter decode
  -> word assembly
  -> translated std::string
  -> TokenType::MorseString
```

Example:

```text
".... . .-.. .-.. ---"
```

becomes:

```text
HELLO
```

## Error Behavior

The lexer can throw:

- invalid token errors
- malformed line-ending errors
- file-open failures

These flow upward to root `main.cpp`, which prints them and exits non-zero.

## Current Limits

- column numbers are not tracked yet
- Morse decoding is focused on quoted strings
- tokenization is still intentionally narrow for the current grammar
