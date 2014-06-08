murp is JSON in everything but name. This is in case I want to do soemthing different with it.

It is slight and designed for deserializing JSON-ish objects in to C structs. Therefore it doesn't impose too much of its own object system like other parsers. It pulls each key-value pair (arrays will have a NULL key) out of an object, identifies the type of the value and then asks you what to do with it via the callback function you pass to render. The object that represents key-value-type are called Atoms and the callback function takes a single Atom as a param. From here **you** consider what the Atom is and what to do with it. Drop its value in the correct member of a struct? Throw it away? Push the Atom to a linked list somewhere? Use murp to parse its value? Forget about it and leak memory? Missy, the choice is **yours**...

murp is named for the sound you make when you try to think of another name for something and fail.
