run:
	g++ src/main.cpp src/lexer.cpp src/parser.cpp src/scopr_tr.cpp -o main -std=c++17
	./main