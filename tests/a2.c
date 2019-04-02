/**
 * a2.c -Test code for external struct (in header)
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
#include <stdlib.h>
#include "a2.h"
#include "ctpl.h"

int main (int argc, char *argv[])
{
    struct books B[3] = {
        {"Pragmatic Programmer" , 111, "Andrew & Dave"},
        {"Passionate Programmer", 222, "Chad Fowler"}  ,
        {"Clean Coder"          , 333, "Bob Martin"}   ,
    };
    int  N            = 3;
    char bbuf[BUFSIZ] = {0};
    char *tpl =
        "<books>\n"
        "@{\n" /* <--- Template Start */ 
        "for (int i = 0; i < N; i++) {\n"
        "   _(\"<book>\") NL\n"
        "   TAB _(\"<name>\") _(B[i].name) _(\"</name>\") NL\n"
        "   TAB _(\"<isbn>\") _(B[i].isbn) _(\"</isbn>\") NL\n"
        "   TAB _(\"<author>\") _(B[i].author) _(\"</author>\") NL\n"
        "   _(\"</book>\") NL\n"
        "}\n"
        "@}\n" /* <--- Template End */ 
        "</books>\n"
        "\n";

    ctpl_ctx C;
    ctpl_init(&C, bbuf, sizeof bbuf);
    ctpl_custom_type(&C, B, "struct books []", "a2.h"); /* Defn from file */
    ctpl_expand(&C, tpl, N); /* Basic types can be given as is */
    if (ctpl_status(&C) == ctpl_ok) {
        printf("\nExpanded:\n%s\n", bbuf);
    } else {
        printf("\nFailed: %s\n", ctpl_errmsg(&C));
    }
    return 0;
}
