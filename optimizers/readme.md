# ⚡ Optimizers - Phase 4: Code Optimization (Optional)

## What are the Optimizers?

The **Optimizers** form the **fourth (optional) phase** of the M2C compiler. They improve code **performance and efficiency** without changing what the program does.

This phase is **optional** - you can skip it if runtime speed is not a concern.

```
AST (working but possibly inefficient):
    x = 5 + 3 + 2
                ↓
    [OPTIMIZERS improve]
                ↓
Optimized AST:
    x = 10  (computed at compile time)
```

## Input & Output

### 📥 Input
**Annotated AST** from the Semanter (already type-checked and validated)

### 📤 Output
**Optimized AST** - Same logical meaning, but faster execution

## Key Responsibilities

✅ **Dead Code Elimination** - Remove unreachable code
✅ **Constant Folding** - Pre-compute constant expressions  
✅ **Loop Optimization** - Simplify or unroll loops
✅ **Algebraic Simplification** - Simplify expressions
✅ **Common Subexpression Elimination** - Eliminate redundant calculations

## Optimization Techniques

### 1. **Dead Code Elimination**

Remove code that never executes:

```
// Before
if (false) {
    x = 10;          // Dead code
}
y = 20;

// After
y = 20;
```

### 2. **Constant Folding**

Evaluate constant expressions at compile time:

```
// Before
x = 5 + 3 * 2;
y = "hello" + " world";

// After
x = 11;
y = "hello world";
```

### 3. **Loop Optimization**

Simplify loops when possible:

```
// Before
for (i = 0; i < 5; i = i + 1) {
    print(i);
}

// Optimized (unrolled)
print(0);
print(1);
print(2);
print(3);
print(4);
```

### 4. **Algebraic Simplification**

Simplify algebraic expressions:

```
// Before
x = y * 1;
z = a + 0;
w = b * 0;

// After
x = y;
z = a;
w = 0;
```

### 5. **Common Subexpression Elimination**

Avoid recalculating the same expression:

```
// Before
x = a + b;
y = a + b;
z = a + b;

// After
temp = a + b;
x = temp;
y = temp;
z = temp;
```

## Data Structures

```cpp
class Optimizer {
    Optimizer();
    
    // Apply all optimizations
    void optimize(ASTNode*& node);
    
    // Individual optimization methods
    void eliminateDeadCode(ASTNode*& node);
    void foldConstants(ASTNode*& node);
    void optimizeLoops(ASTNode*& node);
    void simplifyAlgebraic(ASTNode*& node);
    void eliminateCommonExpressions(ASTNode*& node);
    
private:
    // Helper to determine if code is reachable
    bool isReachable(ASTNode* node);
    
    // Helper to evaluate constant expressions
    bool tryEvaluateConstant(Expression* expr, long long& result);
    
    // Expression comparison for CSE
    bool expressionsAreEquivalent(Expression* e1, Expression* e2);
};

class ExpressionEvaluator {
    // Used for constant folding
    bool evaluate(Expression* expr, long long& result);
};
```

## Implementation Guidelines

### 1. **Safe Optimization**

Ensure optimizations never change program behavior:

```cpp
bool Optimizer::canOptimize(ASTNode* node) {
    // Only optimize if semantically safe
    if (!hasNoSideEffects(node)) return false;
    if (node->isSemanticValid == false) return false;
    return true;
}
```

### 2. **Iterative Optimization**

Apply optimizations multiple times until no more changes:

```cpp
void Optimizer::optimize(ASTNode*& node) {
    bool changed = true;
    
    while (changed) {
        changed = false;
        
        changed |= applyDeadCodeElimination(node);
        changed |= applyConstantFolding(node);
        changed |= applyAlgebraicSimplification(node);
        changed |= applyCESE(node);
    }
}
```

### 3. **AST Node Modification**

When optimizing, may remove or replace nodes:

```cpp
ASTNode* Optimizer::foldConstants(Expression* expr) {
    if (BinaryOp* op = dynamic_cast<BinaryOp*>(expr)) {
        long long leftVal, rightVal;
        
        if (tryEvaluateConstant(op->left, leftVal) &&
            tryEvaluateConstant(op->right, rightVal)) {
            
            long long result;
            if (op->op == "+") result = leftVal + rightVal;
            else if (op->op == "*") result = leftVal * rightVal;
            // ... etc
            
            // Replace with constant
            return new Number(result);
        }
    }
    return expr;
}
```

### 4. **Optimization Levels**

Consider supporting different optimization levels:

```cpp
enum OptimizationLevel {
    LEVEL_NONE,        // No optimization
    LEVEL_O1,          // Basic optimizations
    LEVEL_O2,          // Aggressive optimizations
    LEVEL_O3           // Maximum optimization
};

class Optimizer {
    Optimizer(OptimizationLevel level);
    
    // Optimization applied based on level
    void optimize(ASTNode*& node);
};
```

## Interface / Main Entry Point

```cpp
class Optimizer {
    Optimizer(OptimizationLevel level = LEVEL_O2);
    
    // Apply optimizations to the AST
    void optimize(ASTNode*& node);
    
    // Get optimization statistics
    struct OptimizationStats {
        int nodesRemoved;
        int constantsEvaluated;
        int expressionsSimplified;
    };
    OptimizationStats getStats();
};
```

## Testing

Test cases should verify:
- Optimization doesn't change program behavior
- Dead code is removed
- Constants are properly folded
- Algebraic simplifications are correct
- Common subexpressions are identified
- Edge cases and corner cases

### Example Test Case

```cpp
// Test constant folding
ASTNode* ast = parseCode("x = 5 + 3 * 2;");
Optimizer opt;
opt.optimize(ast);

// Verify that x got value 11, not a BinaryOp
Assignment* assign = (Assignment*)ast;
Number* result = dynamic_cast<Number*>(assign->value);
assert(result != nullptr);
assert(result->value == 11);
```

## Performance Considerations

Optimizations can be expensive. Balance between:
- **Optimization time**: How long does optimization take?
- **Generated code quality**: How much better is the generated code?
- **Trade-off**: For simple morse code programs, aggressive optimization may not be worth it

## Next Phase

After optimization, the refined AST flows to the **Generators** which:
- Convert the AST to C code
- Emit valid, compilable C code

See `generations/readme.md` for more details.

---

**Key Concepts**: Dead Code Elimination, Constant Folding, Loop Optimization, Algebraic Simplification, Common Subexpression Elimination
