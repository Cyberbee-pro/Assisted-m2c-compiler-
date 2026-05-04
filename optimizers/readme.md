# Optimizers

The optimizer phase is still planned.

## Current Situation

M2C currently goes from:

```text
Lexer -> Parser -> Generator -> Assembly Runtime -> Binary
```

There is no separate optimizer pass yet.

## Intended Role

An eventual optimizer would sit between semantic analysis and generation.

Likely responsibilities:

- constant folding
- algebraic simplification
- dead-code elimination
- reducing redundant runtime calls
- simplifying expression trees before assembly emission

## Why It Is Not Implemented Yet

The current compiler still uses a small statement-level IR instead of a full AST, so optimization opportunities are intentionally limited.

## Good Next Steps

- introduce a richer AST first
- add a semantic phase that resolves symbols and types
- then implement simple constant folding before assembly generation
