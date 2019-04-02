/**
 * ctpl.h - Magic macros and APIs for C-templates
 *
 * Aniruddha. A (aniruddha.a@gmail.com)
 *
 * Sat Mar 31, 2019
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or http://www.wtfpl.net/
 * for more details.
 */

#ifndef __CTPL_H_
#define __CTPL_H_

#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include "foreach.h"

#if (__STDC_VERSION__ >= 201112L)
# if defined(__GNUC__) && !defined(__clang__)
#  if (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9))
#   define IS_C11
#  endif
# else
#  define IS_C11
# endif
#endif

#ifndef IS_C11
#error "You're on a old stinky compiler! Use a modern C11 compiler"
#endif 

#define PRELUDE_PRINT      "extern void _P(char*, ...);"
#define PRELUDE_FORMATSPEC "#define formatspec(X) _Generic((X),\
char:\"%c\",\
signed char:\"%hhd\",\
unsigned char:\"%hhu\",\
signed short:\"%hd\",\
unsigned short:\"%hu\",\
signed int:\"%d\",\
unsigned int:\"%u\",\
long int:\"%ld\",\
unsigned long int:\"%lu\",\
long long int:\"%lld\",\
unsigned long long int:\"%llu\",\
float:\"%f\",\
double:\"%f\",\
long double:\"%Lf\",\
char*:\"%s\",\
const char*:\"%s\",\
void*:\"%p\")"
#define PRELUDE_UNDERSCORE "#define _(X) _P(formatspec(X),X);"

#define typename(x) _Generic((x),\
        bool                   : "bool"                  ,\
        char                   : "char"                  ,\
        signed char            : "signed char"           ,\
        unsigned char          : "unsigned char"         ,\
        short int              : "short int"             ,\
        unsigned short int     : "unsigned short int"    ,\
        int                    : "int"                   ,\
        unsigned int           : "unsigned int"          ,\
        long int               : "long int"              ,\
        unsigned long int      : "unsigned long int"     ,\
        long long int          : "long long int"         ,\
        unsigned long long int : "unsigned long long int",\
        float                  : "float"                 ,\
        double                 : "double"                ,\
        long double            : "long double"           ,\
        char *                 : "char *"                ,\
        const char *           : "const char *"          ,\
        void *                 : "void *"                ,\
        int *                  : "int *"                 ,\
        char **                : "char **"               ,\
        int **                 : "int **"                ,\
        default                : "OTHER"                  \
)

#define NEED_AMPERSAND(S)  _Generic((S), char *: S, default: &(S) )

#define SAVE_SYM(S) \
  _ctpl_save_sym(_CTX,#S,(uintptr_t)NEED_AMPERSAND(S),typename(S),NULL);

/* *** EXTERNAL API *** */

#define ctpl_expand(C,T,...)        \
    ctpl_ctx *_CTX = (C);           \
    FOR_EACH(SAVE_SYM, __VA_ARGS__) \
    _ctpl_expand(C,T)

#define ctpl_custom_type(C,S,ST,F) _ctpl_save_sym(C,#S,(uintptr_t)&(S),ST,F)

#define CTPL_MAX_SYMBOLS 16 /* limited by foreach macro */
typedef struct _ctpl_syms {
    const char *name;
    uintptr_t   addr;
    const char *typename;
    const char *inc_file;
} ctpl_syms;

typedef struct _ctpl {
    char      *buf;
    size_t     blen;
    char      *tpl;                         /* XXX: only to print err msg */
    int        status;                      /* ctpl_errcodes              */
    ctpl_syms  symbols[CTPL_MAX_SYMBOLS];
    int        nsyms;
} ctpl_ctx;
#define ctpl_status(X) ((X)->status)

enum ctpl_errcodes {
    ctpl_ok,
    ctpl_parse_fail,
    ctpl_compile_fail,
    ctpl_reloc_fail,
    ctpl_symbol_fail,
    ctpl_alloc_fail,
    ctpl_read_fail,
};

bool ctpl_init(ctpl_ctx *c, char *buf, size_t blen);
const char* ctpl_errmsg(ctpl_ctx *c);

/* *** INTERNAL *** */
void _ctpl_expand(ctpl_ctx *c, char *tpl);
void _ctpl_save_sym(ctpl_ctx *c, const char *sym,
                    uintptr_t addr, const char *typ,
                    const char *inc_file);
void _P(char *fmt, ...);

#endif
