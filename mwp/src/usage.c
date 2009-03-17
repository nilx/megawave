/**
 * @file usage.c
 *
 * generate the usage text fort a megawave module
 *
 * @author Jacques Froment <jacques.froment@univ-ubs.fr> (1994 - 2005)
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008 - 2009)
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "usage.h"

/**
 * @brief write the code for the module usage header
 */
void module_presentation(FILE * afile,
			const char * Prog, const char * Vers, 
			const char * Auth, const char * Func,
			const char * Lab)
{
    time_t tloc;
    int year;
     
    /* current year */
    tloc = time(NULL);
    year = localtime(&tloc)->tm_year;
    year %= 100;
    year += 2000;

    fprintf(afile, "  fprintf(stderr, \"%s %s\\n\");\n", Prog, Vers);
    fprintf(afile, "  fprintf(stderr, \"%s\\n\");\n", Func);
    fprintf(afile, "  fprintf(stderr, \""
	    "This program, written by %s\");\n", Auth);
    if (0 < strlen(Lab))
	fprintf(afile, "  fprintf(stderr, \" (%s)\");\n", Lab);
    fprintf(afile, "  fprintf(stderr, \""
	    ", is part of the megawave framework.\\n\");\n");
    fprintf(afile, "  fprintf(stderr, \""
	    "(C)1995-%d CMLA, ENS Cachan, "
	    "94235 Cachan cedex, France.\\n\");\n", year);
    fprintf(afile, "  fprintf(stderr, \""
	    "-> http://megawave.cmla.ens-cachan.fr/\\n\");\n");
    fprintf(afile, "  fprintf(stderr, \"\\n\");\n");

    return;
}
