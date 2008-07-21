/*
 * tfile.c for megawave, section mwplight
 *
 * generate the T-file (documentation in LaTeX)
 *
 * author : Jacques Froment <jacques.froment@univ-ubs.fr> (2005 - 2007)
 * author : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

/* TODO: simplify */
/* TODO: use markdown and/or doxygen */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "mwpl.h"
#include "io.h"

#include "tfile.h"

#define MSG_ERROR_ILLEGAL_ICTYPE \
     "[printtex_interval_and_defval] Unexpected error for C_id=\"%s\" : illegal ICtype=%d"
#define MSG_ERROR_OPEN_FILE \
     "Cannot open file '%s' for writing"


/*
 * write the header and first LaTeX macros
 */
static void writeTheader(FILE * tfile)
{
     /* comments which may be used by a shell to make the whole doc */
     fprintf(tfile, "%%*** Group=%s\n", group_name);
     fprintf(tfile, "%%*** Name=%s\n\n", module_name);

     fprintf(tfile, "%% This file has been generated\n");
     fprintf(tfile, "%% by the megawave preprocessor.\n");
     fprintf(tfile, "%% It contains the LaTeX document skeleton (doc)\n");
     fprintf(tfile, "%% of the module '%s'.\n\n", module_name);

     /* header for the page */
     fprintf(tfile, "\\markboth");
     fprinttex(tfile, "{{\\em %T} \\hfill megawave User's Modules Library ", \
               group_name);
     fprinttex(tfile, "\\hfill {\\bf %T} \\hspace{1cm}}", module_name);
     fprinttex(tfile, "{{\\em %T} \\hfill megawave User's Modules Library ", \
               group_name);
     fprinttex(tfile, "\\hfill {\\bf %T} \\hspace{1cm}}\n\n", module_name);

     /* label for the index */
     fprintf(tfile, "\\label{%s}\n\n", module_name);
     /* index entry  */
     fprinttex(tfile, "\\index{%T@{\\tt %T}}\n\n", module_name, module_name);
     /* name */
     fprinttex(tfile, "\\Name{%T}{%T}\n\n", module_name, H->Function);
}

/*
 * write the Synopsis LaTeX macro.
 * Code similar to the usage one in the A-file (see writeusage() in afile)
 */
static void writeSynopsis(FILE * tfile)
{
     t_argument * a;

     fprinttex(tfile, "\\Synopsis{%T}{", module_name);
     /* options */
     if (H->NbOption > 0)
          for (a = H->usage; a; a = a->next)
               if (ISARG_OPTION(a))
               {
                    fprintf(tfile, " [-%c", a->Flag);
                    if (a->H_id[0] != '\0')
                         fprinttex(tfile, " {\\em %T}] ", a->H_id);
                    else
                         fprinttex(tfile, "] ");
               }
     /* needed arg */
     if (H->NbNeededArg > 0)
          for (a = H->usage; a; a = a->next)
               if (ISARG_NEEDED(a))
               {
                    if (!(ISARG_SCALAR(a) && ISARG_OUTPUT(a)))
                         fprinttex(tfile, "{\\em %T} ", a->H_id);
                    else
                         /*
                          * in case of a scalar output needed argument,
                          * something (as .) has to be given in the
                          * command line to avoid argument shifting.
                          */
                         fprinttex(tfile, "{\\em .} ");
               }
     /* optional argument */
     if (H->NbOptionArg > 0)
     {
          /*
           * beware!
           * see if it would be useful to check if the following test
           * is never verified for all optional arg, in order not to
           * write [].
           */
          fprinttex(tfile, "[");
          for (a = H->usage; a; a = a->next)
               if (ISARG_OPTARG(a))
               {
                    if (!(ISARG_SCALAR(a) && ISARG_OUTPUT(a)))
                         fprinttex(tfile, "{\\em %T} ", a->H_id);
               }
          fprinttex(tfile, "] ");
     }

     /* variable arguments */
     if (H->NbVarArg > 0)
          for (a = H->usage; a; a = a->next)
               if (ISARG_VARIABLE(a))
               {
                    if (!(ISARG_SCALAR(a) && ISARG_OUTPUT(a)))
                         fprinttex(tfile, "{\\em ...}");
               }

     fprinttex(tfile, "}\n\n");
}


