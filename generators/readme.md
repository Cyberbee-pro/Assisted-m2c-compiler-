# 🖨️ Generators - Phase 5: Code Generation

## What are the Generators?

The **Generators** (Code Generator) form the **final phase** of the M2C compiler. They convert the optimized AST into **executable C code**.

This is where morse code becomes actual C programs!

```
Optimized AST:       Program
                      /  |  \
                    For If  ...
                     ↓
        [GENERATORS emit]
                     ↓
   #include <stdio.h>
   int main() {
       for (...) {
           if (...) {
               printf(...)
           }
       }
       return 0;
   }
```

## Input & Output

### 📥 Input
- **Optimized AST** from Optimizers (or Semanter if no optimization)
- All nodes are validated and type-checked

### 📤 Output
- **Valid C code** - ready to compile with gcc/clang
- Includes all necessary headers
- Proper structure and indentation

## Key Responsibilities

✅ **Header Generation** - Add necessary `#include` statements
✅ **Variable Declaration** - Emit `int x;` for all variables
✅ **Statement Translation** - Convert AST nodes to C statements
✅ **Expression Translation** - Convert morse expressions to C operators
✅ **Function Call Translation** - Convert morse I/O to `printf()`
✅ **Code Formatting** - Proper indentation and structure

## C Code Generation Rules

### 1. **Built-in Functions Mapping**

Map morse language functions to C equivalents:

```
Morse Function         →  C Code
──────────────────────────────────────────────
<"....+..">;         →  printf("hi\n");
/{<...>}             →  variable output
```

Note: Morse inside quotes is decoded to ASCII before output

### 2. **Data Types**

Map morse types to C types:

```
Variable Type  →  C Type
──────────────────────────
integer        →  int
float/double   →  double
string         →  char*
```

### 3. **Control Structures**

```
// If Statement
Morse:  (if condition then-part else-part)
C:      if (condition) {
            // then-part
        } else {
            // else-part
        }

// While Loop
Morse:  (while condition body)
C:      while (condition) {
            // body
        }

// For Loop
Morse:  (for init condition update body)
C:      for (init; condition; update) {
            // body
        }
```

## Data Structures

```cpp
class CodeGenerator {
    CodeGenerator();
    
    // Generate C code from AST
    std::string generate(ASTNode* ast);
    
private:
    std::stringstream output;           // Output C code
    int indentLevel;                    // Current indentation level
    std::vector<std::string> headers;   // Required headers
    std::set<std::string> variables;    // Declared variables
    
    // Code generation methods
    void generateProgram(Program* node);
    void generateStatement(Statement* node);
    void generateExpression(Expression* node);
    void generateIfStatement(IfStatement* node);
    void generateWhileLoop(WhileLoop* node);
    void generateForLoop(ForLoop* node);
    void generateBlock(Block* node);
    void generateAssignment(Assignment* node);
    void generateFunctionCall(FunctionCall* node);
    void generateBinaryOp(BinaryOp* node);
    
    // Utility methods
    void emit(std::string code);
    void emitLine(std::string code);
    void indent();
    void dedent();
    void addHeader(std::string header);
    std::string getTypeName(TypeInfo* type);
};
```

## Implementation Guidelines

### 1. **Program Structure**

Every generated C program needs:

```cpp
void CodeGenerator::generateProgram(Program* node) {
    // 1. Add headers
    addHeader("#include <stdio.h>");
    addHeader("#include <stdlib.h>");
    
    // 2. Emit headers
    for (auto header : headers) {
        emitLine(header);
    }
    emitLine("");
    
    // 3. Declare variables
    emitLine("int main() {");
    indent();
    
    // ... declare all variables ...
    
    // 4. Generate statements
    for (Statement* stmt : node->statements) {
        generateStatement(stmt);
    }
    
    // 5. Return statement
    emitLine("return 0;");
    dedent();
    emitLine("}");
}
```

### 2. **Variable Declaration**

Track and declare all variables:

```cpp
void CodeGenerator::declareVariable(std::string name, TypeInfo* type) {
    if (variables.find(name) == variables.end()) {
        emitLine(getTypeName(type) + " " + name + ";");
        variables.insert(name);
    }
}

std::string CodeGenerator::getTypeName(TypeInfo* type) {
    switch (type->baseType) {
        case TYPE_INT:     return "int";
        case TYPE_FLOAT:   return "double";
        case TYPE_STRING:  return "char*";
        case TYPE_VOID:    return "void";
        default:           return "int";  // default
    }
}
```

### 3. **Expression Generation**

