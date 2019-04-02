/**
 * ctpl.c - The C-template engine based on libtcc. Thanks Bellard & team! :)
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
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <libtcc.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctpl.h>

#define TPL_BEGIN "@{"
#define TPL_END   "@}"
#define FUNCNAME  "_func"

static char _cbuf[BUFSIZ]; /* _cbuf is used for both code and output of code  */
static int  _extdlen = 0;  /* external declarations + prelude length in _cbuf */
static int  _explen  = 0;  /* expanded data after dynamic func call           */

#define DEBUG 0 /* Set to 1, to make your head spin */

void
_debug (const char *fmt, ...)
{
    va_list ap;
    if (DEBUG != 1) {
        return;
    }
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

bool
ctpl_init (ctpl_ctx *c,
           char     *buf,
           size_t    blen)
{
    c->buf    = buf;
    c->blen   = blen;
    c->nsyms  = 0;
    c->status = ctpl_ok;

    return true;
}

/**
 * The function that finally gets invoked via template to dump
 * data to buffer.
 * Though this is vararg, for now only single arg is processed, duh!
 */
void
_P (char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    _explen += vsnprintf(_cbuf + _extdlen + _explen,
                         sizeof _cbuf - (_extdlen + _explen), fmt, ap);
    va_end(ap);
}


/* libtcc err handler */
static void
_efunc (void       *opq,
        const char *msg)
{
    // TODO: XXX more precise err msg
#if 0
    const char *p = strchr(msg, ':');
    int ln = atoi(p+1);
    ctpl_ctx *c = (ctpl_ctx*)opq;
    if (ln) {
        p = c->tpl;
        for (int i = 0; i < ln-1; i++) {
            p = strchr(p, '\n');
            p+=1;
        }
        fprintf(stderr, ">> %*.s <<\n",10, p);
    }
#endif
    fprintf(stderr, "ctpl error: %s\n", msg);
}

/**
 * Load a header file - expected to contain only the declarations
 * If this references some other header, then that has to be included
 * too!
 */
static int
_ctpl_load_file (ctpl_ctx    *c,
                 const char  *inc_fpath,
                 char       **buf)
{
    struct stat  sb;
    FILE        *fp = fopen(inc_fpath, "r");

    if (!fp) return 0;
    if (stat(inc_fpath , &sb) == -1) {
        fclose(fp);
        return 0;
    }
    *buf = calloc(1, sb.st_size);
    assert(*buf);
    if (sb.st_size != fread(*buf, 1, sb.st_size, fp)) {
        c->status = ctpl_read_fail;
        _debug("Failed to read file: %s (%s)\n", inc_fpath, strerror(errno));
    }
    fclose(fp);
    return  (int)sb.st_size;
}

/**
 * Prepend external declarations for the variables(symbols)
 * that are used inside the template
 */
void
_ctpl_add_extdecls (ctpl_ctx *c)
{
    char  decl[80] = {0};
    int   i, l;
    char *buf, *p;

    for (i = 0; i < c->nsyms; i++) {
        l = 0; p = NULL;
        if (c->symbols[i].inc_file) {
            l = _ctpl_load_file(c, c->symbols[i].inc_file, &buf);
            if (l && _extdlen + l < BUFSIZ) {
                _debug("Inc File: %s [%s]\n", c->symbols[i].inc_file, buf);
                strcat(_cbuf, buf);
                _extdlen += l;
            }
            if (buf) free(buf);
        }
        p = strstr(c->symbols[i].typename, "[]"); /* spl case for array */
        if (p) {
            _extdlen += snprintf(decl, sizeof decl, "extern %.*s %s[];\n",
                                 (int)(p - c->symbols[i].typename),
                                 c->symbols[i].typename, c->symbols[i].name);
        } else {
            _extdlen += snprintf(decl, sizeof decl, "extern %s %s;\n",
                                 c->symbols[i].typename, c->symbols[i].name);
        }
        strcat(_cbuf, decl);
    }
}

/**
 * Essential declarations and macro definitions
 */
void
_ctpl_add_preludes (void)
{
    _extdlen += snprintf(_cbuf + _extdlen, sizeof _cbuf - _extdlen,
                         "%s\n%s\n%s\n",
                         PRELUDE_PRINT,       /* buffer printer               */
                         PRELUDE_FORMATSPEC,  /* generic printf format detect */
                         PRELUDE_UNDERSCORE); /* generic print function       */
}

const char*
ctpl_errmsg (ctpl_ctx *c)
{
    assert(c != NULL);
    switch (c->status) {
        case ctpl_ok:
            return "ctpl_ok";
        case ctpl_parse_fail:
            return "template parsing failed";
        case ctpl_compile_fail:
            return "template compilation failed";
        case ctpl_reloc_fail:
            return "failed to relocate generated code";
        case ctpl_alloc_fail:
            return "failed to allocate memory";
        case ctpl_symbol_fail:
            return "failed to find symbol";
        case ctpl_read_fail:
            return "failed to read file";
        default:
            return "unknown error";
    }
}

/* Keep track of all symbols used inside the template */
void
_ctpl_save_sym (ctpl_ctx   *c,
                const char *name,
                uintptr_t   addr,
                const char *sym_type,
                const char *inc_file)
{
    c->symbols[c->nsyms].name     = name;
    c->symbols[c->nsyms].addr     = addr;
    c->symbols[c->nsyms].typename = sym_type;
    c->symbols[c->nsyms].inc_file = inc_file;
    c->nsyms++;
    assert(c->nsyms < CTPL_MAX_SYMBOLS);
}

void
_ctpl_populate_tcc_state (TCCState *S,
                          ctpl_ctx *c)
{
    int  i;

    tcc_set_output_type(S, TCC_OUTPUT_MEMORY); /* Imp! */
    tcc_add_symbol(S, "_P", _P); /* buffer printer */
    tcc_define_symbol(S, "NL", "_P(\"\n\");");
    tcc_define_symbol(S, "TAB", "_P(\"\t\");");
    tcc_set_error_func(S, c, _efunc);
    for (i = 0; i < c->nsyms; i++) {
        _debug("\nexport %s = %p\n", c->symbols[i].name,
                                     (void*)c->symbols[i].addr);
        tcc_add_symbol(S, c->symbols[i].name, (void*)c->symbols[i].addr);
    }
}

/**
 * Core! create chunks of C code to be relocated by libtcc and
 * make them dump ourput to our preset buffer
 */
void
_ctpl_expand (ctpl_ctx *c,
              char     *tpl)
{
    char      *p           , *sp, *ep;
    int        codelen;
    void     (*fp)(void) = NULL;
    TCCState  *S         = NULL;

    c->tpl = tpl;

    _extdlen = 0;
    _ctpl_add_preludes();
    _ctpl_add_extdecls(c);
    /* Expand one template at a time, copying before and after parts
     * to the user buffer
     */
    p = tpl;
    while ((sp = strstr(p, TPL_BEGIN))) {
        S = tcc_new();
        if (!S) {
            c->status = ctpl_alloc_fail;
            _debug("Failed to init tcc state");
            return;
        }
        _ctpl_populate_tcc_state(S, c);

        strncat(c->buf, p, (int)(sp - p)); /* before tpl start */
        sp += 2;
        ep = strstr(sp, TPL_END);
        codelen = snprintf(_cbuf + _extdlen, sizeof _cbuf - _extdlen,
                           "void %s(void)\n{\n%.*s\n}",
                           FUNCNAME,
                           (int)(ep - sp), sp);
        p = ep + 2;
        _debug("\nBlock: %d (\n%s\n)\n", codelen, _cbuf);
        if (tcc_compile_string(S, _cbuf) == -1) {
            c->status = ctpl_compile_fail;
            _debug("Failed to compile code string\n");
            return;
        }
        if (tcc_relocate(S, TCC_RELOCATE_AUTO) < 0) {
            c->status = ctpl_reloc_fail;
            _debug("Failed to relocate code\n");
            return;
        }
        fp = tcc_get_symbol(S, FUNCNAME);
        if (!fp) {
            c->status = ctpl_symbol_fail;
            _debug("Failed to get func: %s\n", FUNCNAME);
            return;
        }
        _cbuf[_extdlen] = '\0';
        _explen = 0;
        fp(); /* invoke generated func - eval !! ;-) */
        _debug("\nData expanded: %d\n", _explen);
        strcat(c->buf, _cbuf + _extdlen);
        _cbuf[_extdlen] = '\0';

        tcc_delete(S);
        _debug("\nSo far: [%s]\n", c->buf);
    }
    strcat(c->buf, p); /* remaining */
    c->status = ctpl_ok;
}