/*
 * Print in the T-file the interval and the default
 * value of the argument's value, if applicable.
 * Code similar to print_interval_and_defval() (see afile)
 *
 */
static void  printtex_interval_and_defval(FILE * tfile, t_argument * a)
{
     char A, B;

     if (!(ISARG_INTERVAL(a) || ISARG_DEFAULT(a)))
          return;

     fprintf(tfile, " (");
     if (ISARG_INTERVAL(a))
     {
          switch (a->ICtype)
          {
          case CLOSED :
               A = '[';
               B = ']';
               break;
          case MAX_EXCLUDED :
               A = '[';
               B = '[';
               break;
          case MIN_EXCLUDED :
               A = ']';
               B = ']';
               break;
          case OPEN :
               A = ']';
               B = '[';
               break;
          default :
               error(MSG_ERROR_ILLEGAL_ICTYPE, a->C_id, a->ICtype);
               /* unknown interval type */
               A = '?';
               B = '?';
               break;
          }
          fprinttex(tfile, "in %c%T, %T%c", A, a->Min, a->Max, B);
          if (ISARG_DEFAULT(a))
               fprintf(tfile, ", ");
     }
     if (ISARG_DEFAULT(a))
          fprintf(tfile, "default %s", a->Val);
     fprintf(tfile, ")");
}

/*
 *
 * write Arguments.
 * Beware, with LaTeX 2e do not use macro \Argument but plain text in
 * order to allow \verb.
 * Code similar to the usage one in the A-file (see writeusage() in afile)
 */

static void writeArguments(FILE * tfile)
{
     t_argument * a;

     fprinttex(tfile, "\n% --- ARGUMENTS : BEGIN --- \n");
     fprinttex(tfile, "\\nopagebreak\n");
     fprinttex(tfile, "\\vspace{-5mm}\n");
     fprinttex(tfile, "\\nopagebreak\n");
     fprinttex(tfile, "\\begin{quotation}\n");

     /* options */
     if (H->NbOption > 0)
          for (a = H->usage; a; a = a->next)
               if (ISARG_OPTION(a))
               {
                    fprintf(tfile, "-%c ", a->Flag);
                    if (a->H_id[0] != '\0')
                         fprinttex(tfile, "{\\em %T}", a->H_id);
                    printtex_interval_and_defval(tfile, a);
                    if (ISARG_SCALAR(a) && ISARG_OUTPUT(a))
                         fprinttex(tfile, " screen output");
                    fprinttex(tfile, " : %T\n\n", getprintfstring(a->Cmt));
               }

     /* needed arg */
     if (H->NbNeededArg > 0)
          for (a = H->usage; a; a = a->next)
               if (ISARG_NEEDED(a))
               {
                    if (!(ISARG_SCALAR(a) && ISARG_OUTPUT(a)))
                    {
                         fprinttex(tfile, "{\\em %T}", a->H_id);
                         printtex_interval_and_defval(tfile, a);
                         fprinttex(tfile, " : %T\n\n", getprintfstring(a->Cmt));
                    }
                    else
                         fprinttex(tfile, ". (screen output) : %T\n\n", \
                                   getprintfstring(a->Cmt));
               }

     /* optional arguments */
     if (H->NbOptionArg > 0)
          for (a = H->usage; a; a = a->next)
               if (ISARG_OPTARG(a))
               {
                    if (!(ISARG_SCALAR(a) && ISARG_OUTPUT(a)))
                    {
                         fprinttex(tfile, "{\\em %T}", a->H_id);
                         printtex_interval_and_defval(tfile, a);
                         fprinttex(tfile, " : %T\n\n", getprintfstring(a->Cmt));
                    }
                    else
                         fprinttex(tfile, "screen output : %T\n\n", \
                                   getprintfstring(a->Cmt));
               }

     /* variable arg */
     if (H->NbVarArg > 0)
          for (a = H->usage; a; a = a->next)
               if (ISARG_VARIABLE(a))
               {
                    if (!(ISARG_SCALAR(a) && ISARG_OUTPUT(a)))
                         fprinttex(tfile, "... : %T\n\n", \
                                   getprintfstring(a->Cmt));
                    else
                         fprinttex(tfile, "screen output... : %T\n\n", \
                                   getprintfstring(a->Cmt));
               }

     fprinttex(tfile, "\\end{quotation}\n");
     fprinttex(tfile, "\\Next\n");
     fprinttex(tfile, "% --- ARGUMENTS : END --- \n\n");

}


