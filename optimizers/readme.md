# Optimizers

The optimizer stage is planned for the future.

## Current Pipeline Context

Today the compiler goes directly from parser IR to assembly generation:

```text
std::vector<Token>
  -> std::vector<Statement>
  -> temp.s
```

No optimizer sits between those two stages yet.

## Likely Future Role

An optimizer would eventually consume a richer IR or AST and produce a semantically equivalent but better-shaped form for code generation.

Useful future optimizations:

- constant folding
- algebraic simplification
- dead-code elimination
- reduction of unnecessary runtime calls
- reuse of repeated expressions

## Why It Is Not Here Yet

The current `Statement` IR is intentionally small and flat, so optimization opportunities are limited until the parser grows into a richer structure.
