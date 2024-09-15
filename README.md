![Banner](./branding/truffle-banner.png)

# Truffle
**Compile-time memory management without caveats**

Truffle is compiler engine that provides an easy way to manage memory at compile time without burdening the user with the unsaftey manual memory management or the complexity of an ownership model. Truffle is also a programming language; this language acts as a front end to the truffle compiler engine meant to showcase it's abilities. 

---

## 🚧 Work In Progress

**Note:** Truffle is in a very early state and nearly everything is currently under active development.
Key areas currently being worked on:
- **LLVM Integration**: Leveraging LLVM's backend for highly efficient code generation.
- **AutoFree Functionality**: Implementing an automatic memory cleanup feature where the compiler determines when objects are no longer in use and frees memory without developer intervention. This feature aims to eliminate the need for manual deallocation while ensuring efficient memory use, offering the best of both worlds—high performance without the risk of memory leaks.
- **LLM Enabled Optimization**: This is a highly experimental feature. While LLVM optimizes LLVM IR, it does not focus extensively on optimizing the final machine code for specific target architectures. We are experimenting with using a Large Language Model (LLM) to optimize sections of the final machine code. This feature is inspired by a [research paper published by Meta](https://ai.meta.com/research/publications/meta-large-language-model-compiler-foundation-models-of-compiler-optimization/). 