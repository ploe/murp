#ifndef MURP_PUBLIC
#define MURP_PUBLIC
/*	public.h defines the global types and macros that a user might
	need to know.	

	Think of this junk as the public interface to murp.	*/


/*	public types	*/

typedef int Datatype;
enum {
	NOTSET,
	NIL,
	STRING,
	NUMBER,
	BOOLEAN,
	HASH,
	LIST,

	ERRORS = 100,
	ENDOFSTRING,
	NOQUOTEMARK,
	NONSENSE,
	END_OF_ERRORS,
};

/*	The Atom is what murp throws out on each iteration. It represents
	a JSON key-value pair. This is the block that the user parses in
	their callback code, which I'm calling an Atomizer function. 

	The Atom passed into the Atomizer function will only live as long
	as that Atomizer, whereupon the C strings attached to it will be freed.
	You will need to make a copy if you want those keeping.

	type is the JSON datatype of the value or a murp error code. This
	is so you can identify what'll need doing with it.

	key and value are C strings containing the values we ripped out of the
	object we're lexing.	*/

typedef struct Atom {
	Datatype type, container;
	char *key, *value;
} Atom;

/*	Status codes that our Atomizer function can yield. This tells the
	Lexer what to do, whether to keep on parsing or break.	*/
typedef int Atomizer;
enum {
	CONTINUE,
	BREAK,
	FAIL,
	WHYBOTHER,
};

/*	public functions	*/

/*	Atomize **is** **the** function for murp. Let me explain. When you
	pass in a JSON string and the Atomier function this tears through
	the string sussing out what each key-value pair is, and what type
	the value is.

	This means we can parse it with our Atomizer function.		*/
Atomizer Atomize(char *src, Atomizer (*callback)(Atom *));

/*	public macros	*/

#define IS_ERROR(i) ((i > ERRORS) && (i < END_OF_ERRORS))
#define SEND_ATOM NULL

#endif
