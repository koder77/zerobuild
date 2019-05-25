ZEROBUILD
=========
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/b929bcf0782b4b09b29f5cd090f05a59)](https://www.codacy.com/app/koder77/zerobuild?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=koder77/zerobuild&amp;utm_campaign=Badge_Grade)

Zerobuild is a very simple build tool for C programs.
I wrote it because sometimes I did not have an environment with a full bash & automake tools.

PROS: very simple syntax of "makefiles". Zerobuild is only one executable.

CONS: you have to set up search paths by "hand".

TODO: check if .h files are changed.
I now compiled my Nano VM with zerobuild!

Here is the zerobuild.txt of zerobuild:

<pre>
-------- snip ---------------
# zerobuild makefile

[executable, name = zerobuild]
sources = global.h, parser.c, make.c, process.c

ccompiler = clang-3.9
-------- snip ---------------
</pre>

That's it!

You can use a different name as "zerobuild.txt" too:

<pre>
$ zerobuild foobar.txt
</pre>

And to force a full build:

<pre>
$ zerobuild force
</pre>

Here is an example of how to build a shared library:

<pre>
------------------ snip ------
# zerobuild makefile

[library, shared, name = libtest.so]
sources = arr.c, libtest.c, mutex.c, nanovmlib.c

includes = ../include

ccompiler = gcc
archiver = ar
linker = ld
aflags = "cru"

cflags = "-g -ffast-math -fomit-frame-pointer -O3 -march=native"
lflags = "-lc -shared"
----------------- snip -------
</pre>

Stefan Pietzonke  25.05.2019
