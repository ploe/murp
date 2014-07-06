# What is murp?

murp is JSON in everything but name. This is in case I want to do soemthing different with it.

It is slight and designed for deserializing JSON-ish objects in to C structs. Therefore it doesn't impose too much of its own object system like other parsers. It pulls each key-value pair (arrays will have a NULL key) out of an object, identifies the type of the value and then asks you what to do with it via the callback function you pass to render. The object that represents key-value-type are called Atoms and the callback function takes a single Atom as a param. From here **you** consider what the Atom is and what to do with it. Drop its value in the correct member of a struct? Throw it away? Push the Atom to a linked list somewhere? Use murp to parse its value? Forget about it and leak memory? Missy, the choice is **yours**...

murp is named for the sound you make when you try to think of a name for something and fail.

# Dynamic-sounding names

Most libs have boring, cumbersome interfaces. Not murp! We hand you a small set of features (a handful of functions and types) with almost imperative sounding names. We want you to visualise what our code does, without you having to look at it for too long. If it's not immediately obvious, you're doing it wrong. You'll find more of this sort proselytising in the git log for the project.
