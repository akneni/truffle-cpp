# Use llvm-config to gather the necessary flags
LLVM_CXXFLAGS := `llvm-config --cxxflags`
LLVM_LDFLAGS := `llvm-config --ldflags`
LLVM_LIBS := `llvm-config --libs`

# Target to run the program
run:
	g++ src/main.cpp src/lexer.cpp src/parser.cpp src/scopr_tr.cpp src/dtype_utils.cpp src/code_gen.cpp -o main $(LLVM_CXXFLAGS) $(LLVM_LDFLAGS) $(LLVM_LIBS) -std=c++17 -fexceptions
	./main

build:
	g++ src/main.cpp src/lexer.cpp src/parser.cpp src/scopr_tr.cpp src/dtype_utils.cpp -o main $(LLVM_CXXFLAGS) $(LLVM_LDFLAGS) $(LLVM_LIBS) -std=c++17 -fexceptions

run-t:
	llc -filetype=obj truffle-main.ll -o truffle-main.o -relocation-model=pic
	clang truffle-main.o -o truffle-main -pie
	./truffle-main

clean:
	rm -f main
	rm -f truffle-main
	rm -f truffle-main.o
	rm -f truffle-main.ll
	rm -f truffle-main.bc
