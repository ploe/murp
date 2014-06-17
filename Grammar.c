#include "privates.h"

char *WHITESPACE = " \t\n\r\v\f";

char *Store(Lexer *lexer) {
	char *str = calloc(lexer->len + 1, sizeof(char));
	strncpy(str, lexer->start, lexer->len - 1);
	Ditch(lexer);

	return str;
}

Slice Dispatch(Lexer *lexer) {
	Slice val = {lexer->start, lexer->len - 1};
	Ditch(lexer);

	return val;
}

static void *GetStringValue(Lexer *lexer, Atom *atom) {
	Fear(lexer, "\"");
	if (PrevSteps(lexer, 2) == '\\') {
	}
	
	atom->type = STRING;
	atom->value = Dispatch(lexer);
	return SEND_ATOM;
}

static void *GetValue(Lexer *lexer, Atom *atom) {
	char c = Ignore(lexer, WHITESPACE);

	if (c == '\"') {
		Ditch(lexer);
		return GetStringValue;
	}

	atom->type = NONSENSE;
	return SEND_ATOM;	
}

static void *GetKeyBody(Lexer *lexer, Atom *atom) {
	Fear(lexer, "\"");
	if (PrevSteps(lexer, 2) == '\\') return *GetKeyBody;

	atom->key = Dispatch(lexer);
	
	char c = Ignore(lexer, WHITESPACE);
	switch (c) {
		case ':': return GetValue; break;
	}

	atom->type = NONSENSE;
	return SEND_ATOM;
}

static void *NextHashElement(Lexer *lexer, Atom *atom) {
	char c = Ignore(lexer, WHITESPACE);

	if (c == '\"') {
		Ditch(lexer);
		return GetKeyBody;
	}
	else if(END_OF_STRING(c) || c == '}') {
		atom->type = ENDOFSTRING;
	}
	else {
		atom->type = NOQUOTEMARK;
		atom->value = (Slice) {lexer->start, 5};
	}
	return SEND_ATOM;
}

static void *FindNextHashElement(Lexer *lexer, Atom *atom) {
	/*	skip all the nonsense till we find a comma	*/
	atom->container = HASH;
	char c = Ignore(lexer, WHITESPACE);
	if(c == ',') {
		Next(lexer);
		Ditch(lexer);
		return NextHashElement;
	}

	atom->type = ENDOFSTRING;
	return SEND_ATOM;
}

void *Identify(Lexer *lexer, Atom *atom) {
	// ignore all trailing whitespace
	char c = Ignore(lexer, WHITESPACE);

	Ditch(lexer);
	switch(c) {
		case '{':
			atom->container = HASH;
			return NextHashElement;	
		break;

		case '[': puts("parse array"); break;
	}
	
	atom->type = ENDOFSTRING; 
	return SEND_ATOM;
}

Atomizer Atomize(char *src, Atomizer (*callback)(Atom)) {
	if(!callback) return WHYBOTHER;

	Lexer *lexer = NewLexer(src);
	void * (*grammar)(Lexer *, Atom *) = Identify;

	int status = CONTINUE;
	while (status == CONTINUE) {	
		Atom atom = {NOTSET, NOTSET, {NULL, 0}, {NULL, 0}};
		for (; grammar; grammar = grammar(lexer, &atom));
		status = callback(atom);

		/*	we bust out if the atom type is an exit code	*/
		if (IS_ERROR(atom.type)) {
			status = BREAK;	
		}
		else if (status == CONTINUE) {
			if (atom.container == HASH) grammar = FindNextHashElement;
			else if(atom.container == LIST) puts("NextArrayElement");
		}
	}

	free(lexer);
	return status;
}
