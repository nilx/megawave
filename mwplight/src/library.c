/**
 * @file library.c
 *
 * generate the library code for a megawave module
 *
 * @author Jacques Froment <jacques.froment@univ-ubs.fr> (2005 - 2007), \
 *         Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

/* TODO: move to ANSI-only code */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mwplight-defs.h"
#include "io.h"

#include "library.h"

#define MSG_ERROR_BODY_NOT_PARSED \
     "C body not parsed !"
#define MSG_ERROR_REOPEN_FILE \
     "Cannot reopen file \"%s\" for reading"
#define MSG_ERROR_OPEN_FILE \
     "Cannot open file \"%s\" for writing"


/*
 * write additional text for t_varfunc <f>
 */
static void writeMVarFunc(FILE * mfile, t_varfunc * f)
{
     /*
      * Function declarations (but the main one) are local to the module file
      * (not to be archived in library); therefore the C storage keyword
      * "static" has to be added in front of function declarations.
      */

     /* TODO: use strlen */
     if (ISCI_FUNCDECL(f) && (C->mfunc != f) && (f->v->Cstorage[0] == '\0'))
          fprintf(mfile, "static ");

     /*
      * TODO: in case of the main function declaration, replace it
      * by the content of <protobuf> (K&R and ANSI declarations).
      * See setprotobuf() for more comments.
      */
}


/*
 * write the body
 */
static void writeMbody(FILE * mfile, FILE * sfile)
{
     int l;
     long n;
     t_varfunc * f;

     f = C->varfunc;
     if (f == NULL)
          error(MSG_ERROR_BODY_NOT_PARSED);

     n = 0;

     while ((l = getc(sfile)) != EOF)
     {
          putc(l, mfile);
          n++;
          if (f && (n == ((f->l0) - 1)))
          {
               writeMVarFunc(mfile, f);
               f = f->next;
          }
     }
}


/*
 * generate M-file
 */

void gen_lib_file(FILE * mfile, FILE * sfile)
{
     writeMbody(mfile, sfile);
}
