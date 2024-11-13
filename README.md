![Banner](./branding/truffle-banner.png)

# Truffle
**A Touch of Modern for C/C++**
The Truffle project includes several different aspects to make C/C++ easier, safer, and faster. The entry point to the Truffle suite of tools is TrufC, a Cargo-inspired build system for C/C++. It also integrates Truffles other features, such as AI enabled optimizations. 
The Trufling models are able to read C/C++ source code, and generate optimal compiler flags for speed or safety. They will soon also be able to generate synthetic runtime data for a program to help the compiler further optimize the code. Currently, these models are improve the runtime performance of C by 5%-30% (average of 12%).


---

## 🔭 Scope
- Currently, Truffle only aims to support C/C++. Support for other systems level languages may come in the far future. 
- Truffle only supports Linux and Windows though WSL. MacOS support is on the roadmap. 

## 🚧 Work In Progress
**Note:** Truffle is in a very early state and nearly everything is currently under active development.
Key areas currently being worked on:
- **TrufC**: TrufC is the CLI tool that users will interact with when using Truffle to optimize their code. 
- **Trufling 1 70M**: The Trufling are the models that will produce synthetic runtime data and generate optimal compiler flags.

---

## Quick Start
- You will interact with Truffle using TrufC. Download TrufC using the following script. Change `--user` to `--system` to install TrufC system wide. 
```bash
curl https://raw.githubusercontent.com/akneni/truffle/refs/heads/main/trufc/scripts/install.py -o install.py
python3 install.py --user
```
- You can read the docs here: https://wistful-stone-a74.notion.site/TrufC-Documentation-138f4fffbb4980059e6bfd57a7ac89ba?pvs=4