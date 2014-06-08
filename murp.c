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

Atomizer print_atom(Atom *atom) {
	printf("Atom {\n\tDatatype:\t%d\n\tContainer:\t%d\n\tKey:\t\t%s\n\tValue:\t\t%s\n}\n\n", atom->type, atom->container, atom->key, atom->value);
	if(IS_ERROR(atom->type)) return BREAK;
	return CONTINUE;
}

int main(int argc, char *argv[]) {
	Atomize("{\"vix is teh sex\":\"SHE SURE IS SCAMP\"}", print_atom);
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
