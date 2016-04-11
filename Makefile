Main: Tokenizer.o Parser.o Lexer.o main.cpp regex.cpp
	g++ -std=c++0x -o Aql main.cpp Tokenizer.o Parser.o Lexer.o -O3

Tokenizer.o: Tokenizer.cpp Tokenizer.h
	g++ -std=c++0x -c Tokenizer.cpp -O3

Lexer.o: Lexer.cpp Lexer.h
	g++ -std=c++0x -c Lexer.cpp -O3

Parser.o: Parser.cpp Parser.h
	g++ -std=c++0x -c Parser.cpp -O3

clean:
	rm *.o
