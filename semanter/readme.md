# Semanter

The semanter phase is still planned, not fully implemented as a separate module yet.

## Current Situation

Some semantic checks currently happen inside the parser:

- identifier must be declared before use
- assignments must use valid operand shapes

That was a practical shortcut to get the compiler producing working binaries.

## Intended Role

The long-term job of `semanter/` is to own:

- symbol tables
- declaration/use validation
- type validation
- scope handling
- richer semantic diagnostics

## Future Direction

When this phase is implemented, the likely flow will become:

```text
tokens
  -> parser
  -> AST
  -> semanter
  -> validated/annotated AST
  -> generator
```

## Good Next Steps

- move declaration checks out of the parser
- define a small symbol-table API
- annotate expressions with resolved types
- prepare the generator to consume semantic information instead of inference by convention
