# Semanter

The semanter is the planned semantic-analysis stage.

## Where It Will Sit

The intended future pipeline is:

```text
Lexer
  -> std::vector<Token>
  -> Parser
  -> AST or richer IR
  -> Semanter
  -> validated/annotated IR
  -> Generator
```

## What It Should Eventually Own

- symbol tables
- scope graphs
- declaration/use validation
- type validation
- operation legality checks
- richer diagnostics than the current parser-level checks

## What Happens Today Instead

Today, a small subset of semantic work is still embedded in the parser:

- identifiers must be declared before use
- assignment operands must have supported shapes

That is a practical shortcut, not the intended long-term architecture.

## Recommended Future Data Nodes

When implemented, this stage will likely want:

- symbol entries
- resolved identifier references
- type annotations
- error lists attached to structured nodes
