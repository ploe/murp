#include <stdio.h>
#include "murp.h"

/* murp - the half arsed json-esque parser:
	murp's purpose is to be a small, almost pointless json parser.
	It picks out each key-value pair and identifies its type.
	It then passes this in to a callback that decides what to do with the data
*/

mp_Atomizer print_atom(mp_Atom atom, void *p) {
	printf("Atom {\n\tDatatype:\t%d\n\tContainer:\t%d\n", atom.type, atom.container);
	if(atom.key.start) printf("\tKey:\t%.*s\n", atom.key.len, atom.key.start);
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
	char *json = "{\"dis be trooo\" : true, \"dix bwe fuls\" : false, \"vix is teh sex\":\"SHE SURE IS SCAMP\", \"NUSHNUSH\": {\"hello\" : \"world\", \"nested array\" : [[1, 2, 3, 4], {}]}, \"ARRAY EXAMPLE\" : [{123}\"\"],  \"well what do we have here\"    :      \"HOT men steal trunks\", \"und null?\" : null , \"noomboo\" : 215555 , \"nomboh\" : -1234567e-1000	,}";
		
	json = "[\"element\", 1234, true, false, null, 3.14, 9.99e-2, {\"test\" : 12345, \"urk\" : [1, 2, 3, 4]}, [1, 2, 3, 4, 5]]";

	mp_Atomize(json, print_atom);

	int counter = 0;
	mp_Probe(json, count_elems, &counter);

	printf("I counted %d elements in this string.\n", counter);
}
