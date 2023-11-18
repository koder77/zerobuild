#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#if __OpenBSD__ || __FreeBSD__ || __NetBSD__ || __HAIKU__
#include <sys/wait.h>
#endif

typedef char		            U1;
typedef int16_t                 S2;     /* INT     */
typedef int32_t                 S4;     /* LONGINT */
typedef int64_t					S8; 

/*
 zer0build makefile

[library, static, name = libshared.a]
sources = checkd.c, exe_time.c, mem.c, msg.c, vm.c, file.c, endian.c
includes = ../include

cflags = -O0 -g
*/

// #define NULL                    0L
#define TRUE                    1
#define FALSE                   0

#define BLOCK_OPEN              '['
#define BLOCK_CLOSE             ']'

#define ASSIGN                  '='
#define COMMA                   ','
#define QUOTE                   '"'
#define SPACE                   ' '

#define TYPE_LIBRARY            "library"
#define TYPE_EXECUTABLE         "executable"

#define INCLUDES                "includes"
#define SOURCES                 "sources"
#define OBJECTS                 "objects"
#define NAME                    "name"

#define STATIC                  "static"
#define SHARED                  "shared"

#define CFLAGS                  "cflags"
#define CPLUSPLUSFLAGS          "c++flags"

#define CCOMPILER               "ccompiler"
#define CPLUSPLUSCOMPILER       "c++compiler"

#define ARCHIVER                "archiver"
#define AFLAGS                  "aflags"

#define LFLAGS                  "lflags"

#define STRIPPED 				"strip"

#define NOTDEF                  0
#define EXECUTABLE              1
#define LIBRARY                 2

#define BUILD_STATIC            1
#define BUILD_SHARED            2

#define MAXSTRLEN               4000
#define MAXSOURCES              100
#define MAXINCLUDES             100
#define MAXOBJECTS              100

#if __linux__ || __CYGWIN__ || __MACH__ ||  __OpenBSD__ || __FreeBSD__ || __NetBSD__ || __HAIKU__
#define OS_LINUX                TRUE
#define OS_WINDOWS              FALSE
#endif

#if _WIN32
#define OS_LINUX                FALSE
#define OS_WINDOWS              TRUE
#endif


struct parsed_line
{
    U1 type;                        // executable, library
    U1 buildtype;                   // static, shared
	U1 stripped;
    U1 name[MAXSTRLEN];
    U1 sources[MAXSOURCES][MAXSTRLEN];
    S4 sources_ind;
    U1 includes[MAXINCLUDES][MAXSTRLEN];
    S4 includes_ind;
    U1 objects[MAXOBJECTS][MAXSTRLEN];
    S4 objects_ind;
    U1 cflags[MAXSTRLEN];
    U1 cplusplusflags[MAXSTRLEN];
    U1 aflags[MAXSTRLEN];
    U1 lflags[MAXSTRLEN];
};

struct comp
{
    U1 c[MAXSTRLEN];
    U1 cplusplus[MAXSTRLEN];
    U1 archiver[MAXSTRLEN];
};