/*
 * write Summary
 * Beware, with LaTeX 2e do not use macro \Argument but plain text in
 * order to allow \verb.
 * Code similar to the one in setprotobuf() (See ifile.c).
 * As with setprotobuf, both K&R and ANSI declaration should be implemented.
 */
static void writeSummary(FILE * tfile)
{
     t_varfunc * f;
     t_variable * p;

     fprinttex(tfile, "\n% --- SUMMARY : BEGIN --- \n");
     fprinttex(tfile, "\\Mark{\\Large\\bf  Function Summary} ");
     fprinttex(tfile, "\\nopagebreak\\bigskip\n");
     fprinttex(tfile, "\\nopagebreak\n\n");

     /* function name and arguments list */
     f = C->mfunc;
     fprinttex(tfile, "%T %T ( ", f->v->Ftype, f->v->Name);
     for (p = f->param; p; p = p->next)
     {
          if (p != f->param)
               fprintf(tfile, " , ");
          fprinttex(tfile, "%T", p->Name);
     }
     fprintf(tfile, " )\n\n");

     /* Type of arguments */
     for (p = f->param; p; p = p->next)
          fprinttex(tfile, "%T %T ;\n\n", p->Ftype, p->Name);

     fprinttex(tfile, "\\Next\n");
     fprinttex(tfile, "% --- SUMMARY : END --- \n\n");
}


/*
 * write Description.
 * Beware, with LaTeX 2e do not use macro \Argument but plain text in
 * order to allow \verb.
 */
static void writeDescription(FILE * tfile)
{
     fprinttex(tfile, "\n% --- DESCRIPTION : BEGIN --- \n");
     fprinttex(tfile, "\\Mark{\\Large\\bf  Description} \\nopagebreak\\bigskip\n");
     fprinttex(tfile, "\\nopagebreak\n\n");
     fprinttex(tfile, "\\nopagebreak\n");
     fprintf(tfile, "\\input{src/%s.tex}\n\n", module_name);
     fprinttex(tfile, "\\Next\n");
     fprinttex(tfile, "% --- DESCRIPTION : END --- \n\n");
}

/*
 * write the foot (last LaTeX macros).
 */
static void writeTfoot(FILE * tfile)
{
     char User_DepFile[BUFSIZ];
     char * lastmodifdate;
     char Auth[TREESTRSIZE], Lab[TREESTRSIZE];
     char An0[5], An1[5];

     /* see also */
     sprintf(User_DepFile, "%s.dep", module_name);
     fprintf(tfile, "\\input{obj/DEPENDENCIES/%s}\n\n", User_DepFile);
     /* fet date of last modification of the module file */
     lastmodifdate=ctime(&(module_fstat.st_mtime));
     /* version */
     fprinttex(tfile, "\\Version{%T}{%s}\n", H->Version, lastmodifdate);
     /* author */
     strcpy(An0, "1993");
     strncpy(An1, &lastmodifdate[strlen(lastmodifdate) - 5], 4);
     An1[4]='\0';

     if (H->Labo[0] != '\0')
          strcpy(Lab, getprintfstring(H->Labo));
     else
          strcpy(Lab, "CMLA, ENS Cachan, 94235 Cachan cedex, France");

     strcpy(Auth, getprintfstring(H->Author));
     if (strcmp(An0, An1) == 0)
          fprinttex(tfile, "\\Author{%s}{%s}{%T}\n", Auth, An0, Lab);
     else
          fprinttex(tfile, "\\Author{%s}{%s-%s}{%T}\n", Auth, An0, An1, Lab);
}


/*
 * generate T-file
 */
void genTfile(FILE * tfile)
{
     writeTheader(tfile);
     writeSynopsis(tfile);
     writeArguments(tfile);
     writeSummary(tfile);
     writeDescription(tfile);
     writeTfoot(tfile);
}
