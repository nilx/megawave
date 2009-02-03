/**
 * @file usage.c
 *
 * generate the usage text fort a megawave module
 *
 * @author Jacques Froment <jacques.froment@univ-ubs.fr> (1994 - 2005)
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

/* TODO: simplify, make parseable and clean */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "definitions.h"
#include "io.h"

#include "usage.h"

#define MSG_ERROR_UNEXPECTED_HEADER \
     "[cutstring] unexpected error with the header"
#define MSG_ERROR_MEMORY \
     "[module_presentation] Not enough memory"

#define MSG_COPYRIGHT \
     "megawave : (C)1998-%d CMLA, ENS Cachan, 94235 Cachan cedex, France.\\n"
#define MSG_WEBSITE \
     "Last version at http://megawave.cmla.ens-cachan.fr/\\n"
#define MSG_COPYRIGHT_WEBSITE \
     "megawave : (C)1998-%d CMLA, ENS Cachan. http://megawave.cmla.ens-cachan.fr/\\n"

/* maximum length of a line */
#define LINE_LEN  79

static void xprint(FILE * afile, char c, size_t n)
{
     size_t i;
     for (i=1; i<=n; i++)
          putc(c, afile);
}


static void cutstring(char * Ch1, char * Ch2, char * Ch, size_t l)
{
     size_t lC, i;

     lC = strlen(Ch);
     if (lC <= l) {
          strcpy(Ch1, Ch);
          Ch2[0] = '\0';
          return;
     }
     for (i = l; (i > 0) && (Ch[i] != ' '); i--);
     if (i == 0)
          error(MSG_ERROR_UNEXPECTED_HEADER);
     strncpy(Ch1, Ch, i);
     Ch1[i] = '\0';
     Ch = (char *) &Ch[i + 1];
     strncpy(Ch2, Ch, l);
     Ch2[l] = '\0';
}


/* TODO: simplify the presentation */
int module_presentation(FILE * afile, char * Prog, char * Vers, char * Auth,
                        char * Func, char * Lab)
{
     size_t lP, lu, lV, dV;
     char * Ch, * Ch1, * Ch2, * Ch3, * Ch4;
     time_t tloc;
     int yearXX, yearXXXX;

     lP = strlen(Prog);
     if (lP < 2)
          return(-1);
     /* usable length of the text */
     lu = LINE_LEN - lP - 6;
     Ch1 = (char *) calloc(lu + 1, sizeof(char *));
     Ch2 = (char *) calloc(lu + 1, sizeof(char *));
     Ch3 = (char *) calloc(lu + 1, sizeof(char *));
     Ch4 = (char *) calloc(lu + 1, sizeof(char *));
     Ch = (char *) calloc(255, sizeof(char *));
     if ((Ch1 == NULL) || (Ch2 == NULL) || (Ch3 == NULL))
          error(MSG_ERROR_MEMORY);

     /* current year */
     (void)time(&tloc);
     yearXX = localtime(&tloc)->tm_year;
     /* if tm_year really is the number of years since 1900 */
     if (yearXX >= 100)
          yearXX -= 100;
     /* written in 1998... */
     if (yearXX < 98)
          yearXXXX = 2000 + yearXX;
     else
          yearXXXX = 1900 + yearXX;

     /* string generation */
     strcpy(Ch, "Author(s) : ");

     if (Auth != NULL && *Auth != '\0') {
          sprintf(Ch1, "%s. ", Auth);
          strcat(Ch, Ch1);
     }
     if (Lab != NULL && *Lab != '\0') {
          sprintf(Ch2, "%s.", Lab);
          strcat(Ch, Ch2);
     }
     cutstring(Ch1, Ch2, Func, lu-1);
     cutstring(Ch3, Ch4, Ch, lu);

     /* line #1 */
     fprintf(afile, "  fprintf(stderr, \"");
     xprint(afile, '-', LINE_LEN);
     fprintf(afile, "\\n\");\n");

     /* line #2 */
     fprintf(afile, "  fprintf(stderr, \"");
     xprint(afile, '\\', 4);
     xprint(afile, ' ', lP);
     fprintf(afile, "//  ");
     if (Ch1[0] != '\0')
          fprintf(afile, "%s", Ch1);
     if (Ch2[0] == '\0')
          putc('.', afile);
     fprintf(afile, "\\n\");\n");

     /* line #3 */
     fprintf(afile, "  fprintf(stderr, \"");
     fprintf(afile, " \\\\\\\\");
     xprint(afile, ' ', lP-2);
     fprintf(afile, "//   ");
     if (Ch2[0] != '\0')
          fprintf(afile, "%s.", Ch2);
     fprintf(afile, "\\n\");\n");

     /* line #4 */
     fprintf(afile, "  fprintf(stderr, \"");
     fprintf(afile, "  %s    ", Prog);
     if ((Auth != NULL && *Auth != '\0') || (Lab != NULL && *Lab != '\0'))
          fprintf(afile, "%s\\n", Ch3);
     else
          Ch4[0] = '\0';
     fprintf(afile, "\");\n");

     /* line #5 */
     /* TODO: check copyrights */
     /* TODO: update addresses? */
     /* TODO: exact license info */
     fprintf(afile, "  fprintf(stderr, \"");
     sprintf(Ch, MSG_COPYRIGHT, yearXXXX);
     fprintf(afile, " //");
     xprint(afile, ' ', lP - 2);
     fprintf(afile, "\\\\\\\\   ");
     if (Ch4[0] == '\0')
          fprintf(afile, "%s", Ch);
     else
          fprintf(afile, "%s\\n", Ch4);
     fprintf(afile, "\");\n");

     /* line #6 */
     fprintf(afile, "  fprintf(stderr, \"");
     lV = strlen(Vers);
     dV = (lP - lV - 1) / 2;
     fprintf(afile, "//");
     if ((lV > 0) && (lV <= lP-1)) {
          xprint(afile, ' ', dV);
          if ((2 * dV < lP - lV - 1) && (lV < lP - 1)) {
               fprintf(afile, "V %s", Vers);
               dV++;
          }
          else
               fprintf(afile, "V%s", Vers);
          xprint(afile, ' ', lP - dV - lV - 1);
     }
     else
          xprint(afile, ' ', lP);
     fprintf(afile, "\\\\\\\\  ");
     if (Ch4[0] == '\0')
          sprintf(Ch, MSG_WEBSITE);
     else
          sprintf(Ch, MSG_COPYRIGHT_WEBSITE, yearXXXX);
     fprintf(afile, "%s", Ch);
     fprintf(afile, "\");\n");

     /* last lines */
     fprintf(afile, "  fprintf(stderr, \"");
     xprint(afile, '-', LINE_LEN);
     fprintf(afile, "\\n\\n");
     fprintf(afile, "\");\n");

     return 0;
}
