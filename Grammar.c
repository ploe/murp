#include "privates.h"

void dumpLexer(_mp_Lexer *lexer) {
	puts("Lexer {");
	printf("\tsrc:\t%s\n", lexer->src);
	printf("\tbuff:\t%.*s\n", lexer->len, lexer->start);
	printf("\tstart:\t%lu\n",  (lexer->start-lexer->src));
	printf("\tlen:\t%d\n", lexer->len);
	printf("\tnext:\t'%c'\n", _mp_Peek(lexer));
	puts("}");
}

char *_mp_WHITESPACE = " \t\n\r\v\f";

char *Store(_mp_Lexer *lexer) {
	char *str = calloc(lexer->len + 1, sizeof(char));
	strncpy(str, lexer->start, lexer->len - 1);
	_mp_Ditch(lexer);

	return str;
}

mp_Slice DispatchString(_mp_Lexer *lexer) {
	mp_Slice val = {lexer->start, lexer->len - 1};
	_mp_Ditch(lexer);

	return val;
}

mp_Slice DispatchContainer(_mp_Lexer *lexer) {
	mp_Slice val = {lexer->start - 1, lexer->len+1};
	_mp_Ditch(lexer);

	return val;
}

mp_Slice DispatchCustom(_mp_Lexer *lexer, char *start, unsigned int len) {
	mp_Slice val = {start, len};
	_mp_Ditch(lexer);

	return val;
}

mp_Slice DispatchPattern(_mp_Lexer *lexer, char *value) {
	unsigned int len = strlen(value);
    mp_Slice val = {lexer->start-1, len};
	lexer->len += len-1;
    _mp_Ditch(lexer);

    return val;
}

static void *GetStringValue(_mp_Lexer *lexer, mp_Atom *atom) {
	_mp_Fear(lexer, "\"");
	if (_mp_PrevSteps(lexer, 2) == '\\') {
	}
	
	atom->type = mp_STRING;
	atom->value = DispatchString(lexer);
	return _mp_SEND_ATOM;
}

static void *GetBooleanValue(_mp_Lexer *lexer, mp_Atom *atom) {
	if (_mp_Match(lexer, "true")) atom->value = DispatchPattern(lexer, "true");
	else if (_mp_Match(lexer, "false")) atom->value = DispatchPattern(lexer, "false");

	else atom->type = mp_EOVALUE;
	return _mp_SEND_ATOM;
}


static char NestedObject(_mp_Lexer *lexer, mp_Atom *atom);
static char NestedArray(_mp_Lexer *lexer, mp_Atom *atom);
static char NestedQuote(_mp_Lexer *lexer, mp_Atom *atom);
static inline char NestedCallbacks(char c, _mp_Lexer *lexer, mp_Atom *atom); 

