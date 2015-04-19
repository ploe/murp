## murp - what is it?

![murp](http://ploe.co.uk/murp-logo.gif)

murp is a JSON lexer. It isn't a parser because I believe that you, the programmer, should be in charge of your heap. It presents a simple interface called Atomize, which you drop your parse function in to. 

If you're wanting to keep state there's an interface called Probe which works like Atomize but lets you drop an object in too. Any object... This is C after all.

You'll notice that serialization has been distinctly left out. That's because you know the shape of your data and where it needs to be put and I don't. STDOUT, a file, a string somewhere? In the interest of keeping the API lean I've left you to roll your own...

It is loosely based on the pattern for a lexer put forward in the video "[Lexical Scanning in Go](http://blog.golang.org/two-go-talks-lexical-scanning-in-go-and)" by Rob Pike.

It is called murp because I don't like what makes the JS in JSON and wanted to distance myself from it. It also means that if I want to I can deviate from the RFC's. I'm aiming to be compliant though. murp is the sound you make when you try to think of a name for your JSON lexer, and fail...

## Static Libraries

   - [Linux - Debian 64bit (1.0)](http://ploe.co.uk/libmurp-debian64.zip)
   - [Mac OS X (1.0)](http://ploe.co.uk/libmurp-osx.zip)

## The API

murp is a lexer. This means that unlike JSON parsers it does not allocate memory for the JSON data it unmarshals. My reason for this is that in C you're likely to want to define the shape of your own data and allocate it, so you won't want me taking my time and filling up your RAM with my own kind of weird data. It's best I just leave that bit to you, don't you think?

To unmarshal a JSON structure in murp there are four functions you can use:

```c
mp_Atomizer mp_Atomize(char *src, mp_Atomizer (*callback)(mp_Atom, void *));
mp_Atomizer mp_AtomizeSlice(mp_Slice *src, mp_Atomizer (*callback)(mp_Atom, void *));


mp_Atomizer mp_Probe(char *src, mp_Atomizer (*callback)(mp_Atom, void *), void *);
mp_Atomizer mp_ProbeSlice(mp_Slice *src, mp_Atomizer (*callback)(mp_Atom, void *), void *probe);
```

**mp_Atomize** is the simplest of the four. It takes a JSON object or array as its *src* and a *callback* function which we call an **mp_Atomizer**

```c
mp_Atomize(json, print_atom);
```

murp will then ensue to deserialize the elements in your JSON Object or Array.  It does this by setting the attributes on a struct called **mp_Atom**


```c
typedef struct {
	mp_Datatype type, container;
	mp_Slice key, value;
} mp_Atom;
```

First it identifies the sort of JSON structure we're unmarshalling. This is what container gets set to. It can be one of these three values.

```c
	mp_OBJECT,
	mp_ARRAY,
```

Next it moves on to unmarshalling the elements. If the *container* is an **mp_OBJECT** it will first look for a *key* (as JSON Objects are structures containing key-value pairs e.g. '{"name":"murp"}') If a key is successfully identified we set *key* on the **mp_Atom** to it. *key* is a type called **mp_Slice**

```c
typedef struct {
	char *start;
	unsigned int len;
} mp_Slice;

```

An **mp_Slice** defines a subset of your *src* string. *start* is a pointer to where the substring starts and *len* is the amount of characters until it ends i.e. its length. So *key.start* gets set to the start of the key, just after the opening quotation mark (**"**) and *key.len* gets set to the length, which is after the last character before the closing quotation mark.

If the *container* is an **mp_ARRAY** the *key* slice doesn't get set (this is because JSON Arrays are an ordered list of values e.g. [1,2,3,"myke",true,"etc"])

If the structure is somehow malformed and we can't identify it murp sets the *type* of the **mp_Atom** to the error code **mp_EOSTRUCT** - we will cover error codes in a moment.

Now we identify the *value* of the element and its *type*.

*value* is another **mp_Slice** - this is the value of the element in the JSON structure.

*type* will be one of the usual JSON types which are the following values in murp.

```c
	mp_NOTSET,
	mp_NULL,
	mp_STRING,
	mp_NUMBER,
	mp_BOOLEAN,
	mp_OBJECT,
	mp_ARRAY
```

There are also a bunch of error codes that the type can be set to if murp gets some invalid data. If murp encounters an error code it will stop trying to parse the string.

```c
	mp_DONE,
	mp_EOSTRUCT,
	mp_EOKEY,
	mp_EOVALUE,
```

   - **mp_DONE** is set when we reached the end of parsing cleanly. 
   - **mp_EOSTRUCT** is set when we couldn't identify the JSON structure we're parsing.
   - **mp_EOKEY** is set when we have trouble reading the *key* in an **mp_ARRAY**
   - **mp_EOVALUE** is set when we have trouble reading the *value*

There is also a macro for checking if a type is an error code.

```c
mp_IS_ERROR(atom.type);
```

So once we have set the **mp_Atom's** attributes we will pass it to your **mp_Atomizer** which should be defined with the following signature.

```c
mp_Atomizer foo(mp_Atom atom, void *p)

```

Your **mp_Atomizer** takes a copy of the **mp_Atom** we just built up, it also requires a void pointer which will be used when your **mp_Atomizers** with **mp_Probe** and **mp_ProbeSlice** - It will also have to return a status code.

```c
	mp_CONTINUE,
	mp_BREAK,
	mp_FAIL,
	mp_WHYBOTHER,
```

   - You should return **mp_CONTINUE** when you want murp to keep lexing the structure. If you get an error code then murp will not call the the **mp_Atomizer**
   - You should return **mp_BREAK** when you want murp to stop lexing the structure. Say if you've got all the elements you need.
   - You should return **mp_FAIL** if you want to differentiate your failures from your breaks.
   - **mp_WHYBOTHER** is returned by **mp_Atomize** when you pass it a NULL *callback* - you can use this if you want but why bother?

So here I have defined a simple **mp_Atomizer** that crawls over the **mp_Atoms** and prints their values.

```c
mp_Atomizer print_atom(mp_Atom atom, void *p) {
	printf("Atom {\n\tDatatype:\t%d\n\tContainer:\t%d\n", atom.type, atom.container);
	if(atom.key.start) printf("\tKey:\t%.*s\n", atom.key.len, atom.key.start);
	printf("\tValue:\t%.*s\n", atom.value.len, atom.value.start);
	puts("\n}\n");
	
	return mp_CONTINUE;
}
```

So it prints the attributes of each **mp_Atom** - like so:

```
Atom {
	Datatype:	2
	Container:	6
	Value:	element

}

Atom {
	Datatype:	3
	Container:	6
	Value:	1234

}

Atom {
	Datatype:	4
	Container:	6
	Value:	true

}

Atom {
	Datatype:	4
	Container:	6
	Value:	false

}

Atom {
	Datatype:	1
	Container:	6
	Value:	null

}

Atom {
	Datatype:	3
	Container:	6
	Value:	3.14

}

Atom {
	Datatype:	3
	Container:	6
	Value:	9.99e-2

}

Atom {
	Datatype:	5
	Container:	6
	Value:	{"test" : 12345, "urk" : [1, 2, 3, 4]}

}

Atom {
	Datatype:	6
	Container:	6
	Value:	[1, 2, 3, 4, 5]

}

Atom {
	Datatype:	9001
	Container:	6
	Value:	

}
```

Using **mp_Atomizers** to examine each **mp_Atom** you pass in you can decide what you want done with the data. Do you want to allocate some space for it? Do you want to gloss over it? That's completely up to you!

If you're wanting to pass some class of C object to the **mp_Atomizer** there is a similar interface called **mp_Probe** (actually **mp_Atomize** is a macro for **mp_Probe** where the void * is set to NULL) - Here is an example!

```c
mp_Atomizer count_elems(mp_Atom atom, void *probe) {
	if(mp_IS_ERROR(atom.type)) return mp_BREAK;
	
	int *count = probe;
	*count += 1;

	return mp_CONTINUE;
}

/* ...later in main... */
	int counter = 0;
	mp_Probe(json, count_elems, &counter);
```

The output would be something like:

```
I counted 9 elements in this string.
```

So what do we do with nested structures? What if the value we return is an **mp_ARRAY** or an **mp_OBJECT** We can pass the **mp_Slices** to **mp_AtomizeSlice** and **mp_ProbeSlice** like in the **mp_Atomizer** below.

```c
mp_Atomizer ps_PrintAtom(mp_Atom atom, void *p) {
	printf("Atom {\n\tDatatype:\t%d\n\tContainer:\t%d\n", atom.type, atom.container);
	if(atom.key.start) printf("\tKey:\t%.*s\n", atom.key.len, atom.key.start);
	printf("\tValue:\t%.*s\n", atom.value.len, atom.value.start);
	if ((atom.type == mp_OBJECT) || (atom.type == mp_ARRAY)) {
		int i;
		mp_ProbeSlice(&atom.value, ps_CountElems, &i);
		printf("\tElements:\t%d\n", i);
	}
	puts("\n}\n");

	// calls itself recursively
	if ((atom.type == mp_OBJECT) || (atom.type == mp_ARRAY)) mp_AtomizeSlice(&atom.value, ps_PrintAtom);
	
	if(mp_IS_ERROR(atom.type)) return mp_BREAK;
	return mp_CONTINUE;
}

```

Pretty magical, ey? I'm sure there will be plenty more examples as I use the API myself in my own projects. If you do anything swish with it, let me know!

## License

```
Copyright (c) 2015, Myke Atkinson
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```
