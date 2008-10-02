/*
 * parse.c for megawave, section mwplight
 *
 * parse the moduke source
 *
 * author : Jacques Froment <jacques.froment@univ-ubs.fr> (2005 - 2007)
 * author : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

/* FIXME: drop */
#define _POSIX_SOURCE
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mwplight-defs.h"
#include "io.h"
#include "tree.h"
#include "header.h"
#include "instruction.h"

#include "parser.h"

/* t_header ID for megawave */
#define HEADER_ID1 "/* mwcommand"
#define HEADER_ID1b "/*mwcommand"
/* New header ID for megawave */
#define HEADER_ID2 "/* mwheader"
#define HEADER_ID2b "/*mwheader"

#define MSG_ERROR_NULL_FILE_PTR \
   "NULL file pointer fs"
#define MSG_ERROR_HEADER_ID \
   "Cannot find megawave t_header ID ('%s' or '%s')"
#define MSG_ERROR_INCORRECT_ARGUMENT \
   "Invalid usage for C_id=\"%s\" : incorrect argument type"
#define MSG_ERROR_MAIN_FUNCTION \
   "Cannot find module's function in C body.\nRemember that the main function's name must be the same that the module's filename"
#define MSG_ERROR_NULL_TREE \
   "[CompleteHC] NULL C tree. Need C body to be parsed"
#define MSG_ERROR_NULL_MAIN \
   "[CompleteHC] NULL main function"
#define MSG_ERROR_UNSUPPORTED_RETURN \
   "Unsupported return type \"%s\" for the main function (Ctype=%d).\nReturn types can be only scalar or megawave internal types.\nAlso, remember that the light preprocessor does not process #define and #include directives."
#define MSG_ERROR_TWO_RETURNS \
   "Two variables '%s' and '%s' on the return value of the main function"
#define MSG_ERROR_NO_RETURN \
   "Invalid usage for C_id=\"%s\" : the main function does not return any value"
#define MSG_ERROR_MISSING_VARIABLE \
   "t_variable '%s' is declared in the header but is missing in main function"
#define MSG_ERROR_UNSUPPORTED_PARAMETER \
   "Unsupported type \"%s\" for the parameter \"%s\" of the main function (Ctype=%d).\nI/O variables can be only of scalar types or of megawave internal types (or pointers to).\nAlso, remember that the light preprocessor does not process #define and #include directives."
#define MSG_ERROR_MISSING_PARAMETER \
     "Parameter '%s' of the main function is not declared in the header"
#define MSG_ERROR_OPEN_FILE \
     "Cannot open file \"%s\" for reading"
#define MSG_ERROR_EOF \
     "Unexpected end of file while parsing the header !"

#define MSG_DEBUG_FUNC \
     "entering %s"

/*
 * Enter the header.
 * Return the header ID number (1 or 2).
 */
static int EnterHeader(FILE * sfile)
{
     char line[STRSIZE];
     int l;
     short version = 0;

     if (sfile == NULL)
          error(MSG_ERROR_NULL_FILE_PTR);
     do
     {
          l = getline(sfile, line);
          removespaces(line);
          /*printf("line='%s'\n",line);*/
          if ((strcmp(line, HEADER_ID1) == 0) || \
              (strcmp(line, HEADER_ID1b) == 0))
          {
               version = 1;
               break;
          }
          if ((strcmp(line, HEADER_ID2) == 0) || \
              (strcmp(line, HEADER_ID2b) == 0))
          {
               version = 2;
               break;
          }
     }
     while (l != EOF);
     if (version == 0)
          error(MSG_ERROR_HEADER_ID, HEADER_ID1, HEADER_ID2);
     return version;
}


/*
 * Parse the header.
 * Return EOF (error) or EOH (normal case).
 */

static int ParseHeader(FILE * sfile)
{
     char s[STRSIZE];
     char name[STRSIZE];
     char value[STRSIZE];
     int l;
     t_argument * a;

     if (sfile == NULL)
          error(MSG_ERROR_NULL_FILE_PTR);
     while (((l = getsentence(sfile, s)) != EOF) && (l != EOH))
     /* s contains the sentence to parse */
     {
          GetHeaderStatement(s, name, value);
          AnalyseHeaderStatement(name,value);
     }
     inside_header=0;

     /* compute the number of arguments, by types */
     for (a = H->usage; a; a = a->next)
          switch(a->Atype)
          {
          case OPTION:
               H->NbOption++;
               break;
          case NEEDEDARG:
               H->NbNeededArg++;
               break;
          case VARARG:
               H->NbVarArg++;
               break;
          case OPTIONARG:
               H->NbOptionArg++;
               break;
          case NOTUSEDARG:
               H->NbNotUsedArg++;
               break;
          default:
               error(MSG_ERROR_INCORRECT_ARGUMENT, a->C_id);
          }

     return l;
}


