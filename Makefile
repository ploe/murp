all:
	cc -c -O2 Grammar.c Lexer.c
	ld -r -o murp.o Grammar.o Lexer.o
	rm -vf Grammar.o Lexer.o
