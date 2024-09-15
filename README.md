# Truffle

**Compile-time memory management without caveats**

Truffle is a statically-typed compiled language built on LLVM that aims to offer compile-time memory management without caveats. It combines the performance benefits of languages like C/C++ with the ease of memory safety, eliminating the manual memory management pitfalls associated with low-level languages and the complexity of ownership models found in modern languages like Rust.

---

## 🚧 Work In Progress

**Truffle is currently under active development.** While many core concepts are already in place, the language is continuously evolving as we build out features, refine its design, and improve performance. Please note that certain areas of the language and its toolchain are still being worked on, and we welcome feedback from early adopters and contributors.

Key areas we are working on:
- Enhanced error handling and diagnostics
- Expanded standard library
- Optimized LLVM integration
- More comprehensive benchmarking and performance testing
- Extensive documentation

Expect frequent updates as we iterate and bring Truffle closer to production readiness.

---

## 🚀 Features
- **Compile-Time Memory Management**: Memory management is resolved at compile time, avoiding runtime overhead.
- **LLVM Backend**: Truffle leverages the LLVM backend for efficient, optimized code generation.
- **Type Safety**: Strong static typing with type inference and immutability by default.
- **Simplicity & Performance**: Designed for low-level systems programming without the pitfalls of manual memory handling.
- **Developer Velocity**: Unlike C/C++ and Rust, Truffle aims to be easy to quickly iterate with.
- **Warning ⚠️**: The main purpose of this project is to create a compiler engine that manages memory at compile time without caveats. The truffle language itself merely acts as a front end to this compiler engine to show case it's abilities. 
