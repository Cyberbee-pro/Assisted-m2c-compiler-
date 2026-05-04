# Contributing To M2C Compiler

This file mirrors [contribution.md](contribution.md).

## Mental Model

Treat the compiler as a pipeline of data nodes:

```text
CliOptions
  -> Token vector
  -> Statement vector
  -> Assembly file
  -> Linked executable
```

## Before Editing

Read:

- [working.md](working.md)
- the phase README for the component you plan to change

## Validate Locally

```bash
bash build.sh
bash run.sh m2c_files/test_math.cym2c demo_math
./demo_math
```

## Update Docs When Needed

If you change any stage boundary or language behavior, update the docs in the same pass.
