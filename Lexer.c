#include "murp_privates.h"
#define LEXER_OVERFLOW(l) (l->last && (l->len > l->last))

_mp_Lexer NewLexer(char *src, unsigned int last) {
	_mp_Lexer lexer;
	lexer.src = lexer.start = src;
	lexer.last = last;
	lexer.len = 0;

	return lexer;
}

// next pops the next letter out of our lexer
char _mp_Next(_mp_Lexer *lexer) {
	char c = *(lexer->start + lexer->len);
	lexer->len += 1;

	if (LEXER_OVERFLOW(lexer)) {
		lexer->len = lexer->last;
		return '\0';
	}

	return c;
}

// look back at the last letter
char _mp_Prev(_mp_Lexer *lexer) {
	if (lexer->len <= 0) return '\0';
	return *(lexer->start + lexer->len - 1);
}

// Peek takes a little look-ahead so we know what the next letter is
char _mp_Peek(_mp_Lexer *lexer) {
	if (lexer->last && ((lexer->len + 1) > lexer->last)) return '\0';
	return *(lexer->start + lexer->len);
}

// looks back a number of steps in the current token, we can't look
// beyond the start otherwise we get a null char
char _mp_PrevSteps(_mp_Lexer *lexer, unsigned int steps) {
	char *c = lexer->start - steps;
	if (c < lexer->start) return '\0';
	return *c;
}


// accepts a string of the characters that we can just wash over
static int accept(_mp_Lexer *lexer, char *valid) {
	char c = _mp_Next(lexer);
	for (; *valid != '\0'; valid++) {
		if(c == *valid) return YES;
	}

	return NO;
}

char _mp_Ignore(_mp_Lexer *lexer, char *valid) {
	while (accept(lexer, valid)) {
		char c = _mp_Peek(lexer);
		if (_mp_END_OF_STRING(c)) return c;
	}
	return _mp_Prev(lexer);
}

char _mp_Fear(_mp_Lexer *lexer, char *valid) {
	while (!accept(lexer, valid)) {
		char c = _mp_Peek(lexer);
		if (_mp_END_OF_STRING(c)) return c;
	}
	return _mp_Prev(lexer);
}

int _mp_Match(_mp_Lexer *lexer, char *valid) {
	/*  start-1 so we can match patterns "true", "false" and "null" */
	return !(strncmp(valid, lexer->start-1, strlen(valid)));
}

/* moves the lexer's forward */
void _mp_Ditch(_mp_Lexer *lexer) {
	lexer->start += lexer->len;
	lexer->len = 0;
}

#undef LEXER_OVERFLOW
