## murp - what is it?

**murp is still a work in progress - that includes this README doc**

murp is a JSON lexer. It isn't a parser because I believe that you, the programmer, should be in charge of your heap. It presents a simple interface called Atomize, which you drop your parse function in to. 

If you're wanting to keep state there's an interface called Probe which works like Atomize but lets you drop an object in too. Any object... This is C after all.

You'll notice that serialization has been distinctly left out. That's because you know the shape of your data and where it needs to be put and I don't. STDOUT, a file, a string somewhere? In the interest of keeping the API lean I've left you to roll your own...

It is loosely based on the pattern for a lexer put forward in the video "[Lexical Scanning in Go](http://blog.golang.org/two-go-talks-lexical-scanning-in-go-and)" by Rob Pike.

It is called murp because I don't like what makes the JS in JSON and wanted to distance myself from it. It also means that if I want to I can deviate from the RFC's. I'm aiming to be compliant though. murp is the sound you make when you try to think of a name for your JSON lexer, and fail...

## The API

The function you use to do the parsing is an Atomizer function. The reason why is that murp rips apart your JSON data and identifies the key, the value, and its type for you. This chunk of identified data is called an Atom.

```c
typedef struct Atom {
        Datatype type, container;
        Slice key, value;
} Atom;
```

An Atom is composed of two Slices. One is the key for the data and the other is the value. A Slice is a type in murp and is very similar to its Go counterpart. It's a pointer to a char and a length for the ensuing buffer.

```c
typedef struct Slice {
        char *start;
        unsigned int len;
} Slice;
```


It also has a type which is identified by the lexer automatically. If an error occurs in the parsing, the type of error is used instead.


The signature of your Atomizer function should be 

```c
Atomizer foo(Atom, void *);
```

And it can be called thusly:

```c
Atomize(json, foo);

Probe(json, foo, probe);
```

The first parameter is the Atom. For each Atom found in your JSON your atomizer is called on it. You can then unpack the value how you want to. There's no hash table eating up your precious memory and resources - just you and the data getting parsed, and a little room eaten up by the Lexer's state. It isn't overly sophisticated.

If it encounters an error it'll throw it as an Atom so that your Atomizer can react to it. There a bunch of different errors. That allows you to decide how you're going to deal with it - does junk need deallocating?

## License

Copyright (c) 2015, Myke Atkinson
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
