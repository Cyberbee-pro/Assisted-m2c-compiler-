# 🔤 Lexer - Phase 1: Tokenization

## What is the Lexer?

The **Lexer** (Lexical Analyzer) is the **first phase** of the M2C compiler. It reads raw morse code input and **converts it into a stream of tokens**—the building blocks that the next phase (Syntaxer) will use.

Think of it like this:
```
Raw morse input:         (% (.-;-----; .....) (<"----.---.....---">))
                         Note: Everything is morse! (i=.-, 0=-----, etc.)
                         ↓
    [LEXER processes morse → base characters]
                         ↓
Token stream:            ( % ( i ; 0 ; 5 ) ( < "done" > ) )
                         Decoded to standard notation for parser
```

## ⚠️ Key Insight: Complete Morse Encoding

**The lexer must handle morse-encoded numbers and identifiers too**, not just operators:
- **Morse Numbers**: `-----` (0), `.----` (1), `.---.` (9), etc.
- **Morse Identifiers**: Variable names like `....+.` represent a two-character variable
- **Morse Strings**: Every character inside `"..."` is morse-encoded
- **Morse Operators**: `~`, `%`, `;`, `<`, `>`, etc. (as before)

The lexer reads all of these, decodes them to standard characters, and produces standard tokens.

## Input & Output

### 📥 Input
- **Raw morse code string** (sequences of morse characters: dots `·`, dashes `-`, spaces)
- Example (with morse-encoded numbers & identifiers):
  ```
  (%(....; -----; .....) (<"ste">)})
  (i=...., 0=-----, 5=....., s=..., t=-, e=.)
  ```

### 📤 Output  
- **Token Stream** - A sequence of `Token` objects, each containing:
  - `type` - What kind of token (symbol, operator, number, etc.)
  - `value` - The actual text (decoded from morse)
  - `line` / `column` - Where it appeared (for error messages)

### Example Transformation

```
Input Morse:   ( % ( .... ; ----- ; ..... ) ( < ".-.-" > ) )
               (Morse-encoded: i=...., 0=-----, 5=....., "st"=.-+.)

Output Tokens (decoded):
  Token(LPAREN, "(", 1, 0)
  Token(OPERATOR, "%", 1, 2)
  Token(LPAREN, "(", 1, 4)
  Token(IDENTIFIER, "i", 1, 6)           ← decoded from .... 
  Token(SEMICOLON, ";", 1, 7)
  Token(NUMBER, "0", 1, 8)               ← decoded from -----
  Token(SEMICOLON, ";", 1, 9)
  Token(NUMBER, "5", 1, 10)              ← decoded from .....
  Token(RPAREN, ")", 1, 12)
  Token(LT_BRACKET, "<", 1, 13)
  Token(STRING, "st", 1, 14)             ← decoded from .-.+.
  Token(GT_BRACKET, ">", 1, 17)
  Token(RPAREN, ")", 1, 18)
  Token(EOF, "", 1, 19)
```

## Key Responsibilities

✅ **Morse Code Recognition**
- Know which morse patterns map to which characters
- Example: `.` = 'e', `...` = 's', `....` = 'i', `-----` = '0', `.---.` = '9'
- **Important**: Numbers and identifiers are also morse-encoded!

✅ **Tokenization**  
- Group morse sequences into meaningful tokens
- Recognize operators (`~`, `%`, `;`, `<`, etc.)
- Decode and identify morse-encoded numbers (e.g., `·---·` → 9)
- Decode and identify morse-encoded variable names (identifiers)
- Decode morse characters inside strings

✅ **Error Detection**
- Detect invalid morse sequences
- Catch unrecognized patterns
- Report errors with line/column numbers

✅ **Position Tracking**
- Remember line and column for each token
- Helps other phases report accurate errors

## Token Types

```cpp
enum TokenType {
    // Symbols & Delimiters
    LPAREN,              // (
    RPAREN,              // )
    LT_BRACKET,          // <
    GT_BRACKET,          // >
    
    // Operators
    OPERATOR,            // ~, %, etc.
    SEMICOLON,           // ;
    
    // Literals
    NUMBER,              // 0, 1, 10, etc.
    STRING,              // "hello"
    IDENTIFIER,          // variable names
    
    // Special
    EOF_TOKEN,
    ERROR
};
```

## Core Data Structures

### Token Structure

```cpp
struct Token {
    TokenType type;           // What is this?
    std::string value;        // The actual text
    int line;                 // Which line? (1-based)
    int column;               // Which column? (1-based)
    
    Token(TokenType t, std::string v, int l, int c)
        : type(t), value(v), line(l), column(c) {}
};
```

### Lexer Class Interface

```cpp
class Lexer {
public:
    explicit Lexer(const std::string& source);
    
    // Get next token
    Token nextToken();
    
    // Get all tokens at once  
    std::vector<Token> tokenize();
    
    // Error handling
    bool hasErrors() const;
    std::string getErrorMessage() const;
    
private:
    std::string source;       // Input morse code
    size_t current;           // Current position
    int line, column;         // For error reporting
    std::vector<Token> tokens;
};
```