/*	inline because I don't want all these vars pushing to the stack 
as we drill down in to the nested objects, I can imagine it could get 
quite expensive	*/
static inline char NestedCallbacks(char c, _mp_Lexer *lexer, mp_Atom *atom) {
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

static void *GetObjectValue(_mp_Lexer *lexer, mp_Atom *atom) {
	char c = NestedObject(lexer, atom);

	if (_mp_END_OF_STRING(c)) {
		atom->type = mp_EOVALUE;
		atom->value = DispatchCustom(lexer, lexer->start-1, lexer->len);
	}
	else if (c == '}') {
		atom->value = DispatchContainer(lexer);
	}

	return _mp_SEND_ATOM;
}

static void *GetArrayValue(_mp_Lexer *lexer, mp_Atom *atom) {
	char c = NestedArray(lexer, atom);

	if (_mp_END_OF_STRING(c)) {
		atom->type = mp_EOVALUE;
		atom->value = DispatchCustom(lexer, lexer->start-1, lexer->len);
	}
	else if (c == ']') {
		atom->value = DispatchContainer(lexer);
	}

	return _mp_SEND_ATOM;
}

static char NestedObject(_mp_Lexer *lexer, mp_Atom *atom) {
	char c;
	do {
		c = _mp_Fear(lexer, "{}[\"");
		if (OPEN_NESTED(c)) {
			c = NestedCallbacks(c, lexer, atom);
			if (c == '}') c = _mp_Next(lexer);
		}
	} while ((c != '}') && !(_mp_END_OF_STRING(c)));

	return c;
}

static char NestedArray(_mp_Lexer *lexer, mp_Atom *atom) {
	char c;
	do {
		c = _mp_Fear(lexer, "{[]\"");
		if (OPEN_NESTED(c)) {
			c = NestedCallbacks(c, lexer, atom);
			if (c == ']') c = _mp_Next(lexer);
		}
	} while ((c != ']') && !(_mp_END_OF_STRING(c)));

	return c;
}

static char NestedQuote(_mp_Lexer *lexer, mp_Atom *atom) {
	char c;
	do {
		c = _mp_Fear(lexer, "\"");
	} while ((c != '\"') && !(_mp_END_OF_STRING(c)));

	return c;
}

static void *GetValue(_mp_Lexer *lexer, mp_Atom *atom) {
	char c = _mp_Ignore(lexer, _mp_WHITESPACE);

	if (c == '\"') {
		_mp_Ditch(lexer);
		return GetStringValue;
	}
	else if (c == '{') {
		_mp_Ditch(lexer);
		atom->type = mp_OBJECT;
		return GetObjectValue;
	}
	else if (c == '[') {
		_mp_Ditch(lexer);
		atom->type = mp_ARRAY;
		return GetArrayValue;
	}
	else if (c == 't' || c == 'f') {
		_mp_Ditch(lexer);
		atom->type = mp_BOOLEAN;
		return GetBooleanValue;
	}
	else if (c == 'n') {
		 _mp_Ditch(lexer);
		 atom->type = mp_NULL;
		if (_mp_Match(lexer, "null")) { 
			atom->value = DispatchPattern(lexer, "null");
			return _mp_SEND_ATOM;
		}
	}
	

	atom->type = mp_EOVALUE;
	return _mp_SEND_ATOM;	
}

static void *GetKeyBody(_mp_Lexer *lexer, mp_Atom *atom) {
	_mp_Fear(lexer, "\"");
	if (_mp_PrevSteps(lexer, 2) == '\\') return *GetKeyBody;

	atom->key = DispatchString(lexer);
	
	char c = _mp_Ignore(lexer, _mp_WHITESPACE);
	switch (c) {
		case ':': return GetValue; break;
	}

	atom->type = mp_EOKEY;
	return _mp_SEND_ATOM;
}

static void *NextObjectElement(_mp_Lexer *lexer, mp_Atom *atom) {
	char c = _mp_Ignore(lexer, _mp_WHITESPACE);

	if (c == '\"') {
		_mp_Ditch(lexer);
		return GetKeyBody;
	}
	else if (_mp_END_OF_STRING(c)) {
		atom->type = mp_EOSTRUCT;
	}
	else if (c == '}') {
		atom->type = mp_DONE;
	}
	else {
		atom->type = mp_EOKEY;
		atom->value = (mp_Slice) {lexer->start, 5};
	}
	return _mp_SEND_ATOM;
}

static void *FindNextObjectElement(_mp_Lexer *lexer, mp_Atom *atom) {
	/*	skip all the nonsense till we find a comma	*/
	atom->container = mp_OBJECT;
	char c = _mp_Ignore(lexer, _mp_WHITESPACE);
	if(c == ',') {
		_mp_Next(lexer);
		_mp_Ditch(lexer);
		return NextObjectElement;
	}
	else if(c == '}') {
		atom->type = mp_DONE;
	}
	else {
		atom->type = mp_EOSTRUCT;
	}

	return _mp_SEND_ATOM;
}

static void *Identify(_mp_Lexer *lexer, mp_Atom *atom) {
	// ignore all trailing whitespace
	char c = _mp_Ignore(lexer, _mp_WHITESPACE);

	_mp_Ditch(lexer);
	switch(c) {
		case '{':
			atom->container = mp_OBJECT;
			return NextObjectElement;	
		break;

		case '[': puts("parse array");
		break;
	}
	
	atom->type = mp_EOSTRUCT; 
	return _mp_SEND_ATOM;
}

mp_Atomizer lex(_mp_Lexer *lexer, mp_Atomizer (*callback)(mp_Atom, void *), void *probe) {
	if(!callback) return mp_WHYBOTHER;

	void * (*grammar)(_mp_Lexer *, mp_Atom *) = Identify;	
	int status = mp_CONTINUE;
	while (status == mp_CONTINUE) {	
		mp_Atom atom = {mp_NOTSET, mp_NOTSET, {NULL, 0}, {NULL, 0}};
		for (; grammar; grammar = grammar(lexer, &atom));
		status = callback(atom, probe);
		/*	we bust out if the atom type is an exit code	*/
		if (mp_IS_ERROR(atom.type)) {
			status = mp_BREAK;	
		}
		else if (status == mp_CONTINUE) {
			if (atom.container == mp_OBJECT) grammar = FindNextObjectElement;
			else if(atom.container == mp_ARRAY) puts("NextArrayElement");
		}
	}

	return status;
} 

mp_Atomizer mp_ProbeSlice(mp_Slice *src, mp_Atomizer (*callback)(mp_Atom, void *), void *probe) {
	_mp_Lexer lexer = NewLexer(src->start, src->len);
	return lex(&lexer, callback, probe);
}

mp_Atomizer mp_Probe(char *src, mp_Atomizer (*callback)(mp_Atom, void *), void *probe) {
	_mp_Lexer lexer = NewLexer(src, strlen(src));
	return lex(&lexer, callback, probe);
}
