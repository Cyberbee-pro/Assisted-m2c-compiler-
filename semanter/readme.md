# ✅ Semanter - Phase 3: Semantic Analysis

## What is the Semanter?

The **Semanter** (Semantic Analyzer) is the **third phase** of the M2C compiler. It checks if the parse tree makes **logical sense**:
- Do all variables exist before use?
- Are types consistent?
- Do scopes make sense?

While the Syntaxer asks "Is this valid grammar?", the Semanter asks "Does this actually mean something valid?"

```
AST (syntactically correct):   x = y + 10
                                    ↓
        [SEMANTER analyzes]
                                    ↓
✅ Valid:    x and y are declared, types match
❌ Invalid:  y is undefined, type mismatch, etc.
```

## Input & Output

### 📥 Input
- **Abstract Syntax Tree (AST)** from the Syntaxer

### 📤 Output
- **Annotated AST** with type information
- Nodes marked as valid/invalid
- Semantic error list (if any)

## Data Structures

### Symbol Table

```cpp
class Symbol {
    std::string name;
    std::string type;        // "int", "string", "float", etc.
    bool isInitialized;
    int declarationLine;
};

class SymbolTable {
    // Add a symbol to current scope
    void addSymbol(Symbol symbol);
    
    // Look up a symbol
    Symbol* lookupSymbol(std::string name);
    
    // Enter a new scope (for blocks, loops, etc.)
    void enterScope();
    
    // Exit the current scope
    void exitScope();
    
private:
    std::vector<std::map<std::string, Symbol>> scopes;
};
```

### Type System

```cpp
enum Type {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_VOID,
    TYPE_UNKNOWN,
    TYPE_ERROR
};

class TypeInfo {
    Type baseType;
    bool isArray;
    
    // Check if two types are compatible
    static bool isCompatible(Type t1, Type t2);
};
```

### Annotated AST Node

```cpp
class ASTNode {
    // ... existing fields ...
    TypeInfo* typeInfo;              // Type of this node
    bool isSemanticValid;            // Did semantic check pass?
    std::vector<std::string> errors; // Semantic errors found
};
```

## Implementation Guidelines

### 1. **AST Visitor Pattern**

Use the visitor pattern to traverse the AST:

```cpp
class SemanticAnalyzer {
    SemanticAnalyzer();
    
    // Main entry point
    void analyze(Program* ast);
    
    // Visitor methods for each node type
    void visit(Program* node);
    void visit(IfStatement* node);
    void visit(WhileLoop* node);
    void visit(ForLoop* node);
    void visit(BinaryOp* node);
    void visit(Identifier* node);
    void visit(Assignment* node);
    void visit(FunctionCall* node);
    // ... etc for all AST node types
    
private:
    SymbolTable symbolTable;
    std::vector<std::string> errors;
    TypeInfo currentNodeType;
};
```

### 2. **Type Inference**

Determine the type of each expression:

```cpp
TypeInfo* SemanticAnalyzer::inferType(Expression* expr) {
    if (Number* num = dynamic_cast<Number*>(expr)) {
        return new TypeInfo(TYPE_INT);
    }
    else if (String* str = dynamic_cast<String*>(expr)) {
        return new TypeInfo(TYPE_STRING);
    }
    else if (BinaryOp* op = dynamic_cast<BinaryOp*>(expr)) {
        TypeInfo* leftType = inferType(op->left);
        TypeInfo* rightType = inferType(op->right);
        
        if (!TypeInfo::isCompatible(leftType->baseType, rightType->baseType)) {
            addError("Type mismatch in binary operation");
            return new TypeInfo(TYPE_ERROR);
        }
        return leftType;
    }
    return new TypeInfo(TYPE_UNKNOWN);
}
```

### 3. **Scope Handling**

Track variable scope:

```cpp
void SemanticAnalyzer::visit(Block* block) {
    symbolTable.enterScope();
    
    for (Statement* stmt : block->statements) {
        analyzeStatement(stmt);
    }
    
    symbolTable.exitScope();
}

void SemanticAnalyzer::visit(Assignment* node) {
    // Check if variable exists
    Symbol* sym = symbolTable.lookupSymbol(node->variableName);
    if (!sym) {
        addError("Undefined variable: " + node->variableName);
        return;
    }
    
    // Check type compatibility
    TypeInfo* exprType = inferType(node->value);
    if (!TypeInfo::isCompatible(sym->type, exprType->baseType)) {
        addError("Type mismatch in assignment");
    }
}
```

### 4. **Function Call Validation**

```cpp
void SemanticAnalyzer::visit(FunctionCall* node) {
    // Validate built-in functions
    if (node->functionName == "printf") {
        // printf expects at least one argument (format string)
        if (node->arguments.empty()) {
            addError("printf requires at least one argument");
        }
    }
    else if (node->functionName == "scanf") {
        // scanf expects variable references
        if (node->arguments.empty()) {
            addError("scanf requires at least one argument");
        }
    }
    // ... handle other functions
}
```

## Semantic Error Examples

```
Error: Undefined variable
    x = y + 10;
        ^ y not found in scope

Error: Type mismatch
    int x = "hello";
        Attempting to assign string to int variable

Error: Redeclaration of variable
    int x;
    int x;  <- Error: x already declared on line 3

Error: Missing statement body
    if (x > 5)
    ^ Expected block after condition

Error: Invalid function call
    printf();
           ^ printf requires arguments
```

## Interface / Main Entry Point

```cpp
class SemanticAnalyzer {
    SemanticAnalyzer();
    
    // Analyze the AST for semantic correctness
    void analyze(Program* ast);
    
    // Get all errors found
    std::vector<std::string> getErrors();
    
    // Check if analysis was successful
    bool isValid();
};
```

## Testing

Test cases should cover:
- Variable declaration and usage
- Scope boundaries (can't use variable outside its scope)
- Type checking in assignments
- Type checking in operations
- Function argument validation
- Undefined variable detection
- Duplicate declaration detection
- Proper error messages with line numbers

### Example Test Case

```cpp
// Semantic error: undefined variable in morse code
std::string code = "(~(x...-..-.--.....--....-...--.-)\n) (<\"....+..\">;))";  // if x < 10 print x (x undefined)

// Parse to AST...
// Then analyze:
SemanticAnalyzer analyzer;
analyzer.analyze(ast);

assert(!analyzer.isValid());
assert(analyzer.getErrors().size() > 0);
assert(analyzer.getErrors()[0].find("x") != std::string::npos);
```

## Next Phase

After semantic analysis, the annotated AST flows to the **Optimizers** which:
- Perform code optimization
- Remove dead code
- Improve performance

See `optimizers/readme.md` for more details.

---

**Key Concepts**: Type Checking, Scope Analysis, Symbol Table, Semantic Errors, Type Inference
