#ifdef __cplusplus
extern "C" {
#endif

#ifndef MURP_PUBLIC
#define MURP_PUBLIC
/*	public.h defines the global types and macros that a user might
	need to know.	

	Think of this junk as the public interface to murp.	*/


/*	public types	*/

typedef int mp_Datatype;
enum {
	mp_NOTSET,
	mp_NULL,
	mp_STRING,
	mp_NUMBER,
	mp_BOOLEAN,
	mp_OBJECT,
	mp_ARRAY,

	_mp_ERRORS = 9000,
	mp_DONE,
	mp_EOSTRUCT,
	mp_EOKEY,
	mp_EOVALUE,
	_mp_END_OF_ERRORS,
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

typedef struct {
	char *start;
	unsigned int len;
} mp_Slice;

typedef struct {
	mp_Datatype type, container;
	mp_Slice key, value;
} mp_Atom;

/*	Status codes that our Atomizer function can yield. This tells the
	Lexer what to do, whether to keep on parsing or break.	*/
typedef int mp_Atomizer;
enum {
	mp_CONTINUE,
	mp_BREAK,
	mp_FAIL,
	mp_WHYBOTHER,
};

/*	public functions	*/

/*	Atomize **is** **the** function for murp. Let me explain. When you
	pass in a JSON string and the Atomier function this tears through
	the string sussing out what each key-value pair is, and what type
	the value is.

	This means we can parse it with our Atomizer function.		*/
mp_Atomizer mp_Probe(char *src, mp_Atomizer (*callback)(mp_Atom, void *), void *);
#define mp_Atomize(src, callback) mp_Probe(src, callback, NULL)

mp_Atomizer mp_ProbeSlice(mp_Slice *src, mp_Atomizer (*callback)(mp_Atom, void *), void *probe);
#define mp_AtomizeSlice(src, callback) mp_ProbeSlice(src, callback, NULL)

/*	public macros	*/

#define mp_IS_ERROR(i) ((i > _mp_ERRORS) && (i < _mp_END_OF_ERRORS))

#endif

#ifdef __cplusplus
}
#endif


