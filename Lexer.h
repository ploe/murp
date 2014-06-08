#ifndef MURP_LEXER
#define MURP_LEXER

/*	The Lexer is the type that maintains the object we're parsing at
	the mo. It has a few methods for reading its state. They're for
	legibility because the values could be obtained by directly 
	fussing with start and len.

	The Grammar module is what uses the Lexer. */

typedef struct Lexer {
	char *src, *start;
	unsigned int len;
} Lexer;

/*	Lexer methods	*/

Lexer *NewLexer(char *src);
char Next(Lexer *lexer);
char Prev(Lexer *lexer);
char Peek(Lexer *lexer);
char PrevSteps(Lexer *lexer, unsigned int steps);
char Ignore(Lexer *lexer, char *valid);
char Fear(Lexer *lexer, char *valid);
void Ditch(Lexer *lexer);

/* Lexer macros	*/

#define END_OF_STRING(c) ((c == EOF) || (c == '\0'))

#endif
