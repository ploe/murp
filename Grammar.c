#include "privates.h"

char *WHITESPACE = " \t\n\r\v\f";

char *Store(Lexer *lexer) {
	char *str = calloc(lexer->len + 1, sizeof(char));
	strncpy(str, lexer->start, lexer->len - 1);
	Ditch(lexer);

	return str;
}

Slice DispatchString(Lexer *lexer) {
	Slice val = {lexer->start, lexer->len - 1};
	Ditch(lexer);

	return val;
}

Slice DispatchContainer(Lexer *lexer) {
	Slice val = {lexer->start - 1, lexer->len+1};
        Ditch(lexer);

        return val;
}

Slice DispatchCustom(Lexer *lexer, char *start, unsigned int len) {
	Slice val = {start, len};
        Ditch(lexer);

        return val;
}

static void *GetStringValue(Lexer *lexer, Atom *atom) {
	Fear(lexer, "\"");
	if (PrevSteps(lexer, 2) == '\\') {
	}
	
	atom->type = STRING;
	atom->value = DispatchString(lexer);
	return SEND_ATOM;
}

/* Hm... this code smells a little funky... */
static void *GetContainerValue(Lexer *lexer, Atom *atom) {
	char c = Fear(lexer, "{}[]\"");
	if (c == '\"') {
		if (PrevSteps(lexer, 2) != '\\') {
			if (lexer->quotes.open == lexer->quotes.closed) lexer->quotes.open += 1;
			else lexer->quotes.closed += 1;
		}
	}
	if (lexer->quotes.open > lexer->quotes.closed) return GetContainerValue;

	if (c == '{') lexer->curlies.open += 1;
	else if (c == '}') lexer->curlies.closed += 1;
	else if (c == '[') lexer->squares.open += 1;
	else if (c == ']') lexer->squares.closed += 1;

	DelimitMatch(lexer);
	if (AllDelimited(lexer)) {
		if (DelimitMatch(lexer) == atom->type) {
			atom->value = DispatchContainer(lexer);
		}
		else {
			atom->type = MANGLEDCONTAINER;
			atom->value = DispatchContainer(lexer);
		}

		return SEND_ATOM;
	}

	if (END_OF_STRING(c)) {
		atom->type = UNFINISHEDVALUE;
		atom->value = DispatchCustom(lexer, lexer->start-1, lexer->len);
                return SEND_ATOM;
	}
	
	return GetContainerValue;
}

static void *GetValue(Lexer *lexer, Atom *atom) {
	char c = Ignore(lexer, WHITESPACE);

	if (c == '\"') {
		Ditch(lexer);
		return GetStringValue;
	}
	else if (c == '{') {
		Ditch(lexer);
		atom->type = OBJECT;
		lexer->curlies.open += 1;
		return GetContainerValue;
	}
	else if (c == '[') {
		Ditch(lexer);
                atom->type = ARRAY;
                lexer->squares.open += 1;
                return GetContainerValue;
	}

	atom->type = NONSENSE;
	return SEND_ATOM;	
}

static void *GetKeyBody(Lexer *lexer, Atom *atom) {
	Fear(lexer, "\"");
	if (PrevSteps(lexer, 2) == '\\') return *GetKeyBody;

	atom->key = DispatchString(lexer);
	
	char c = Ignore(lexer, WHITESPACE);
	switch (c) {
		case ':': return GetValue; break;
	}

	atom->type = NONSENSE;
	return SEND_ATOM;
}

static void *NextObjectElement(Lexer *lexer, Atom *atom) {
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

static void *FindNextObjectElement(Lexer *lexer, Atom *atom) {
	/*	skip all the nonsense till we find a comma	*/
	atom->container = OBJECT;
	char c = Ignore(lexer, WHITESPACE);
	if(c == ',') {
		Next(lexer);
		Ditch(lexer);
		return NextObjectElement;
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
			atom->container = OBJECT;
			return NextObjectElement;	
		break;

		case '[': puts("parse array");
		break;
	}
	
	atom->type = ENDOFSTRING; 
	return SEND_ATOM;
}

Atomizer lex(Lexer *lexer, Atomizer (*callback)(Atom, void *), void *probe) {
	if(!callback) return WHYBOTHER;

	void * (*grammar)(Lexer *, Atom *) = Identify;	
	int status = CONTINUE;
	while (status == CONTINUE) {	
		Atom atom = {NOTSET, NOTSET, {NULL, 0}, {NULL, 0}};
		for (; grammar; grammar = grammar(lexer, &atom));
		status = callback(atom, probe);

		/*	we bust out if the atom type is an exit code	*/
		if (IS_ERROR(atom.type)) {
			status = BREAK;	
		}
		else if (status == CONTINUE) {
			if (atom.container == OBJECT) grammar = FindNextObjectElement;
			else if(atom.container == ARRAY) puts("NextArrayElement");
		}
	}

	return status;
} 

Atomizer ProbeSlice(Slice *src, Atomizer (*callback)(Atom, void *), void *probe) {
	Lexer lexer = NewLexer(src->start, src->len);
	return lex(&lexer, callback, probe);
}

Atomizer Probe(char *src, Atomizer (*callback)(Atom, void *), void *probe) {
	Lexer lexer = NewLexer(src, 0);
	return lex(&lexer, callback, probe);
}
