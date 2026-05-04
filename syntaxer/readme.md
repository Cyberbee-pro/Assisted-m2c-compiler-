# Syntaxer

The syntaxer is the parser stage of the current compiler.

It converts the lexer's raw token stream into a small structured intermediate representation that the generator can consume directly.

## Files

- [include/parser.h](include/parser.h)
- [source/parser.cpp](source/parser.cpp)

## Input And Output

### Input

```cpp
const std::vector<Token> &
```

### Output

```cpp
std::vector<Statement>
```

This is the parser’s main pipeline boundary.

## Parser Data Nodes

### `Operand`

```cpp
struct Operand
{
    TokenType type;
    std::string value;
    int line;
};
```

Used for:

- integer literals
- identifier references

### `Statement`

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

This is the parser’s output node.

## Supported Statement Types

- `PrintString`
- `PrintValue`
- `VariableAssignment`
- `ArithmeticAssignment`

## Parser Pipeline

```text
std::vector<Token>
  -> Parser::parse(...)
  -> token dispatch loop
  -> parsePrintStatement() / parseAssignmentStatement()
  -> std::vector<Statement>
```

## Internal Parser State

The parser tracks:

- `tokens_`
- `currentIndex_`
- `declaredVariables_`

That last set means the parser currently performs some semantic validation in addition to syntax validation.

## Parsing Rules

### Print Statements

Accepted forms:

```text
<".... . .-.. .-.. ---">;
<x>;
<42>;
```

Validation:

- statement must start with `<`
- next token must be a translated string, identifier, or number
- `>` must follow
- `;` must terminate the statement

Output:

- `PrintString` if the payload is a `MorseString`
- `PrintValue` if the payload is an identifier or number

### Assignment Statements

Accepted forms:

```text
let x = 5;
let y = x;
let z = y + 3;
```

Validation:

- `let` must begin the statement
- next token must be an identifier
- next token must be `=`
- right-hand side must be:
  - one operand
  - or operand operator operand
- statement must end with `;`

Output:

- `VariableAssignment`
- `ArithmeticAssignment`

## Helper Functions

### `expect(...)`

Consumes and validates an exact token type.

### `peek()`

Reads the current token without consuming it.

### `advance()`

Moves the parser forward by one token.

### `parseOperand(...)`

Builds an `Operand` from either:

- `Identifier`
- `Number`

### `ensureIdentifierIsDeclared(...)`

Rejects identifier use before declaration.

### `buildLineSnippet(...)`

Reconstructs a readable line fragment for `CompileError` messages.

## Error Behavior

The parser throws `CompileError` for:

- malformed print syntax
- malformed assignment syntax
- unexpected top-level tokens
- undeclared identifier use

## Current Limits

- no full AST hierarchy yet
- no nested expression grammar
- no operator precedence handling
- no control-flow parsing yet
- semantic checks are only partial