```cpp
void CodeGenerator::generateExpression(Expression* node) {
    if (Number* num = dynamic_cast<Number*>(node)) {
        emit(std::to_string((int)num->value));
    }
    else if (String* str = dynamic_cast<String*>(node)) {
        emit("\"" + str->value + "\"");
    }
    else if (Identifier* id = dynamic_cast<Identifier*>(node)) {
        emit(id->name);
    }
    else if (BinaryOp* op = dynamic_cast<BinaryOp*>(node)) {
        emit("(");
        generateExpression(op->left);
        emit(" " + op->op + " ");
        generateExpression(op->right);
        emit(")");
    }
}
```

### 4. **Statement Generation**

```cpp
void CodeGenerator::generateStatement(Statement* node) {
    if (IfStatement* ifStmt = dynamic_cast<IfStatement*>(node)) {
        generateIfStatement(ifStmt);
    }
    else if (WhileLoop* loop = dynamic_cast<WhileLoop*>(node)) {
        generateWhileLoop(loop);
    }
    else if (ForLoop* loop = dynamic_cast<ForLoop*>(node)) {
        generateForLoop(loop);
    }
    else if (Assignment* assign = dynamic_cast<Assignment*>(node)) {
        generateAssignment(assign);
    }
    else if (FunctionCall* call = dynamic_cast<FunctionCall*>(node)) {
        generateFunctionCall(call);
    }
    else if (Block* block = dynamic_cast<Block*>(node)) {
        generateBlock(block);
    }
}
```

### 5. **Control Flow Generation**

```cpp
void CodeGenerator::generateIfStatement(IfStatement* node) {
    emit("if (");
    generateExpression(node->condition);
    emitLine(") {");
    
    indent();
    generateStatement(node->thenBranch);
    dedent();
    
    if (node->elseBranch) {
        emitLine("} else {");
        indent();
        generateStatement(node->elseBranch);
        dedent();
    }
    
    emitLine("}");
}

void CodeGenerator::generateWhileLoop(WhileLoop* node) {
    emit("while (");
    generateExpression(node->condition);
    emitLine(") {");
    
    indent();
    generateStatement(node->body);
    dedent();
    
    emitLine("}");
}

void CodeGenerator::generateForLoop(ForLoop* node) {
    emitLine("for (", false);
    // ... init; condition; update ...
    emitLine(") {");
    
    indent();
    generateStatement(node->body);
    dedent();
    
    emitLine("}");
}
```

### 6. **Function Call Generation**

```cpp
void CodeGenerator::generateFunctionCall(FunctionCall* node) {
    if (node->functionName == "printf") {
        // printf requires format string
        emitLine("printf(\"");
        for (int i = 0; i < node->arguments.size(); i++) {
            if (i > 0) emit(" ");
            // Guess type and use appropriate format
            emit("%d");
        }
        emit("\\n\"");
        
        for (auto arg : node->arguments) {
            emit(", ");
            generateExpression(arg);
        }
        emitLine(");");
    }
    else if (node->functionName == "scanf") {
        emitLine("scanf(\"");
        for (int i = 0; i < node->arguments.size(); i++) {
            emit("%d");
        }
        emit("\"");
        
        for (auto arg : node->arguments) {
            emit(", &");
            generateExpression(arg);
        }
        emitLine(");");
    }
}
```

## Interface / Main Entry Point

```cpp
class CodeGenerator {
    CodeGenerator();
    
    // Generate C code from AST
    std::string generate(ASTNode* ast);
    
    // Get generated code
    std::string getCode();
    
    // Write to file
    void writeToFile(std::string filename);
};
```

## Testing

Test cases should verify:
- Generated code is valid C syntax
- Code compiles without errors
- Output behavior matches input morse code
- Control structures are correctly translated
- Variable declarations are generated
- Function calls use correct C syntax
- Proper indentation and formatting

### Example Test Case

```cpp
// Parse morse code for: while x < 10 print x
ASTNode* ast = parseMorseCode("· · · · ··· ·· ");

CodeGenerator gen;
std::string cCode = gen.generate(ast);

// Verify generated C code
assert(cCode.find("#include <stdio.h>") != std::string::npos);
assert(cCode.find("while") != std::string::npos);
assert(cCode.find("printf") != std::string::npos);
assert(cCode.find("int main()") != std::string::npos);
```

## Generated Code Example

Given this morse code input:
```
(%(x;0;--;....)
    (<"....+..">;)
    (x--;x)
)
```
(for x=0; x<5; x++ print x)

Generated C code:
```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    int x;
    for (x = 0; x < 5; x++) {
        printf("hi\n");
    }
    return 0;
}
```

## Output Validation

The generated code should:
1. Be syntactically valid C
2. Compile without errors
3. Execute with correct behavior
4. Be reasonably readable and formatted
5. Include necessary headers and structure

---

**Key Concepts**: AST Traversal, Code Emission, C Syntax, Control Flow Translation, Variable Management
