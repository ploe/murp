#ifndef MURP_LEXER
#define MURP_LEXER

/*	The Lexer is the type that maintains the object we're parsing at
	the mo. It has a few methods for reading its state. They're for
	legibility because the values could be obtained by directly 
	fussing with start and len.

	The Grammar module is what uses the Lexer.

	Delimiters are the delimiters we've counted in 
	the current parsed object	*/

typedef struct {
	char *src, *start;
	unsigned int last, len;
	mp_Datatype container;
} _mp_Lexer;

/*	Lexer methods	*/

_mp_Lexer NewLexer(char *src, unsigned int last);
char _mp_Next(_mp_Lexer *lexer);
char _mp_Prev(_mp_Lexer *lexer);
char _mp_Peek(_mp_Lexer *lexer);
char _mp_PrevSteps(_mp_Lexer *lexer, unsigned int steps);
char _mp_Ignore(_mp_Lexer *lexer, char *valid);
char _mp_Fear(_mp_Lexer *lexer, char *valid);
int _mp_Match(_mp_Lexer *lexer, char *valid);
void _mp_Ditch(_mp_Lexer *lexer);

/* Lexer macros	*/

#define _mp_END_OF_STRING(c) ((c == EOF) || (c == '\0'))

#endif
