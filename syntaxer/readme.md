# 🌳 Syntaxer - Phase 2: Parsing & AST Building

## What is the Syntaxer?

The **Syntaxer** (Syntax Analyzer) is the **second phase** of the M2C compiler. It takes the token stream from the Lexer (which has already decoded morse to standard notation) and:
1. **Validates** that tokens follow the morse language grammar
2. **Builds** an Abstract Syntax Tree (AST) - a tree representation of program structure
3. **Reports** syntax errors if tokens violate grammar rules

**Key Point:** Morse numbers and identifiers have already been decoded by the Lexer, so the Syntaxer works with standard tokens like `5`, `i`, `hello`, etc.

```
Morse input:     ( % ( .... ; ----- ; ..... ) ... )
                         ↓
            [LEXER decodes morse]
                         ↓
Token stream:     ( % ( i ; 0 ; 5 ) ... )
                         ↓
        [SYNTAXER processes]
                         ↓
                    Program AST
                      /    \
                  ForLoop   ...
                  /  |  \
                init cond body
```

## Input & Output

### 📥 Input
- **Token Stream** from the Lexer
- Example: `Token(LPAREN), Token(OPERATOR, "%"), Token(LPAREN), ...`

### 📤 Output
- **Abstract Syntax Tree (AST)**  
- Tree nodes representing program structure
- Type: Root node is `Program` class

## Key Responsibilities

✅ **Grammar Validation**
- Ensure tokens match valid morse syntax
- Check parentheses balance and nesting

✅ **AST Construction**
- Create appropriate AST nodes for each construct
- Link nodes into a tree structure

✅ **Error Detection**
- Catch missing tokens or unexpected sequences
- Report syntax errors with helpful context

✅ **Position Tracking**
- Map AST nodes back to source locations
- Enable accurate error reporting

## Grammar Rules

The M2C compiler supports these structures with morse syntax:

```
Program ::= Statement*

Statement ::= IfStatement | ForLoop | WhileLoop | FunctionCall

IfStatement ::= "~" "(" Condition ")" Statement

ForLoop ::= "%" "(" Var ";" Init ";" Condition ";" Update ")" Statement

WhileLoop ::= "%%" "(" Condition ")" Statement  

FunctionCall ::= "/{" ArgumentList "}" | "<" String ">;"

Condition ::= Expression ComparisonOp Expression

Expression ::= Term (("\+"|"\-") Term)*

Term ::= Factor (("\*"|"\/") Factor)*

Comparison Operators: 
  - "\<" (less than)
  - "\>" (greater than)  
  - "==" (equality, NO backslash)
```

## Data Structures - AST Nodes

```cpp
// Base class for all AST nodes
class ASTNode {
    virtual ~ASTNode() {}
    virtual void accept(ASTVisitor* visitor) = 0;
};

// Expression nodes
class Expression : public ASTNode {
    // abstract
};

class BinaryOp : public Expression {
    Expression* left;
    std::string op;      // +, -, *, /, <, >, ==, etc.
    Expression* right;
};

class Identifier : public Expression {
    std::string name;
};

class Number : public Expression {
    double value;
};

class String : public Expression {
    std::string value;
};

// Statement nodes
class Statement : public ASTNode {
    // abstract
};

class IfStatement : public Statement {
    Expression* condition;
    Statement* thenBranch;
    Statement* elseBranch;  // can be nullptr
};

class WhileLoop : public Statement {
    Expression* condition;
    Statement* body;
};

class ForLoop : public Statement {
    Statement* init;
    Expression* condition;
    Statement* update;
    Statement* body;
};

class Block : public Statement {
    std::vector<Statement*> statements;
};

class FunctionCall : public Statement {
    std::string functionName;
    std::vector<Expression*> arguments;
};

class Assignment : public Statement {
    std::string variableName;
    Expression* value;
};

class Program : public ASTNode {
    std::vector<Statement*> statements;
};
```

## Implementation Guidelines

### 1. **Recursive Descent Parser**

Use recursive descent parsing - each grammar rule becomes a function:

```cpp
class Parser {
    Parser(std::vector<Token> tokens);
    
    Program* parse();
    
private:
    std::vector<Token> tokens;
    int current;
    
    // Grammar rule functions
    Statement* parseStatement();
    IfStatement* parseIfStatement();
    WhileLoop* parseWhileLoop();
    ForLoop* parseForLoop();
    Expression* parseExpression();
    Expression* parseTerm();
    Expression* parseFactor();
    
    // Utility functions
    Token peek();
    Token advance();
    bool check(TokenType type);
    bool match(TokenType type);
    void error(std::string message);
};
```

### 2. **Error Recovery**

When encountering syntax errors:
- Report the error with location information
- Attempt to recover to continue parsing
- Collect multiple errors if possible

```cpp
void Parser::error(std::string message) {
    Token current = peek();
    std::cerr << "Syntax Error at line " << current.line 
              << ", column " << current.column 
              << ": " << message << std::endl;
}
```

### 3. **Operator Precedence**

Handle operator precedence correctly:
```
Highest:  * /
          + -
Lowest:   < > == !=
```

## Interface / Main Entry Point

```cpp
class Parser {
    Parser(std::vector<Token> tokens);
    
    // Parse tokens into AST
    Program* parse();
    
    // Get error messages
    std::vector<std::string> getErrors();
    
    // Check if parsing was successful
    bool hasErrors();
};
```

## Testing

Test cases should cover:
- Valid morse/token sequences produce correct AST
- Nested structures (loops within if, etc.)
- All statement types and expressions
- Error messages for invalid syntax
- Operator precedence
- Edge cases (empty blocks, missing semicolons, etc.)

### Example Test Case

```cpp
// Input tokens for: "if (x < 10) printf(x);"
std::vector<Token> tokens = {
    Token(KEYWORD, "if", 1, 0),
    Token(LPAREN, "(", 1, 2),
    Token(IDENTIFIER, "x", 1, 3),
    Token(OPERATOR, "<", 1, 4),
    Token(NUMBER, "10", 1, 6),
    Token(RPAREN, ")", 1, 8),
    Token(KEYWORD, "printf", 1, 10),
    Token(LPAREN, "(", 1, 16),
    Token(IDENTIFIER, "x", 1, 17),
    Token(RPAREN, ")", 1, 18),
    Token(SEMICOLON, ";", 1, 19),
    Token(END_OF_FILE, "", 1, 20)
};

Parser parser(tokens);
Program* ast = parser.parse();

// Verify AST structure
assert(ast->statements.size() == 1);
assert(dynamic_cast<IfStatement*>(ast->statements[0]) != nullptr);
IfStatement* ifStmt = (IfStatement*)ast->statements[0];
assert(dynamic_cast<BinaryOp*>(ifStmt->condition) != nullptr);
```

## Next Phase

After parsing, the AST flows to the **Semanter** which:
- Checks semantic correctness (types, scope, etc.)
- Detects logical errors
- Annotates AST with type information

See `semanter/readme.md` for more details.

---

**Key Concepts**: Recursive Descent Parsing, Abstract Syntax Tree, Operator Precedence, Error Recovery
