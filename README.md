## murp - what is it?

**murp is still a work in progress - that includes this README doc**

murp is a JSON lexer. It isn't a parser because I believe that you, the programmer, should be in charge of your heap. It presents a simple interface called Atomize, which you drop your parse function in to. 

If you're wanting to keep state there's an interface called Probe which works like Atomize but lets you drop an object in too. Any object... This is C after all.

You'll notice that serialization has been distinctly left out. That's because you know the shape of your data and where it needs to be put and I don't. STDOUT, a file, a string somewhere? In the interest of keeping the API lean I've left you to roll your own...

It is loosely based on the pattern for a lexer put forward in the video "Lexical Scanning in Go" by Rob Pike.

It is called murp because I don't like what makes the JS in JSON and wanted to distance myself from it. It also means that if I want to I can deviate from the RFC's. I'm aiming to be compliant though.

## The API

The function you use to do the parsing is an Atomizer function. The reason why is that murp rips apart your JSON data and identifies the key, the value, and its type for you. This chunk of identified data is called an Atom.

```
typedef struct Atom {
        Datatype type, container;
        Slice key, value;
} Atom;
```

An Atom is composed of two Slices. One is the key for the data and the other is the value. A Slice is a type in murp and is very similar to its Go counterpart. It's a pointer to a char and a length for the ensuing buffer.

```
typedef struct Slice {
        char *start;
        unsigned int len;
} Slice;
```


It also has a type which is identified by the lexer automatically. If an error occurs in the parsing, the type of error is used instead.


The signature of your Atomizer function should be 

```
Atomizer foo(Atom, void *);
```

And it can be called thusly

```
Atomize(json, atomizer);

Probe(json, atomizer, probe);
```

The first parameter is the Atom. For each Atom found in your JSON your atomizer is called on it. You can then unpack the value how you want to. There's no hash table eating up your precious memory and resources - just you and the data getting parsed, and a little room eaten up by the Lexer's state. It isn't overly sophisticated.

If it encounters an error it'll throw it as an Atom so that your Atomizer can react to it. There a bunch of different errors. That allows you to decide how you're going to deal with it - does junk need deallocating?

MURP_WHYBOTHER - The Lexer gives up before it starts because you didn't give it an Atomizer.