/*
 * Parse the C body.
 */
static void ParseCbody(FILE * sfile)
{
     if (debug_flag)
          debug(MSG_DEBUG_FUNC, __func__);

     if (sfile == NULL)
          error(MSG_ERROR_NULL_FILE_PTR);

     Init_Cuserdatatype();

     while (GetNextInstruction(sfile) != 0);

     if (C->mfunc == NULL)
          error(MSG_ERROR_MAIN_FUNCTION);

     Free_Cuserdatatype();
}

/*
 * Complete the H tree by setting fields that need the C body to be
 * parsed (C tree filled), i.e. Ctype, Vtype and v fields, and complete
 * the C tree with arg field. Perform also some checking.
 */
/* TODO: store ERR_MSG_FOO */
static void CompleteHC(void)
{
     t_varfunc * f;
     t_argument * a;
     t_variable * v, * p;

     if (C == NULL)
          error(MSG_ERROR_NULL_TREE);
     f = C->mfunc;
     if (f == NULL)
          error(MSG_ERROR_NULL_MAIN);
     if ((f->v->Ctype != VOID_T) && (f->v->Ctype != MW2_T) && \
         ((f->v->Ctype < SCALARMIN_T) || (f->v->Ctype > SCALARMAX_T)))
     {
          if (f->v->Ctype != NONE)
               error(MSG_ERROR_UNSUPPORTED_RETURN, \
                     f->v->Ftype, f->v->Ctype);

          /*
           * Main function does not have any declaration of type.
           * In such a case, the C language considers the type as "int".
           */
          f->v->Ctype = INT_T;
          strcpy(f->v->Stype, "int");
          strcpy(f->v->Ftype, "int");
          f->v->PtrDepth = 0;
     }

     for (a = H->usage; a; a = a->next)
     {
          /*
           * For each argument in the usage, search for the
           * corresponding parameter in the main function.
           */
          for (p = f->param; p && (strcmp(p->Name,a->C_id) != 0); p = p->next);
          if (p)
               /* parameter found */
          {
               a->var = p;
               p->arg = a;
          }
          else
               /*
                * not found
                * check if this variable is not the return value of
                * the function
                */
          {
               if (strcmp(f->v->Name,a->C_id)==0)
                    /* yes, this is the return value of the function */
               {
                    if (H->retmod)
                         error(MSG_ERROR_TWO_RETURNS, H->retmod->C_id, a->C_id);
                    a->var = f->v;
                    f->v->arg = a;
                    H->retmod = a;
                    if (a->var->Ctype == VOID_T)
                         error(MSG_ERROR_NO_RETURN, a->C_id);
               }
               else
                    error(MSG_ERROR_MISSING_VARIABLE, a->C_id);
          }
     }

     for (v = f->param; v; v = v->next)
     {
          if ((v->Ctype != MW2_T) && ((v->Ctype < SCALARMIN_T) || \
                                      (v->Ctype > SCALARMAX_T)))
               error(MSG_ERROR_UNSUPPORTED_PARAMETER, \
                     v->Ftype, v->Name, v->Ctype);
          if (!v->arg)
               error(MSG_ERROR_MISSING_PARAMETER, \
                     v->Name);
     }

}

/*
 * Parse the module
 */
void parse(FILE * sfile)
{

     H = new_header();
     C = new_cbody();

     /* store info about the module file */
     /* TODO: check, missing headers */
     fstat(fileno(sfile), &module_fstat);
     /* init variables */
     inside_comment = 0;
     inside_header = 0;
     /* parse the header and build the H tree */
     inside_header = EnterHeader(sfile);
     if (ParseHeader(sfile) == EOF)
          error(MSG_ERROR_EOF);

     /* check consistency of the header loaded in H */
     CheckConsistencyH();
     /* parse the C body and build the C tree */
     ParseCbody(sfile);
     /* complete the H and C trees and perform some checking */
     CompleteHC();
}
