/*
 * ifile.c for megawave, section mwplight
 *
 * generate the I-file (interface file)
 *
 * author : Jacques Froment <jacques.froment@univ-ubs.fr> (2005)
 * author : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mwpl.h"
#include "io.h"

#include "ifile.h"

#define MSG_ERROR_NULL_TREE \
     "[setprotobuf] NULL C tree. Need C body to be parsed"
#define MSG_ERROR_NULL_MAIN \
     "[setprotobuf] NULL main function"
#define MSG_ERROR_OPEN_FILE \
     "Cannot open file '%s' for writing"
#define MSG_ERROR_BUFFER_OVERFLOW \
     "Buffer overflow for prototype (increase STRSIZE)"

/*
 * Set the <protobuf> variable from the content of C->mfunc :
 * prototype the main function using K&R convention.
 *
 * This text is to be set in the fsummary field of the Mwiline
 * structure (see kernel/lib/include/mwi.h). From this, a
 * full K&R and ANSI C compliant prototype can be derived
 * using call_proto() (in kernel/lib/src/mw.c).
 * When the traditional mwp processor would be removed,
 * this should be simplified by getting a protobuf with
 * both K&R and ANSI C declaration. Then call_proto could
 * be removed and in the M-file, the default main function
 * declaration (usually not ANSI compliant) could be replaced
 * by this one (update mfile.c). So, prototypes would become
 * fully ANSI compliant without having to change the source of modules.
 */

/* TODO: review, drop K&R parts */
/* TODO: no if/then in a macro */
#define ADDPROTO(a)                                             \
     {                                                          \
          char tmp[BUFSIZ];                                     \
          sprintf(tmp, "%s%s", protobuf, a);                    \
          strcpy(protobuf, tmp);                                \
     }

void setprotobuf(void)
{
     t_varfunc * f;
     t_variable * p;
     char buf[STRSIZE];

     if (C == NULL)
          error(MSG_ERROR_NULL_TREE);
     f = C->mfunc;
     if (f == NULL)
          error(MSG_ERROR_NULL_MAIN);

     /* function name and arguments list */
     sprintf(protobuf, "%s %s ( ", f->v->Ftype, f->v->Name);
     for (p = f->param; p; p = p->next)
     {
          if (p != f->param)
               strcat(protobuf," , ");
          ADDPROTO(p->Name);
     }
     ADDPROTO(" )\\n");

     /* Type of arguments */
     for (p = f->param; p; p = p->next)
     {
          sprintf(buf, "%s %s ;\\n", p->Ftype,p->Name);
          ADDPROTO(buf);
     }
     ADDPROTO("\\n");
}

/*
 * generate I-file
 */

void genIfile(FILE * ifile)
{
     /* write fields of the Mwiline structure (see kernel/lib/include/mwi.h) */
     fprintf(ifile, \
             "{ \"%s\", _%s, usage_%s, \"%s\", \"%s\", \"%s\", \"%s\"}\n", \
             module_name, module_name, module_name, \
             group_name, H->Function, usagebuf, protobuf);
}
