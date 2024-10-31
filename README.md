![Banner](./branding/truffle-banner.png)

# Truffle
**PGO Without the Hassle**
Profile Guided Optimization (PGO) is a technique that uses a program's runtime data (or "runtime profile") to make more informed decisions in the compilation process, leading to faster code and smaller binaries. The problem with PGO is that high quality runtime data can be time consuming and tedious to acquire, especially in a fast-paced production setting. Truffle aims to use AI to automatically generate synthetic runtime data based on a program's source code. This one additional step in the compilation process can improve runtime performance of many programs anywhere from 0-15%. 

---

## 🔭 Scope
Currently, Truffle only aims to support C/C++. Support for other systems level languages may come in the far future. 

## 🚧 Work In Progress
**Note:** Truffle is in a very early state and nearly everything is currently under active development.
Key areas currently being worked on:
- **TrufC**: TrufC is the CLI tool that users will interact with when using Truffle to optimize their code. 
- **TPGO 1 25M && TPGO 1 200M**: Truffle PGO (TPGO) are the models that will produce synthetic runtime data.