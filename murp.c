#include "privates.h"

/* murp - the half arsed json-esque parser:
	murp's purpose is to be a small, almost pointless json parser.
	It picks out each key-value pair and identifies its type.
	It then passes this in to a callback that decides what to do with the data

	something like

	parse(json_string, callback_function);

	int callback_function(Atom atom) {
		static Actor *actor;
		if (atom->type == FIRST) {
			actor = malloc(sizeof(Actor));
		}

		if (atom->type == STR) {
			strcmp(atom->key, "firstname") || actor->firstname = atom->value;
		}

		push_queue(actor);
	}
*/

typedef struct callback_t {
	char *token;
	void *callback;
} callback_t;


void *get_callback(callback_t *c) {
	while (c->token != NULL) {
		puts(c->token);
		c++;
	}

	return NULL;
}

Atomizer print_atom(Atom atom, void *p) {
	printf("Atom {\n\tDatatype:\t%d\n\tContainer:\t%d\n\tKey:\t\t", atom.type, atom.container);
	int i;
	for (i = 0; i < atom.key.len; i++) {
		putchar(*(atom.key.start+i));
	}
	printf("\n\tValue:\t\t");
	for (i = 0; i < atom.value.len; i++) {
		putchar(*(atom.value.start+i));
	}
	puts("\n}\n");
	
	if(IS_ERROR(atom.type)) return BREAK;
	return CONTINUE;
}

Atomizer count_elems(Atom atom, void *probe) {
	if(IS_ERROR(atom.type)) return BREAK;
	
	int *count = probe;
	*count += 1;

	return CONTINUE;
}

int main(int argc, char *argv[]) {
	char *json = "{\"vix is teh sex\":\"SHE SURE IS SCAMP\", \"NUSHNUSH\": {12345[]}, \"ARRAY EXAMPLE\" : [],  \"well what do we have here\"    :      \"HOT men steal trunks\"}";

	Atomize(json, print_atom);

	int counter = 0;
	Probe(json, count_elems, &counter);

	printf("I counted %d elements in this string.\n", counter);
//	callback_t callbacks[] = {
//		{"token", NULL}, 
//		{"token2", NULL}
//	};

//	get_callback( (callback_t[]) {
//		{"token", NULL}, 
//      {"token2", NULL},
//		{NULL, NULL}
//	});

}
