# Use llvm-config to gather the necessary flags
LLVM_CXXFLAGS := `llvm-config --cxxflags`
LLVM_LDFLAGS := `llvm-config --ldflags`
LLVM_LIBS := `llvm-config --libs`

# Target to run the program
run:
	cargo run -q --release
	g++ -O3 -fno-reorder-blocks-and-partition -fno-omit-frame-pointer -Wl,--emit-relocs src-cpp/main.cpp src-cpp/lexer.cpp src-cpp/parser.cpp src-cpp/scopr_tr.cpp src-cpp/dtype_utils.cpp src-cpp/code_gen.cpp -o main $(LLVM_CXXFLAGS) $(LLVM_LDFLAGS) $(LLVM_LIBS) -std=c++17 -fexceptions
	./main

run-t:
	llc -filetype=obj truffle-main.ll -o truffle-main.o -relocation-model=pic
	clang truffle-main.o -o truffle-main -pie
	./truffle-main

clean:
	- rm -f main
	- rm -f truffle-main
	- rm -f truffle-main.o
	- rm -f truffle-main.ll
	- rm -f truffle-main.bc
	- rm main.bolt
	- rm perf.*

build-bolt:
	- rm main.bolt
	- rm perf.*
	@echo "\n\n\n"
	perf record -e cache-misses:u -o perf.data -- ./main
	/home/aknen/Documents/build/bin/perf2bolt -p perf.data -o perf.fdata -nl ./main
	/home/aknen/Documents/build/bin/llvm-bolt ./main -o main.bolt -data=perf.fdata -reorder-blocks=ext-tsp -reorder-functions=hfsort -split-functions -split-all-cold -dyno-stats

test:
	./main
	@echo "\n\n\n"
	./main.bolt
