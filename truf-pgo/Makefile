build:
	@gcc -o main -O3 src/main.c -lm

build-bolt:
	@make gp-inst
	@./bolt-build/bin/llvm-bolt ./main -o main.bolt -data=perf.fdata -reorder-blocks=ext-tsp -reorder-functions=hfsort -split-functions -split-all-cold -dyno-stats

build-gcov:
	@make gen-prof-data-gcov
	@make build-from-gcov

build-llvm-pgo:
	@clang -O3 -fprofile-instr-generate -fcoverage-mapping -o main-inst.lpgo src/main.c
	@export LLVM_PROFILE_FILE="main.profraw"
	@./main-inst.lpgo
	@rm main-inst.lpgo
	@mv default.profraw main.profraw
	@llvm-profdata merge -output=main.profdata main.profraw
	@clang -O3 -fprofile-instr-use=main.profdata -o main.lpgo src/main.c

test:
	@echo "./main\n========================================"
	@./main
	@echo "\n\n\n"
	@echo "./main.bolt\n==================================="
	@./main.bolt
	@echo "\n\n\n"
	@echo "./main.gcov\n==================================="
	@./main.gcov
	@echo "\n\n\n"
	@echo "./main.lpgo\n==================================="
	@./main.lpgo

# ===================================================================================================================

build-c:
	@gcc -fno-reorder-blocks-and-partition -fno-omit-frame-pointer -Wl,--emit-relocs -o main -O3 src/main.c -lm

gp-perf:
	@make build-c
	@perf record -e cycles:u -j any,u -o perf.data -- ./main
	@./bolt-build/bin/perf2bolt -p perf.data -o perf.fdata --ignore-build-id ./main

gp-inst:
	@make build-c
	@./bolt-build/bin/llvm-bolt ./main -instrument -o ./main-inst
	@./main-inst
	@rm ./main-inst
	@cp /tmp/prof.fdata prof.fdata
	@mv prof.fdata perf.fdata

gen-prof-data-gcov:
	@cp src/main.c main.c
	@gcc -O3 -fprofile-arcs -ftest-coverage -o main main.c -lm
	@rm main.c
	@./main

build-from-gcov:
	@cp src/main.c main.c
	@gcc -O3 -fprofile-use -o main.gcov main.c -lm
	@rm main.c

clean:
	- rm *.data
	- rm *.data.old
	- rm *.fdata
	- rm *.bolt
	- rm perf.*
	- rm main*
	- rm *.gcda
	- rm *.gcno
