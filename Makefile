run:
	g++ src/main.cpp src/lexer.cpp src/parser.cpp src/scopr_tr.cpp src/dtype_utils.cpp -o main -std=c++17
	./main