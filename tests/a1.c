/**
 * a1.c - Hello C-Template! :)
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
#include "ctpl.h"

int main (int argc, char *argv[])
{
    char          bbuf[128] = {0};
    const char   *iface        = "eth0";
    const char   *addr         = "10.1.1.2";
    unsigned int  mtu          = 1500;
    ctpl_ctx      C;

    char *tpl =
        "interface: @{ _(iface) @} \n"
        "address:   @{ _(addr) @}  \n"
        "MTU:       @{ _(mtu) @}   \n"
        "\n";

    ctpl_init(&C, bbuf, sizeof bbuf);
    ctpl_expand(&C, tpl, iface, addr, mtu);/* all basic types - pass as args! */
    if (ctpl_status(&C) == ctpl_ok) {
        printf("\nExpanded:\n%s\n", bbuf);
    } else {
        printf("\nFailed: %s\n", ctpl_errmsg(&C));
    }
    return 0;
}
