#include "privates.h"

void dumpLexer(Lexer *lexer) {
	puts("Lexer {");
	printf("\tsrc:\t%s\n", lexer->src);
	printf("\tbuff:\t%.*s\n", lexer->len, lexer->start);
	printf("\tstart:\t%lu\n",  (lexer->start-lexer->src));
	printf("\tlen:\t%d\n", lexer->len);
	printf("\tnext:\t'%c'\n", Peek(lexer));
	puts("}");
}

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

static char NestedObject(Lexer *lexer, Atom *atom);
static char NestedArray(Lexer *lexer, Atom *atom);
static char NestedQuote(Lexer *lexer, Atom *atom);
inline char NestedCallbacks(char c, Lexer *lexer, Atom *atom); 

/*	inline because I don't want all these vars pushing to the stack 
as we drill down in to the nested objects, I can imagine it could get 
quite expensive	*/
char NestedCallbacks(char c, Lexer *lexer, Atom *atom) {
	switch(c) {
                case '{':
                        return NestedObject(lexer, atom);
                break;

                case '[':
                        return NestedArray(lexer, atom);
                break;

                case '\"':
                        return NestedQuote(lexer, atom);
                break;
        }

	return c;
}

#define OPEN_NESTED(c) ((c == '{') || (c == '[') || (c == '\"'))

static void *GetObjectValue(Lexer *lexer, Atom *atom) {
	char c = NestedObject(lexer, atom);

	if (END_OF_STRING(c)) {
		atom->type = EOVALUE;
		atom->value = DispatchCustom(lexer, lexer->start-1, lexer->len);
	}
	else if (c == '}') {
		atom->value = DispatchContainer(lexer);
	}

	return SEND_ATOM;
}

static void *GetArrayValue(Lexer *lexer, Atom *atom) {
	char c = NestedArray(lexer, atom);

	if (END_OF_STRING(c)) {
		atom->type = EOVALUE;
		atom->value = DispatchCustom(lexer, lexer->start-1, lexer->len);
	}
	else if (c == ']') {
		atom->value = DispatchContainer(lexer);
	}

	return SEND_ATOM;
}

static char NestedObject(Lexer *lexer, Atom *atom) {
	char c;
	do {
		c = Fear(lexer, "{}[\"");
		if (OPEN_NESTED(c)) {
			c = NestedCallbacks(c, lexer, atom);
			if (c == '}') c = Next(lexer);
		}
	} while ((c != '}') && !(END_OF_STRING(c)));

	return c;
}

static char NestedArray(Lexer *lexer, Atom *atom) {
	char c;
	do {
		c = Fear(lexer, "{[]\"");
		if (OPEN_NESTED(c)) {
			c = NestedCallbacks(c, lexer, atom);
			if (c == ']') c = Next(lexer);
		}
	} while ((c != ']') && !(END_OF_STRING(c)));

	return c;
}

static char NestedQuote(Lexer *lexer, Atom *atom) {
	char c;
	do {
		c = Fear(lexer, "\"");
	} while ((c != '\"') && !(END_OF_STRING(c)));

	return c;
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
		return GetObjectValue;
	}
	else if (c == '[') {
		Ditch(lexer);
                atom->type = ARRAY;
                return GetArrayValue;
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
	Lexer lexer = NewLexer(src, strlen(src));
	return lex(&lexer, callback, probe);
}
