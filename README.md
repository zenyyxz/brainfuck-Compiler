# The High-Performance Brainfuck Compiler

This project is a highly efficient **Brainfuck-to-x86_64 compiler** written from scratch. After building a Pi calculator and a SAT solver, this was the next logical step: turning esoteric code into raw, optimized assembly without using interpreters, LLVM shortcuts, or external libraries.

## The "From Scratch" Flex

I didn't just map BF symbols to instructions. I built a full-blown compilation pipeline:

1.  **Lexer/Parser:** Eats `.bf` files and turns them into a high-level `ActionPlan`.
2.  **Custom IR (Intermediate Representation):** I defined my own "Action" system to represent complex idioms, not just single bytes.
3.  **The Optimizer (The Brains):** This is where it gets nerdy. I implemented:
    *   **Run-Length Encoding (RLE):** Turns `+++++` into a single `add byte [rbx], 5`.
    *   **Offset Folding:** The coolest part. It eliminates redundant pointer moves by calculating relative offsets (e.g., `>++` becomes `add byte [rbx+1], 2`).
    *   **Generalized Multi-Target Multiply:** Detects loops like `[->+++>++++<<]` and replaces them with high-speed `imul` instructions. No more looping for math!
    *   **Scan Loop Optimization:** Replaces `[>]` and `[<]` with tight assembly search loops.
    *   **Set-Zero Logic:** Detects `[-]` and just `mov`s a zero into the cell.
4.  **The Backend:** Generates **pure x86-64 NASM (Intel syntax)**. 
    *   **Zero Dependencies:** No `libc`. No `printf`. Just raw **Linux Syscalls** (`rax=1`, `rax=60`).
    *   **Spec Compliant:** Strictly adheres to the **30,000 cell** limit.

## Building & Running

You'll need `g++` (C++20), `nasm`, and `ld`.

```bash
# Build the compiler
make

# Compile a BF program to a standalone ELF executable
./bin/bfc tests/mandelbrot.bf -o mandelbrot

# Run the beast
./mandelbrot
```

## Performance

The result? The famous **Mandelbrot fractal viewer** (a massive 11KB BF file) compiles and runs in **~0.6s**. Most interpreters are still thinking about the first pixel while this thing is already finished.

## Project Structure

- `src/main.cpp`: The entry point and CLI.
- `src/lexer.cpp`: Turns text into our `ActionPlan`.
- `src/optimizer.cpp`: The heavy lifter that makes the code fast.
- `src/codegen.cpp`: The assembly artist that writes the `.s` file.
- `src/ir.hpp`: Definition of our custom intermediate language.
