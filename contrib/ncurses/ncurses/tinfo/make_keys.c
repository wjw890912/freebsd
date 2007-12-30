/****************************************************************************
 * Copyright (c) 1998-2005,2007 Free Software Foundation, Inc.              *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 *  Author: Thomas E. Dickey <dickey@clark.net> 1997                        *
 ****************************************************************************/

/*
 * This replaces an awk script which translated keys.list into keys.tries by
 * making the output show the indices into the TERMTYPE Strings array.  Doing
 * it that way lets us cut down on the size of the init_keytry() function.
 */

#define USE_TERMLIB 1
#include <curses.priv.h>

MODULE_ID("$Id: make_keys.c,v 1.13 2007/01/07 00:00:14 tom Exp $")

#include <names.c>

#define UNKNOWN (SIZEOF(strnames) + SIZEOF(strfnames))

static size_t
lookup(const char *name)
{
    size_t n;
    bool found = FALSE;
    for (n = 0; strnames[n] != 0; n++) {
	if (!strcmp(name, strnames[n])) {
	    found = TRUE;
	    break;
	}
    }
    if (!found) {
	for (n = 0; strfnames[n] != 0; n++) {
	    if (!strcmp(name, strfnames[n])) {
		found = TRUE;
		break;
	    }
	}
    }
    return found ? n : UNKNOWN;
}

static void
make_keys(FILE *ifp, FILE *ofp)
{
    char buffer[BUFSIZ];
    char from[BUFSIZ];
    char to[BUFSIZ];
    int maxlen = 16;

    while (fgets(buffer, sizeof(buffer), ifp) != 0) {
	if (*buffer == '#')
	    continue;
	if (sscanf(buffer, "%s %s", to, from) == 2) {
	    int code = lookup(from);
	    if (code == UNKNOWN)
		continue;
	    if ((int) strlen(from) > maxlen)
		maxlen = strlen(from);
	    fprintf(ofp, "\t{ %4d, %-*.*s },\t/* %s */\n",
		    code,
		    maxlen, maxlen,
		    to,
		    from);
	}
    }
}

static void
write_list(FILE *ofp, const char **list)
{
    while (*list != 0)
	fprintf(ofp, "%s\n", *list++);
}

int
main(int argc, char *argv[])
{
    static const char *prefix[] =
    {
	"#ifndef NCU_KEYS_H",
	"#define NCU_KEYS_H 1",
	"",
	"/* This file was generated by MAKE_KEYS */",
	"",
	"#if BROKEN_LINKER",
	"static",
	"#endif",
	"const struct tinfo_fkeys _nc_tinfo_fkeys[] = {",
	0
    };
    static const char *suffix[] =
    {
	"\t{ 0, 0} };",
	"",
	"#endif /* NCU_KEYS_H */",
	0
    };

    write_list(stdout, prefix);
    if (argc > 1) {
	int n;
	for (n = 1; n < argc; n++) {
	    FILE *fp = fopen(argv[n], "r");
	    if (fp != 0) {
		make_keys(fp, stdout);
		fclose(fp);
	    }
	}
    } else {
	make_keys(stdin, stdout);
    }
    write_list(stdout, suffix);
    return EXIT_SUCCESS;
}