## Implementation Guidelines

### 1. **Morse Code Mapping**

Define character mappings for morse sequences (letters & numbers):

```cpp
std::map<std::string, char> morseToChar = {
    // Letters
    {".-", 'a'},      {"-...", 'b'},    {"-.-.", 'c'},    {"-..", 'd'},
    {".", 'e'},       {"..-.", 'f'},    {"--.", 'g'},     {"....", 'h'},
    // ...
    
    // Numbers (morse uses 5-character patterns)
    {"-----", '0'},   {".----", '1'},   {"..---", '2'},   {"...--", '3'},
    {"....-", '4'},   {".....", '5'},   {"-....", '6'},   {"--...", '7'},
    {"---..", '8'},   {"----.", '9'},
};
```

**Note:** When tokenizing, recognize sequences of morse patterns separated by spaces or delimiters. For example:
- `....` → 'i' (identifier start)
- `-----` → '0' (number)
- `....` followed by `-` → could be 'i-' (two char identifier)

### 2. **Algorithm Overview**

```cpp
std::vector<Token> Lexer::tokenize() {
    tokens.clear();
    current = 0;
    
    while (current < source.length()) {
        skipWhitespace();
        if (current >= source.length()) break;
        
        Token next = nextToken();
        tokens.push_back(next);
    }
    
    tokens.push_back(Token(EOF_TOKEN, "", line, column));
    return tokens;
}
```

### 3. **Pattern Recognition**

```cpp
Token Lexer::nextToken() {
    char ch = peek();
    
    switch (ch) {
        case '(': return Token(LPAREN, "(", line, column);
        case ')': return Token(RPAREN, ")", line, column);
        case '<': return Token(LT_BRACKET, "<", line, column);
        case '>': return Token(GT_BRACKET, ">", line, column);
        case ';': return Token(SEMICOLON, ";", line, column);
        case '%': return Token(OPERATOR, "%", line, column);
        case '~': return Token(OPERATOR, "~", line, column);
        // ... more symbols ...
    }
    
    if (isdigit(ch)) return readNumber();
    if (isalpha(ch)) return readIdentifier();
    if (ch == '"') return readString();
    
    error("Unknown character");
    return Token(ERROR, std::string(1, ch), line, column);
}
```

### 4. **Error Handling**

```cpp
void Lexer::error(const std::string& message) {
    std::cerr << "Lexer Error at line " << line 
              << ", column " << column << ": " 
              << message << std::endl;
    errors.push_back(message);
}
```

Handle:
- Invalid morse sequences
- Unrecognized patterns
- Unexpected characters

## Interface / Main Entry Point

The lexer should expose key methods for token generation.

## Example: Tokenizing a For Loop

**Input Morse:**
```
( % ( i ; 0 ; 5 ) ( < "count" > ) )
```

**Tokenization Process:**

1. `(` → `Token(LPAREN, "(")`
2. `%` → `Token(OPERATOR, "%")`  
3. `(` → `Token(LPAREN, "(")`
4. `i` → `Token(IDENTIFIER, "i")`
5. `;` → `Token(SEMICOLON, ";")`
6. `0` → `Token(NUMBER, "0")`
7. `;` → `Token(SEMICOLON, ";")`
8. `5` → `Token(NUMBER, "5")`
9. `)` → `Token(RPAREN, ")")`
10. `(` → `Token(LPAREN, "(")`
11. `<` → `Token(LT_BRACKET, "<")`
12. `"count"` → `Token(STRING, "count")`
13. `>` → `Token(GT_BRACKET, ">")`
14. `)` → `Token(RPAREN, ")")`
15. `)` → `Token(RPAREN, ")")`
16. EOF → `Token(EOF_TOKEN, "")`

## Testing Your Lexer

✅ **Valid tokens:** Numbers, identifiers, operators, strings
✅ **Operators:** All morse operators (`~`, `%`, etc.)
✅ **Error cases:** Invalid characters, unexpected symbols
✅ **Edge cases:** Empty input, single token, long identifiers
✅ **Position tracking:** Line/column numbers are correct

**Example test:**
```cpp
void testLexer() {
    Lexer lex("(% (i;0;5) (<\"hi\">))");
    auto tokens = lex.tokenize();
    
    assert(tokens[0].type == LPAREN);
    assert(tokens[1].type == OPERATOR);
    assert(tokens[1].value == "%");
    assert(!lex.hasErrors());
}
```

## Next Phase: Syntaxer

After the Lexer produces tokens, they flow to the **Syntaxer** which:
- Validates token sequences
- Builds an Abstract Syntax Tree (AST)
- Detects syntax errors

See [syntaxer/readme.md](../syntaxer/readme.md) for details.

---

**Implementation File:** [main.cpp](main.cpp)
