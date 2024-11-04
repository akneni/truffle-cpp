apt install llvm-dev -y
apt install clang -y
git clone https://github.com/llvm/llvm-project.git
mkdir bolt-build
cd bolt-build
cmake -G Ninja ../llvm-project/llvm -DLLVM_TARGETS_TO_BUILD="X86;AArch64" -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_ASSERTIONS=ON -DLLVM_ENABLE_PROJECTS="bolt"
ninja bolt