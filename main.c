/*
  main -- main function
  Copyright (C) 1998 Dieter Baron

  This file is part of otftot42, to use TrueType fonts in PostScript.
  The author can be contacted at <dillo@giga.or.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include "t42.h"

#include "config.h"

char *prg;

char version_string[] = 
PACKAGE " " VERSION "\n\
Copyright (C) 1998 Dieter Baron\n"
PACKAGE " comes with ABSOLUTELY NO WARRANTY, to the extent permitted by law.\n\
You may redistribute copies of\n"
PACKAGE " under the terms of the GNU General Public License.\n\
For more information about these matters, see the files named COPYING.\n";

char usage_string[] = "\
Usage: %s [-hVafcF] [-o file] [-e encoding] ttf-file ...\n";

char help_string[] = "\
\n\
  -h, --help            display this help message\n\
  -V, --version         display version number\n\
\n\
  -a, --afm             write afm file\n\
  -f, --font            write t42 file\n\
  -o, --output file     output to file\n\
  -c, --stdout          output to standard output\n\
  -e, --encoding enc    encoding to use (std, pdf, mac, latin1, font)*\n\
  -F, --full            include full TrueType font*\n\
\n\
*: not yet implemented\n\
\n\
Report bugs to <dillo@giga.or.at>.\n";

#define OPTIONS	"hVafo:ce:F"

struct option options[] = {
    { "help",      0, 0, 'h' },
    { "version",   0, 0, 'V' },
    { "name",      0, 0, 'n' },
    { "afm",       0, 0, 'a' },
    { "font",      0, 0, 'f' },
    { "output",    1, 0, 'o' },
    { "stdout",    0, 0, 'c' },
    { "encoding",  1, 0, 'e' },
    { "full",      0, 0, 'F' },
    { NULL,        0, 0, 0   }
};



char *substext(char *fname, char *ext, char *newext);
char *basename(char *name);



int
main(int argc, char **argv)
{
    extern int opterr, optind;
    extern char *optarg;

    int err;
    int c, what, full, cat;
    char *encoding;
    char *outfile;
    font *f;
    FILE *fout;

    char *fontfile;

    prg = argv[0];

    cat = full = what = 0;
    encoding = "std";
    outfile = NULL;

    opterr = 0;
    while ((c=getopt_long(argc, argv, OPTIONS, options, 0)) != EOF) {
	switch (c) {
	case 'f':
	    what |= WHAT_FONT;
	    break;
	case 'a':
	    what |= WHAT_AFM;
	    break;
	case 'F':
	    full = 1;
	    break;
	case 'e':
	    encoding = optarg;
	case 'o':
	    outfile = optarg;
	    break;
	case 'c':
	    cat = 1;
	    break;

	case 'h':
	    printf(usage_string, prg);
	    fputs(help_string, stdout);
	    exit(0);
	case 'V':
	    fputs(version_string, stdout);
	    exit(0);
	default:
	    fprintf(stderr, usage_string, prg);
	    exit(1);
	}
    }

    if (optind == argc) {
	fprintf(stderr, usage_string, prg);
	exit(1);
    }

    init();

    /* post process options */
    if (cat && outfile) {
	fprintf(stderr, "%s: can't write to both standard output "
		"and specified file\n",
		prg);
	exit(1);
    }
    if (cat || outfile) {
	if (optind+1 != argc) {
	    fprintf(stderr, "%s: can't write more than one font to %s\n",
		    prg, cat ? "standard output" : "specified file");
	    exit(1);
	}
	    
	switch (what) {
	case 0:
	    what = WHAT_FONT;
	    break;
	case WHAT_FONT|WHAT_AFM:
	    fprintf(stderr, "%s: can't write both font and afm to %s\n",
		    prg, cat ? "standard output" : "specified file");
	    exit(1);
	}
    }

    if (what == 0)
	what = WHAT_FONT|WHAT_AFM;

    err = 0;
    for (; optind<argc; optind++) {
	fontfile = argv[optind];

	if ((f=open_font(fontfile, what)) == NULL) {
	    err = 1;
	    continue;
	}
	
	if (what & WHAT_AFM) {
	    if (cat)
		fout = stdout;
	    else if (outfile) {
		if ((fout=fopen(outfile, "w")) == NULL) {
		    fprintf(stderr, "%s: can't create file `%s': %s\n",
			    prg, outfile, strerror(errno));
		    err = 1;
		}
	    }
	    else {
		outfile = substext(basename(fontfile), ".ttf", ".afm");
		if ((fout=fopen(outfile, "w")) == NULL) {
		    fprintf(stderr, "%s: can't create file `%s': %s\n",
			    prg, outfile, strerror(errno));
		    err = 1;
		}
		outfile = NULL;
	    }
	    if (fout)
		write_afm(f, fout);
	}
	if (what & WHAT_FONT) {
	    if (cat)
		fout = stdout;
	    else if (outfile) {
		if ((fout=fopen(outfile, "w")) == NULL) {
		    fprintf(stderr, "%s: can't create file `%s': %s\n",
			    prg, outfile, strerror(errno));
		    err = 1;
		}
	    }
	    else {
		outfile = substext(basename(fontfile), ".ttf", ".t42");
		if ((fout=fopen(outfile, "w")) == NULL) {
		    fprintf(stderr, "%s: can't create file `%s': %s\n",
			    prg, outfile, strerror(errno));
		    err = 1;
		}
		outfile = NULL;
	    }
	    if (fout)
		write_t42(f, fout);
	}

	close_font(f);
    }

    done();

    exit(err);
}



char *
substext(char *fname, char *ext, char *newext)
{
    static char b[8192];
    int l, el;

    l = strlen(fname);
    el = strlen(ext);

    if (strcasecmp(fname+l-el, ext) == 0) {
	strncpy(b, fname, l-el);
	strcpy(b+l-el, newext);
    }
    else {
	strcpy(b, fname);
	strcpy(b+l, newext);
    }

    return b;
}
