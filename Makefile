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

clean:
	rm -f main
