# Lexer

The lexer is the first active compiler phase.

Its implementation lives in:

- [include/lexer.h](include/lexer.h)
- [include/token_types.h](include/token_types.h)
- [include/morse.h](include/morse.h)
- [source/lexer.cpp](source/lexer.cpp)
- [source/morse.cpp](source/morse.cpp)

## Responsibilities

- open a `.cym2c` file
- scan line by line
- tokenize the source into `std::vector<Token>`
- decode Morse strings inside quotes
- attach line numbers to tokens
- reject obviously malformed line endings

## Output

The lexer returns:

```cpp
std::vector<Token>
```

It no longer prints token diagnostics as its main job.

## Tokens Currently Produced

- `MainMarker`
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

## Supported Lexical Patterns

- `/` as main marker or division operator
- `{` and `}`
- `//` comments
- `<` and `>`
- `=`
- `+`, `-`, `*`, `/`
- `let`
- integer literals
- identifiers
- Morse strings inside quotes

## Morse Handling

Quoted Morse text is decoded during lexing through [source/morse.cpp](source/morse.cpp).

Example:

```text
<".... . .-.. .-.. ---">;
```

becomes a `MorseString` token with value:

```text
HELLO
```

## Current Limits

- only quoted strings are Morse-decoded
- no column tracking yet
- control-flow markers are recognized lexically but not fully parsed downstream

## Next Logical Improvements

- add column metadata
- support richer token categories if the grammar grows
- isolate more validation out of the lexer if syntax rules become more complex
