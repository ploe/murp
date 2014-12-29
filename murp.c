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

mp_Atomizer print_atom(mp_Atom atom, void *p) {
	printf("Atom {\n\tDatatype:\t%d\n\tContainer:\t%d\n", atom.type, atom.container);
	printf("\tKey:\t%.*s\n", atom.key.len, atom.key.start);
	printf("\tValue:\t%.*s\n", atom.value.len, atom.value.start);
	puts("\n}\n");
	
	if(mp_IS_ERROR(atom.type)) return mp_BREAK;
	return mp_CONTINUE;
}

mp_Atomizer count_elems(mp_Atom atom, void *probe) {
	if(mp_IS_ERROR(atom.type)) return mp_BREAK;
	
	int *count = probe;
	*count += 1;

	return mp_CONTINUE;
}

int main(int argc, char *argv[]) {
	char *json = "{\"dis be trooo\" : true, \"dix bwe fuls\" : false, \"vix is teh sex\":\"SHE SURE IS SCAMP\", \"NUSHNUSH\": {\"hello\" : \"world\", \"nested array\" : [[1, 2, 3, 4], {}]}, \"ARRAY EXAMPLE\" : [{123}\"\"],  \"well what do we have here\"    :      \"HOT men steal trunks\", \"und null?\" : null }";
		

	mp_Atomize(json, print_atom);

	int counter = 0;
	mp_Probe(json, count_elems, &counter);

	printf("I counted %d elements in this string.\n", counter);
}
