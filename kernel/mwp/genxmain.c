/* -------------------------------------------------------------------------
  genxmain Version 1.7
  (c) Copyright J.Froment and S.Parrino
      Generate module's interface to XMegaWave software.
 -------------------------------------------------------------------------*/

/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifdef XMWP
#include <stdio.h>
#include "bintree.h"
#include "symbol.h"
#include "token.h"
#include "mwarg.h"
#include "data_io.h"
#include "omwarg.h"
#include "y.tab.h"
#include "io.h"
#include "genmain.h"
#define GENXMAIN_DEC
#include "genxmain.h"

static int nb = 0;

/* define */
#define PARAM_NAME	"_mw_Params"

#ifdef __STDC__
extern char *type2string(Node *, int);
extern void print_out_arg(FILE *, Valtype, Type, char *, char *, char *, Node *, char *, int);
extern void remove_dquote(char *, char *);
#else
extern char *type2string();
extern print_out_arg();
extern void remove_dquote();
#endif

/* ---------------------------------------------------------------------------
   genxmain
--------------------------------------------------------------------------- */

#ifdef __STDC__
void genxmain(FILE *fd)
#else
genxmain(fd)
FILE *fd;
#endif
{
  char LowName[BUFSIZ];
  Symbol *mws;

#ifdef DEBUG
  PRDBG("genxmain()\n");
#endif
  if ((mws = LOOKUP(mwname->val.text)) != NULL) {
    char algo_func_name[BUFSIZ], nb_sys_opt_name[BUFSIZ];
#ifdef __STDC__
    extern void print_call_algo_func(FILE *, Symbol *, char *, char*);
    extern void print_gen_win_func(FILE *, char *, char *, char *);
#else
    extern print_call_algo_func();
    extern print_gen_win_func();
#endif
    /* Print default include files */
    fprintf(fd, "#include <stdio.h>\n");
    fprintf(fd, "#include \"XMW2.h\"\n");
    fprintf(fd, "#include \"mw.h\"\n");
    fprintf(fd, "\n");
    
    /* Create functions and variables names */
    sprintf(algo_func_name, "%s%s_mod", MW_PFX2, mws->name);
    sprintf(nb_sys_opt_name, "%sSysOptNb", MW_PFX2);
    
    /* Print default variable declarations */
    fprintf(fd, "static int %s;\n", nb_sys_opt_name);

    /* Define the variable which records the current XMW2 module's function */
    fprintf(fd, "extern void (*xmwfunc)();\n");
    fprintf(fd, "\n");

    /* Generate Input datas, call algorithm and output datas function */
    print_call_algo_func(fd, mws, nb_sys_opt_name, algo_func_name);
    fprintf(fd, "\n");
    
    /* Generate create window function */
    print_gen_win_func(fd, nb_sys_opt_name, mws->name, algo_func_name);

    /* JF 22/7/97 : link de myxmw2 avec libmymw ou le usage est defini
       fprintf(fd, "\nusage_%s()\n", mws->name);
       fprintf(fd, "{\n");
       fprintf(fd, "}\n");
    */


#ifdef DEBUG
    PRDBG("genxmain : end\n");
#endif
  }
  else
    fatal_error("MegaWave function \"%s\" does not exist\n", mwname->val.text);
}

/* ---------------------------------------------------------------------------
   print_call_algo_func
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_call_algo_func(FILE *fd, Symbol *mws, char *nb_sys_opt_name, char *func_name)
#else
print_call_algo_func(fd, mws, nb_sys_opt_name, func_name)
FILE *fd;
Symbol *mws;
char *nb_sys_opt_name;
char *func_name;
#endif
{
#ifdef __STDC__
  extern void print_dec_var(FILE *, Symbol *);
  extern void print_opt_optarg_needed_i(FILE *, char *);
  extern void print_var_mod_io(FILE *, Symbol *, char *);
  extern void print_opt_optarg_needed_o(FILE *, Symbol *);
  extern void print_free(FILE *);
#else
  extern print_dec_var();
  extern print_opt_optarg_needed_i();
  extern print_var_mod_io();
  extern print_opt_optarg_needed_o();
  extern print_free();
#endif
  fprintf(fd, "_MW_PANEL_FUNCTION(%s)\n", func_name);
  fprintf(fd, "{\n");
  fprintf(fd, "  char buffer[BUFSIZ];\n");
  
  /* Generate declaration line for variable */
  print_dec_var(fd, mws);
  
  /* For System options */
  fprintf(fd, "\n/* Call the function which reads input for system opt */\n");
  fprintf(fd, "  _mw_InputSystemOption(_mw_Params);\n");
  
  /* Generate input line for user's option (if needed), optional arguments
     (if needed) and needed arguments */
  print_opt_optarg_needed_i(fd, nb_sys_opt_name);
  
  /* Generate input line for variable argument (if needed), call to the module
     and output line for variable argument */
  print_var_mod_io(fd, mws, nb_sys_opt_name);
  
  /* Generate output line for user's option (if needed), optional arguments
     (if needed) and needed arguments */
  print_opt_optarg_needed_o(fd, mws);
 
  /* Generate free allocation line (if needed) */
  print_free(fd);
  
  fprintf(fd, "}\n");
}


/* ---------------------------------------------------------------------------
   printvaraccess
--------------------------------------------------------------------------- */

#ifdef __STDC__
void printvaraccess(FILE *fd, Io rw, Node *n, char *id)
#else
printvaraccess(fd, rw, n, id)
FILE *fd;
short rw;
Node *n;
char *id;
#endif
{
  if (n != NULL) {
    Node *i, *nid;
    for (i = n; i->left != NULL; i = i->left);
    nid = mkleaf(NAME, NULL, 0, id);
    if (rw == READ && !IS_PTR(n)) {
      Node *nid2;
      nid2 = mknode(DEREF, NULL, 0, NULL, nid);
      i->left = nid2;
      printnode(fd, n);
      free(nid2);
    }
    else {
      i->left = nid;
      printnode(fd, n);
    }
    i->left = (Node *) NULL;
    free(nid);
  }
  else if (rw == READ) {
    Node *nid, *nid2;
    nid = mkleaf(NAME, NULL, 0, id);
    nid2 = mknode(DEREF, NULL, 0, NULL, nid);
    printnode(fd, nid2);
    free(nid2);
    free(nid);
  }
  else
    fprintf(fd ,"%s", id);
}


/* ---------------------------------------------------------------------------
   printcast
     Print in the file <fd> the string
     <id> = ( idtype> )
     where idtype is the type of the C variable <id>
--------------------------------------------------------------------------- */

#ifdef __STDC__
void printcast(FILE *fd, Io rw, Node *t, Node *n, char *id)
#else
printcast(fd, rw, t, n, id)
FILE *fd;
short rw;
Node *t;
Node *n;
char *id;
#endif
{
  fprintf(fd, "%s = (", id);
  printnode(fd, t);
  if (n != NULL) {
    if (rw == READ && !IS_PTR(n)) {
      fprintf(fd, "*");
    }
    else {
      printnode(fd, n);
    }
  }
  else if (rw == READ) {
    fprintf(fd, "*");
  }
  fprintf(fd, ")");
}


/*-----------------------------------------------------------------------------
  print_dec_var
    Print the declaration for the local variables.
-----------------------------------------------------------------------------*/

#ifdef __STDC__
void print_dec_var(FILE *fd, Symbol *mws)
#else
print_dec_var(fd, mws)
FILE *fd;
Symbol *mws;
#endif
{
  Cell *c;
  
  /*-----------------------------------------------------------*/
  fprintf(fd, "\n/* Declarations */\n");
  /*-----------------------------------------------------------*/

  if (GET_NUMBER(optionlist) != 0) {

    /*-----------------------------------------------------------*/
    fprintf(fd, "\n/* Variables for options */\n");
    /*-----------------------------------------------------------*/

    for (c = GET_FIRST(optionlist); c != NULL; c = GET_NEXT(c)) {
      Mwarg *opt;
      opt = (Mwarg *)GET_ELT(c);
      fprintf(fd, "  "); printnode(fd, opt->type);
      if (opt->d.o.d.t == FILEARG && opt->d.o.d.rw == WRITE && opt->access == NULL) {
        Node *n;
        n = mkleaf(DEREF, NULL, 0, NULL);
        printvaraccess(fd, opt->d.o.d.rw, n, opt->name);
        free(n);
      }
      else
        printvaraccess(fd, opt->d.o.d.rw, opt->access, opt->name);
      print_ndecl(fd,opt);  /* Modified by jf 29/9/98 : see omwarg.c */
      
      if (opt->d.o.d.t == SCALARARG && opt->d.o.d.rw == READ) {
        char buffer[BUFSIZ];
        switch(opt->d.o.d.v.p.t) {
          case CHAR_T :
          case UCHAR_T :
          case SHORT_T :
          case USHORT_T :
          case INT_T :
          case UINT_T :
          case LONG_T :
          case ULONG_T :
            fprintf(fd, "  long *%s%s = NULL;\n", MW_PFX2, opt->name);
            break;
          case FLOAT_T :
          case DOUBLE_T :
            fprintf(fd, "  double *%s%s = NULL;\n", MW_PFX2, opt->name);
            break;
          case QSTRING_T :
          case MW2_T :
          case NONE_T :
            /* Nothing to do ! */
            break;
          default :
            INT_ERROR("print_dec_var");
            break;
        }
      }
      else
      if (opt->d.o.d.t == SCALARARG && opt->d.o.d.rw == WRITE) {
        char buffer[BUFSIZ];
        fprintf(fd, "  "); printnode(fd, opt->type);
        sprintf(buffer, "%s%s", MW_PFX2, opt->name);
        printaccess(fd, opt->d.o.d.rw, opt->access, buffer);
        fprintf(fd, ";\n");
      }
    }
  }
  
  if (GET_NUMBER(optarglist)) {
    /*-----------------------------------------------------------*/
    fprintf(fd, "\n/* Variables for optionals arguments  */\n");
    /*-----------------------------------------------------------*/
    for (c = GET_FIRST(optarglist); c != NULL; c = GET_NEXT(c)) {
      Mwarg *opt;
      opt = (Mwarg *)GET_ELT(c);
      fprintf(fd, "  "); printnode(fd, opt->type);
      if (opt->d.a.t == FILEARG && opt->d.a.rw == WRITE && opt->access == NULL) {
        Node *n;
        n = mkleaf(DEREF, NULL, 0, NULL);
        printvaraccess(fd, opt->d.a.rw, n, opt->name);
        free(n);
      }
      else
        printvaraccess(fd, opt->d.a.rw, opt->access, opt->name);

      print_ndecl(fd,opt);  /* Modified by jf 29/9/98 : see omwarg.c */

      if (opt->d.a.t == SCALARARG && opt->d.a.rw == READ) {
        char buffer[BUFSIZ];
        switch(opt->d.a.v.p.t) {
          case CHAR_T :
          case UCHAR_T :
          case SHORT_T :
          case USHORT_T :
          case INT_T :
          case UINT_T :
          case LONG_T :
          case ULONG_T :
            fprintf(fd, "  long *%s%s = NULL;\n", MW_PFX2, opt->name);
            break;
          case FLOAT_T :
          case DOUBLE_T :
            fprintf(fd, "  double *%s%s = NULL;\n", MW_PFX2, opt->name);
            break;
          case QSTRING_T :
          case MW2_T :
          case NONE_T :
            /* Nothing to do ! */
            break;
          default :
            INT_ERROR("print_dec_var");
            break;
        }
      }
      else /* Fin ajout SP 27/06/95 */
      if (opt->d.a.t == SCALARARG && opt->d.a.rw == WRITE) {
        char buffer[BUFSIZ];
        fprintf(fd, "  "); printnode(fd, opt->type);
        sprintf(buffer, "%s%s", MW_PFX2, opt->name);
        printaccess(fd, opt->d.a.rw, opt->access, buffer);
        fprintf(fd, ";\n");
      }
    }
  }
   
  /*-----------------------------------------------------------*/
  fprintf(fd, "\n/* Variables for needed arguments */\n"); 
  /*-----------------------------------------------------------*/

  for (c = GET_FIRST(neededarglist); c != NULL; c = GET_NEXT(c)) {
    Mwarg *opt;
    opt = (Mwarg *)GET_ELT(c);
    if (opt == mwfuncret) {
      fprintf(fd, "  "); printnode(fd, mws->type);
      fprintf(fd, "%s%s ;\n", MW_PFX2, mws->name);
      fprintf(fd, "  extern "); printnode(fd, mws->type);
      printvaraccess(fd, opt->d.a.rw, mws->access, mws->name);
      fprintf(fd, " ;\n");
    }
    else {
      fprintf(fd, "  "); printnode(fd, opt->type);
      if (opt->d.a.t == FILEARG && opt->d.a.rw == WRITE && opt->access == NULL) {
        Node *n;
        n = mkleaf(DEREF, NULL, 0, NULL);
        printvaraccess(fd, opt->d.a.rw, n, opt->name);
        free(n);
      }
      else
        printvaraccess(fd, opt->d.a.rw, opt->access, opt->name);
      
      print_ndecl(fd,opt);  /* Modified by jf 29/9/98 : see omwarg.c */

      if (opt->d.a.t == SCALARARG && opt->d.a.rw == READ) {
        char buffer[BUFSIZ];
        switch(opt->d.a.v.p.t) {
          case CHAR_T :
          case UCHAR_T :
          case SHORT_T :
          case USHORT_T :
          case INT_T :
          case UINT_T :
          case LONG_T :
          case ULONG_T :
/*
            fprintf(fd, "  long %s%s = 0;\n", MW_PFX2, opt->name);
*/
            fprintf(fd, "  long *%s%s = NULL;\n", MW_PFX2, opt->name);
            break;
          case FLOAT_T :
          case DOUBLE_T :
/*
            fprintf(fd, "  double %s%s = 0.0;\n", MW_PFX2, opt->name);
*/
            fprintf(fd, "  double *%s%s = NULL;\n", MW_PFX2, opt->name);
            break;
          case QSTRING_T :
          case MW2_T :
          case NONE_T :
            /* Nothing to do ! */
            break;
          default :
            INT_ERROR("print_dec_var");
            break;
        }
      }
      else /* Fin ajout SP 27/06/95 */
      if (opt->d.a.t == SCALARARG && opt->d.a.rw == WRITE) {
        char buffer[BUFSIZ];
        fprintf(fd, "  "); printnode(fd, opt->type);
        sprintf(buffer, "%s%s", MW_PFX2, opt->name);
        printaccess(fd, opt->d.a.rw, opt->access, buffer);
        fprintf(fd, " ;\n");
      }
    }
  }
  
  if (GET_NUMBER(vararglist) != 0) {
    Mwarg *opt;
    char buffer[BUFSIZ];

    /*-----------------------------------------------------------*/
    fprintf(fd, "\n/* Variables for variables arguments  */\n");
    /*-----------------------------------------------------------*/

    fprintf(fd, "  short %svar_flg = TRUE ;\n", MW_PFX2);
    c = GET_FIRST(vararglist);
    
    opt = (Mwarg *)GET_ELT(c);
    fprintf(fd, "  "); printnode(fd, opt->type);
    if (opt->d.a.t == FILEARG && opt->d.a.rw == WRITE && opt->access == NULL) {
      Node *n;
      n = mkleaf(DEREF, NULL, 0, NULL);
      printvaraccess(fd, opt->d.a.rw, n, opt->name);
      free(n);
    }
    else
      printvaraccess(fd, opt->d.a.rw, opt->access, opt->name);

    print_ndecl(fd,opt);  /* Modified by jf 29/9/98 : see omwarg.c */
      
    switch(opt->d.a.rw) {
      case READ :
        fprintf(fd, "  "); printnode(fd, opt->type);
        sprintf(buffer, "%s%s", MW_PFX2, opt->name);
        printvaraccess(fd, opt->d.a.rw, opt->access, buffer);
	print_ndecl(fd,opt);  /* Modified by jf 29/9/98 : see omwarg.c */
        break;
      case WRITE :
        fprintf(fd, "  int *%s%s_ind = 0;\n", MW_PFX2, opt->name);
        break;
    }
    if (opt->d.a.t == SCALARARG && opt->d.a.rw == READ) {
      char buffer[BUFSIZ];
      switch(opt->d.a.v.p.t) {
        case CHAR_T :
        case UCHAR_T :
        case SHORT_T :
        case USHORT_T :
        case INT_T :
        case UINT_T :
        case LONG_T :
        case ULONG_T :
	  fprintf(fd, "  long *%s%s = NULL;\n", MW_PFX2, opt->name);
          break;
        case FLOAT_T :
        case DOUBLE_T :
	  fprintf(fd, "  double *%s%s = NULL;\n", MW_PFX2, opt->name);
          break;
        case QSTRING_T :
        case MW2_T :
        case NONE_T :
          /* Nothing to do ! */
          break;
        default :
          INT_ERROR("print_dec_var");
          break;
      }
    }
    else /* Fin ajout SP 27/06/95 */
    if (opt->d.a.t == SCALARARG && opt->d.a.rw == WRITE) {
      fprintf(fd, "  "); printnode(fd, opt->type);
      sprintf(buffer, "%s%s", MW_PFX2, opt->name);
      printaccess(fd, opt->d.a.rw, opt->access, buffer);
      fprintf(fd, ";\n");
    }
  }
  
  if (GET_NUMBER(notusedarglist)) {

    /*-----------------------------------------------------------*/
    fprintf(fd, "\n/* Variables for not used arguments  */\n");
    /*-----------------------------------------------------------*/

    for (c = GET_FIRST(notusedarglist); c != NULL; c = GET_NEXT(c)) {
      Mwarg *opt;
      opt = (Mwarg *)GET_ELT(c);
      fprintf(fd, "  "); printnode(fd, opt->type);
      printvaraccess(fd, opt->d.a.rw, opt->access, opt->name);
      print_ndecl(fd,opt);  /* Modified by jf 29/9/98 : see omwarg.c */
    }
  }
}


/* ---------------------------------------------------------------------------
   print_alloc_out_arg
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_alloc_out_arg(FILE *fd, char *name, Io rw, Valtype t, Node *type, Node *access,
                        char *indent, int needed)
#else
print_alloc_out_arg(fd, name, rw, t, type, access, indent, needed)
FILE *fd;
char *name;
short rw;
Valtype t;
Node *type;
Node *access;
char *indent;
int needed;
#endif
{
  if (needed)
    indent+=2;
    
  if (!needed) {
    fprintf(fd, "%sif (%s != NULL) {\n", indent, name);
    fprintf(fd, "%s  free(%s);\n", indent, name);
  }

  switch(t) {
    case FILEARG :
      fprintf(fd, "%s  ", indent);
      printcast(fd, rw, type, access, name);
      fprintf(fd, "malloc(sizeof("); printnode(fd, type); fprintf(fd, "));\n");
      fprintf(fd, "%s  if (%s == NULL) {\n", indent, name);
/*
      fprintf(fd, "%s    fprintf(stderr, \"Cannot alloc memory.\\nExit.\\n\");\n",
                  indent);
      fprintf(fd, "%s    mwexit(1);\n", indent);
*/
      fprintf(fd,"%s  _xmw_Fatal_Error(NULL,199);\n", indent);

      fprintf(fd, "%s  }\n", indent);
      fprintf(fd, "%s  *%s = mw_new_%s();\n", indent, name, type2string(type, FALSE));
      break;
    case SCALARARG :
      fprintf(fd, "%s  %s = &%s%s;\n", indent, name, MW_PFX2, name);
      break;
  }
  if (!needed)
    fprintf(fd, "%s}\n", indent);
}


/* ---------------------------------------------------------------------------
   print_point_scalar_i
     Print pointer of scalar input variables 
     T_GetValue (type returned by _mw_GetValue) is "double" or "long"
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_point_scalar_i(FILE *fd, char *nb_sys_opt_name, char *T_GetValue, Mwarg *opt)
#else
print_point_scalar_i(fd, nb_sys_opt_name, T_GetValue, opt)
FILE *fd;
char *nb_sys_opt_name;
char *T_GetValue;
Mwarg *opt;
#endif

{
  /* Example of print with double -> float */

  /* _mw2_nlevel = (double *) _mw_GetValue... */
/*
  fprintf(fd, "  %s%s = (%s *)_mw_GetValue(%s[%s + %d]);\n", MW_PFX2,
                     opt->name,T_GetValue,PARAM_NAME, nb_sys_opt_name, nb);
*/
  fprintf(fd, "  %s%s = (%s *)_mw_GetValue(%s[%s + %d]);\n", MW_PFX2,
                     opt->name,T_GetValue,PARAM_NAME, nb_sys_opt_name, nb++);
 /* Modif JF 2/7/97 */

  /* if (_mw2_nlevel != NULL) { */
  fprintf(fd,"  if (%s%s != NULL) {\n  ",MW_PFX2,opt->name);

  /* nlevel = (float *) malloc(sizeof(float)); */
  printcast(fd, opt->d.o.d.rw, opt->type, opt->access, opt->name);
  fprintf(fd,"   malloc(sizeof(");
  printnode(fd,opt->type);
  fprintf(fd,"));\n");

  /* if (nlevel == NULL)...*/
  fprintf(fd, "    if (%s == NULL) {\n",opt->name);
/*
  fprintf(fd, "      fprintf(stderr, \"Cannot alloc memory.\\nExit.\\n\");\n");
  fprintf(fd, "      mwexit(1);\n");
*/
  fprintf(fd,"    _xmw_Fatal_Error(NULL,199);\n");

  fprintf(fd, "                    }\n");

 /* *nlevel = (float) * _mw2_nlevel; */
  fprintf(fd,"    *%s = (",opt->name);
  printnode(fd,opt->type);
  fprintf(fd, ") *%s%s;\n",MW_PFX2,opt->name);

  /* free(_mw2_nlevel); */
  fprintf(fd,"    free(%s%s);\n",MW_PFX2,opt->name);

  /* } end of if (_mw2_nlevel != NULL) { */
  fprintf(fd,"  }\n");
}

/* ---------------------------------------------------------------------------
   print_value_scalar_i
     Print value of scalar input variables 
     T_GetValue (type returned by _mw_GetValue) is "double" or "long"
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_value_scalar_i(FILE *fd, char *nb_sys_opt_name, char *T_GetValue, Mwarg *opt)
#else
print_value_scalar_i(fd, nb_sys_opt_name, T_GetValue, opt)
FILE *fd;
char *nb_sys_opt_name;
char *T_GetValue;
Mwarg *opt;
#endif

{
  /* Example of print with double -> float */

  /* _mw2_nlevel = * (double *) _mw_GetValue... */
  /* fprintf(fd, "  %s%s = * (%s *)_mw_GetValue(%s[%s + %d]);\n", MW_PFX2,
                     opt->name,T_GetValue,PARAM_NAME, nb_sys_opt_name, nb);
   */
  fprintf(fd, "  %s%s = * (%s *)_mw_GetValue(%s[%s + %d]);\n", MW_PFX2,
	  opt->name,T_GetValue,PARAM_NAME, nb_sys_opt_name, nb++);
  /* Modif JF 2/7/97 */

  fprintf(fd, "  ");
  /* nlevel = (float *) &_mw2_nelevl; */
  printcast(fd, opt->d.o.d.rw, opt->type, opt->access, opt->name);
  fprintf(fd, " &%s%s;\n", MW_PFX2, opt->name);
}

/*----------------------------------------------------------------------------
  print_opt_optarg_needed_i
    Print input variables 
 ---------------------------------------------------------------------------*/

#ifdef __STDC__
void print_opt_optarg_needed_i(FILE *fd, char *nb_sys_opt_name)
#else
print_opt_optarg_needed_i(fd, nb_sys_opt_name)
FILE *fd;
char *nb_sys_opt_name;
#endif
{
  Cell *c, *c_optarg_read;
  
  /*
   * WARNING : you must respect the order of creation of each set of entries
   *           in function print_gen_win_func when you input datas.
   */
   
  fprintf(fd, "  /* Inputs */\n");

  if (GET_NUMBER(optionlist) != 0) {
    nb++;		/* _mw_CreateSubPanel() creates an entrie in Params[] */
    /*-----------------------------------------------------------*/
    fprintf(fd, "\n/* Inputs for options */\n");
    /*-----------------------------------------------------------*/
    for (c = GET_FIRST(optionlist); c != NULL; c = GET_NEXT(c)) {
      Mwarg *opt;
      opt = (Mwarg *)GET_ELT(c);

      if (opt->d.o.d.t != SCALARARG || opt->d.o.d.rw != READ) 
	{
	  fprintf(fd, "  ");
	  if (opt->d.o.d.t == FILEARG && opt->d.o.d.rw == WRITE && opt->access == NULL) {
	    Node *n;
	    n = mkleaf(DEREF, NULL, 0, NULL);
	    printcast(fd, opt->d.o.d.rw, opt->type, n, opt->name);
	    free(n);
	  }
	  else 
	    printcast(fd, opt->d.o.d.rw, opt->type, opt->access, opt->name);
	  /*
	     fprintf(fd, "_mw_GetValue(%s[%s + %d]);\n", PARAM_NAME, nb_sys_opt_name, nb);
	     */
	  fprintf(fd, "_mw_GetValue(%s[%s + %d]);\n", PARAM_NAME, nb_sys_opt_name, nb++);  /* Modif JF 2/7/97 */
	}
      else {
        char buffer[BUFSIZ];
        fprintf(fd, "  ");
        switch(opt->d.o.d.v.p.t) {
          case CHAR_T :
          case UCHAR_T :
          case SHORT_T :
          case USHORT_T :
          case INT_T :
          case UINT_T :
          case LONG_T :
          case ULONG_T :
	    print_point_scalar_i(fd, nb_sys_opt_name, "long", opt);
            break;
          case FLOAT_T :
          case DOUBLE_T :
	    print_point_scalar_i(fd, nb_sys_opt_name, "double", opt);
            break;
          case QSTRING_T :
          case MW2_T :
          case NONE_T :
            break;
          default :
            INT_ERROR("print_opt_optarg_needed_i");
            break;
        }
      }
      
      if (opt->d.o.d.rw == WRITE) {
        if (opt->d.o.d.t == FILEARG && opt->d.o.d.rw == WRITE && opt->access == NULL) {
          Node *n;
          n = mkleaf(DEREF, NULL, 0, NULL);
          print_alloc_out_arg(fd, opt->name, opt->d.o.d.rw, opt->d.o.d.t, opt->type,
                              n, "  ", FALSE);
          free(n);
        }
        else
          print_alloc_out_arg(fd, opt->name, opt->d.o.d.rw, opt->d.o.d.t, opt->type,
                              opt->access, "  ", FALSE);
      }

      /* nb++; */ /* Modif JF 2/7/97 */

    }       /* end of for (C = GET_FIRST(optionlist)...) */
  }         /* end of if (GET_NUMBER...) */
  
  /* Variables for optionals arguments  */
  if (GET_NUMBER(optarglist) != 0) {
    /*-----------------------------------------------------------*/
    fprintf(fd, "\n/* Inputs for optionals arguments  */\n");
    /*-----------------------------------------------------------*/
    nb++;		/* _mw_CreateSubPanel() creates an entrie in Params[] */
    if (GET_NUMBER(optarglist) > 1) {
      Mwarg *opt;
      c = GET_FIRST(optarglist);
      opt = (Mwarg *)GET_ELT(c);
      
      if (opt->d.a.t != SCALARARG || opt->d.a.rw != READ) {
        fprintf(fd, "  ");
        if (opt->d.a.t == FILEARG && opt->d.a.rw == WRITE && opt->access == NULL) {
          Node *n;
          n = mkleaf(DEREF, NULL, 0, NULL);
          printcast(fd, opt->d.a.rw, opt->type, n, opt->name);
          free(n);
        }
        else
          printcast(fd, opt->d.a.rw, opt->type, opt->access, opt->name);
        /* fprintf(fd, "_mw_GetValue(%s[%s + %d]);\n", PARAM_NAME, nb_sys_opt_name, nb); */
        fprintf(fd, "_mw_GetValue(%s[%s + %d]);\n", PARAM_NAME, nb_sys_opt_name, nb++); /* Modif JF 2/7/97 */
      }
      else {
        char buffer[BUFSIZ];
        fprintf(fd, "  ");
        switch(opt->d.a.v.p.t) {
          case CHAR_T :
          case UCHAR_T :
          case SHORT_T :
          case USHORT_T :
          case INT_T :
          case UINT_T :
          case LONG_T :
          case ULONG_T :
	    print_point_scalar_i(fd, nb_sys_opt_name, "long", opt);
            break;
          case FLOAT_T :
          case DOUBLE_T :
	    print_point_scalar_i(fd, nb_sys_opt_name, "double", opt);
            break;
          case QSTRING_T :
          case MW2_T :
          case NONE_T :
            break;
          default :
            INT_ERROR("print_opt_optarg_needed_i");
            break;
        }
      }

      if (opt->d.a.rw == WRITE) {
        if (opt->d.a.t == FILEARG && opt->d.a.rw == WRITE && opt->access == NULL) {
          Node *n;
          n = mkleaf(DEREF, NULL, 0, NULL);
          print_alloc_out_arg(fd, opt->name, opt->d.a.rw, opt->d.a.t, opt->type,
                              n, "  ", FALSE);
          free(n);
        }
        else
          print_alloc_out_arg(fd, opt->name, opt->d.a.rw, opt->d.a.t, opt->type,
                              opt->access, "  ", FALSE);
      }
      /* nb++; */ /* Modif JF 2/7/97 */
      fprintf(fd, "  if (%s) {\n", opt->name);
      for (c = GET_NEXT(c); c != NULL; c = GET_NEXT(c)) {
        opt = (Mwarg *)GET_ELT(c);
        if (opt->d.a.t != SCALARARG || opt->d.a.rw != READ) {
          fprintf(fd, "    ");
          if (opt->d.a.t == FILEARG && opt->d.a.rw == WRITE && opt->access == NULL) {
            Node *n;
            n = mkleaf(DEREF, NULL, 0, NULL);
            printcast(fd, opt->d.a.rw, opt->type, n, opt->name);
            free(n);
          }
          else 
            printcast(fd, opt->d.a.rw, opt->type, opt->access, opt->name);
          /* fprintf(fd, "_mw_GetValue(%s[%s + %d]);\n", PARAM_NAME, nb_sys_opt_name, nb);*/
	  fprintf(fd, "_mw_GetValue(%s[%s + %d]);\n", PARAM_NAME, nb_sys_opt_name, nb++);       /* Modif JF 2/7/97 */
        }
        else {
          char buffer[BUFSIZ];
          fprintf(fd, "  ");
          switch(opt->d.a.v.p.t) {
            case CHAR_T :
            case UCHAR_T :
            case SHORT_T :
            case USHORT_T :
            case INT_T :
            case UINT_T :
            case LONG_T :
            case ULONG_T :
	      print_point_scalar_i(fd, nb_sys_opt_name, "long", opt);
              break;
            case FLOAT_T :
            case DOUBLE_T :
	      print_point_scalar_i(fd, nb_sys_opt_name, "double", opt);
              break;
            case QSTRING_T :
            case MW2_T :
            case NONE_T :
              break;
            default :
              INT_ERROR("print_opt_optarg_needed_i");
              break;
          }
        }
        if (opt->d.a.rw == WRITE) {
          if (opt->d.a.t == FILEARG && opt->d.a.rw == WRITE && opt->access == NULL) {
            Node *n;
            n = mkleaf(DEREF, NULL, 0, NULL);
            print_alloc_out_arg(fd, opt->name, opt->d.a.rw, opt->d.a.t, opt->type,
                                n, "  ", FALSE);
            free(n);
          }
          else
            print_alloc_out_arg(fd, opt->name, opt->d.a.rw, opt->d.a.t, opt->type,
                                opt->access, "    ", FALSE);
        }
	/* nb++; */ /* Modif JF 2/7/97 */
      }
      fprintf(fd, "  }\n");
    }
    else {
      Mwarg *opt;
      c = GET_FIRST(optarglist);
      opt = (Mwarg *)GET_ELT(c);
      if (opt->d.a.t != SCALARARG || opt->d.a.rw != READ) {
        fprintf(fd, "  ");
        if (opt->d.a.t == FILEARG && opt->d.a.rw == WRITE && opt->access == NULL) {
          Node *n;
          n = mkleaf(DEREF, NULL, 0, NULL);
          printcast(fd, opt->d.a.rw, opt->type, n, opt->name);
          free(n);
        }
        else
          printcast(fd, opt->d.a.rw, opt->type, opt->access, opt->name);
        /* fprintf(fd, "_mw_GetValue(%s[%s + %d]);\n", PARAM_NAME, nb_sys_opt_name, nb); */
	fprintf(fd, "_mw_GetValue(%s[%s + %d]);\n", PARAM_NAME, nb_sys_opt_name, nb++);	/* Modif JF 2/7/97 */
      }
      else {
        char buffer[BUFSIZ];
        fprintf(fd, "  ");
        switch(opt->d.a.v.p.t) {
          case CHAR_T :
          case UCHAR_T :
          case SHORT_T :
          case USHORT_T :
          case INT_T :
          case UINT_T :
          case LONG_T :
          case ULONG_T :
	    print_point_scalar_i(fd, nb_sys_opt_name, "long", opt);
            break;
          case FLOAT_T :
          case DOUBLE_T :
	    print_point_scalar_i(fd, nb_sys_opt_name, "double", opt);
            break;
          case QSTRING_T :
          case MW2_T :
          case NONE_T :
            break;
          default :
            INT_ERROR("print_opt_optarg_needed_i");
            break;
        }
      }
      if (opt->d.a.rw == WRITE) {
        if (opt->d.a.t == FILEARG && opt->d.a.rw == WRITE && opt->access == NULL) {
          Node *n;
          n = mkleaf(DEREF, NULL, 0, NULL);
          print_alloc_out_arg(fd, opt->name, opt->d.a.rw, opt->d.a.t, opt->type,
                              n, "  ", FALSE);
          free(n);
        }
        else
          print_alloc_out_arg(fd, opt->name, opt->d.a.rw, opt->d.a.t, opt->type,
                            opt->access, "  ", FALSE);
      }
      /* nb++; */ /* Modif JF 2/7/97 */
    }
  }

  /*-----------------------------------------------------------*/
  fprintf(fd, "\n/* Inputs for needed arguments */\n");
  /*-----------------------------------------------------------*/
  for (c = GET_FIRST(neededarglist); c != NULL; c = GET_NEXT(c)) {
    Mwarg *opt;
    opt = (Mwarg *)GET_ELT(c);
    if (opt != mwfuncret) {
      switch(opt->d.a.rw) {
        case READ:
          if (opt->d.a.t != SCALARARG) {
            fprintf(fd, "  ");
            printcast(fd, opt->d.a.rw, opt->type, opt->access, opt->name);
            /*
	       fprintf(fd, "_mw_GetValue(%s[%s + %d]);\n", PARAM_NAME, nb_sys_opt_name, nb);
*/
	    fprintf(fd, "_mw_GetValue(%s[%s + %d]);\n", PARAM_NAME, nb_sys_opt_name, nb++); /* Modif JF 2/7/97 */
          }
          else {
            char buffer[BUFSIZ];
            fprintf(fd, "  ");
            switch(opt->d.a.v.p.t) {
              case CHAR_T :
              case UCHAR_T :
              case SHORT_T :
              case USHORT_T :
              case INT_T :
              case UINT_T :
              case LONG_T :
              case ULONG_T :
/*
		print_value_scalar_i(fd, nb_sys_opt_name, "long", opt);
*/
		print_point_scalar_i(fd, nb_sys_opt_name, "long", opt);
                break;
              case FLOAT_T :
              case DOUBLE_T :
/*
		print_value_scalar_i(fd, nb_sys_opt_name, "double", opt);
*/
		print_point_scalar_i(fd, nb_sys_opt_name, "double", opt);
                break;
              case QSTRING_T :
              case MW2_T :
              case NONE_T :
                break;
              default :
                INT_ERROR("print_opt_optarg_needed_i");
                break;
            }
          }
          break;
        case WRITE:
          if (opt->d.a.t == FILEARG && opt->d.a.rw == WRITE && opt->access == NULL) {
            Node *n;
            n = mkleaf(DEREF, NULL, 0, NULL);
            print_alloc_out_arg(fd, opt->name, opt->d.a.rw, opt->d.a.t, opt->type,
                                n, "  ", TRUE);
            free(n);
          }
          else
            print_alloc_out_arg(fd, opt->name, opt->d.a.rw, opt->d.a.t, opt->type,
                                opt->access, "  ", TRUE);
          break;
      }
    }
    /* nb++; */ /* Modif JF 2/7/97 */
  }
}


/* ---------------------------------------------------------------------------
   print_var_mod_io
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_var_mod_io(FILE *fd, Symbol *mws, char *nb_sys_opt_name)
#else
print_var_mod_io(fd, mws, nb_sys_opt_name)
FILE *fd;
Symbol *mws;
char *nb_sys_opt_name;
#endif
{
  Cell *c;
  Node *mwfunc, *idlist, *n;
  char *indent = "";
  
  if (GET_NUMBER(vararglist) != 0) {
    Mwarg *opt;
    indent = "  ";
    c = GET_FIRST(vararglist);
    opt = (Mwarg *)GET_ELT(c);
    switch(opt->d.a.rw) {
      case READ :
        fprintf(fd, "  for(%s%s = %s[%s + %d]; %s%s != NULL || %svar_flg == TRUE; %s%s++) {\n",
                    MW_PFX2, opt->name, PARAM_NAME, nb_sys_opt_name, nb, MW_PFX2, opt->name, 
                    MW_PFX2, MW_PFX2, opt->name);
        fprintf(fd, "    /* Input for variable argument */\n");
        fprintf(fd, "    %s = *%s%s;\n", opt->name, MW_PFX2, opt->name);
        break;
      case WRITE :
        fprintf(fd, "  %s%s_ind = (int *)_mw_GetValue(%s[%s + %d]);\n", MW_PFX2, opt->name, PARAM_NAME,
                    nb_sys_opt_name, nb);
        fprintf(fd, "  for(; *%s%s_ind != 0 || %svar_flg == TRUE; (*%s%s_ind)--) {\n", MW_PFX2,
                    opt->name, MW_PFX2, MW_PFX2, opt->name);
        fprintf(fd, "    /* Variable output for variable argument */\n");
        switch(opt->d.a.t) {
          case FILEARG :
            fprintf(fd, "    ");
            printcast(fd, opt->d.a.rw, opt->type, opt->access, opt->name);
            fprintf(fd, "malloc(sizeof("); printnode(fd, opt->type);
            fprintf(fd, "));\n");
            fprintf(fd, "    if (%s == NULL) {\n", opt->name);
/*
            fprintf(fd, "      fprintf(stderr, \"Cannot alloc memory.\\nExit.\\n\");\n");
            fprintf(fd, "      mwexit(1);\n");
*/
	    fprintf(fd,"    _xmw_Fatal_Error(NULL,199);\n");

            fprintf(fd, "    }\n");
            fprintf(fd, "    *%s = mw_new_%s();\n", opt->name, type2string(opt->type, FALSE));
            break;
          case SCALARARG :
            fprintf(fd, "    /* Output for scalar variable argument %s : do nothing */\n",
                        opt->name);
            break;
        }
        break;
    }
  }
  
  /* Initialisation of pointers to specific trees of MegaWave function */
  mwfunc = (Node *)GET_SDESC(mws);
  for (n = mwfunc->left->left; n->name != FUNCDECL; n=n->left);
  idlist = n->right;
  
  /*-----------------------------------------------------------*/
  fprintf(fd, "\n%s  /* MegaWave function call */\n", indent);
  /*-----------------------------------------------------------*/
  if (idlist != NULL) {
    Symbol *s;
    Mwarg *e;
    if (mwfuncret != NULL)
      fprintf(fd, "%s  %s%s =  %s (",indent, MW_PFX2, mwfuncret->name, mws->name);
    else
      fprintf(fd, "%s  %s (",indent, mws->name);
    n = idlist;
    if (n->name != NAME) {
      for (; n->name == ','; n = n->right) {
        Node *id;
        id = n->left;
        if ((s = LOOKUP(id->val.text)) != NULL) {
          if ((e = (Mwarg *)GET_SFATHER(s)) != NULL) {

/*
            if ((e->t==NEEDEDARG && e->d.a.rw==READ) ||
                (e->t==OPTION && e->d.o.d.t==FILEARG && e->d.o.d.rw==WRITE && e->access==NULL) ||
                (e->t!=OPTION && e->d.a.t==FILEARG && e->d.a.rw==WRITE && e->access==NULL)
               )
*/
/* Modif JF 3/7/97 */
            if ((e->t==NEEDEDARG && e->d.a.rw==READ) ||
                (e->t==OPTION && e->d.o.d.t==FILEARG && e->access==NULL) ||
                (e->t!=OPTION && e->d.a.t==FILEARG && e->access==NULL)
               )

              fprintf(fd, "*%s, ", (char *)id->val.text);
            else
              fprintf(fd, "%s, ", (char *)id->val.text);
          }
          else
            error("'%s' : not declared in usage statement\n", id->val.text);
        }
        else
          error("'%s' : is not declared\n", n->val.text);
      }
    }
    
    /* Last variable in the call */
    if ((s = LOOKUP(n->val.text)) != NULL) {
      if ((e = (Mwarg *)GET_SFATHER(s)) != NULL) {
/*
        if ((e->t == NEEDEDARG && e->d.a.rw == READ) ||
            (e->t==OPTION && e->d.o.d.t==FILEARG && e->d.o.d.rw==WRITE && e->access==NULL) ||
            (e->t!=OPTION && e->d.a.t==FILEARG && e->d.a.rw==WRITE && e->access==NULL)
           )
*/
/* Modif JF 3/7/97 */
        if ((e->t == NEEDEDARG && e->d.a.rw == READ) ||
            (e->t==OPTION && e->d.o.d.t==FILEARG && e->access==NULL) ||
            (e->t!=OPTION && e->d.a.t==FILEARG && e->d.a.rw==WRITE && e->access==NULL)
           )

          fprintf(fd, "*%s) ;\n", (char *)e->name);
        else
          fprintf(fd, "%s) ;\n", (char *)e->name);
      }
      else
        error("'%s' : not declared in usage statement\n", n->val.text);
    }
    else
      error("'%s' : is not declared\n",n->val.text );
  }
  else {
    if (mwfuncret != NULL) {
      fprintf(fd, "%s  %s%s =  %s () ;\n", indent, MW_PFX2, mwfuncret->name, mws->name);
    }
    else
      error("MegaWave function must return datas\n");
  }

  if (GET_NUMBER(vararglist) != 0) {
    Mwarg *opt;
    c = GET_FIRST(vararglist);
    opt = (Mwarg *)GET_ELT(c);
    fprintf(fd, "    %svar_flg = FALSE;\n", MW_PFX2);
    if (opt->d.a.rw = WRITE) {
      fprintf(fd, "    /* Output for variable argument */\n");
      print_out_arg(fd, opt->d.a.t, opt->d.a.v.p.t, opt->name,
                            opt->texname, opt->desc, opt->type, "    ", FALSE);
    }
    fprintf(fd, "  }\n");
  }
}


/* ---------------------------------------------------------------------------
   print_scalar_output
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_scalar_output(FILE *fd, Type t, char *name, char *texname)
#else
print_scalar_output(fd, t, name, texname)
FILE *fd;
Type t;
char *name;
char *texname;
#endif
{
  fprintf(fd, "sprintf(buffer, \"%s = ", texname);
  switch(t) {
    case QSTRING_T:
      fprintf(fd, "%%s");
      break;
    case CHAR_T:
      fprintf(fd, "%%c");
      break;
    case UCHAR_T:
      fprintf(fd, "0x%%X");
      break;
    case SHORT_T:
      fprintf(fd, "%%d");
      break;
    case USHORT_T:
      fprintf(fd, "0x%%X");
      break;
    case INT_T:
      fprintf(fd, "%%d");
      break;
    case UINT_T:
      fprintf(fd, "0x%%X");
      break;
    case LONG_T:
      fprintf(fd, "%%d");
      break;
    case ULONG_T:
      fprintf(fd, "0x%%X");
      break;
    case FLOAT_T:
      fprintf(fd, "%%g");
      break;
    case DOUBLE_T:
      fprintf(fd, "%%g");
      break;
    case MW2_T:
    case NONE_T:
    default :
      INT_ERROR("print_scalar_output");
      break;
  }
  fprintf(fd, "\", %s);\n", name);
}


/* ---------------------------------------------------------------------------
   print_out_arg
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_out_arg(FILE *fd, Valtype ta, Type t, char *name, char *texname, char *desc,
                   Node *type, char *indent, int modret_flg)
#else
print_out_arg(fd, ta, t, name, texname, desc, type, indent, modret_flg)
FILE *fd;
Valtype ta;
Type t;
char *name;
char *texname;
char *desc;
Node *type;
char *indent;
int modret_flg;
#endif
{
  char buffer[BUFSIZ];
  switch(ta) {
    case FILEARG :
      fprintf(fd, "%s_mw_CreateStructWin(%s, (void *)%s%s, %s);\n", indent, desc,
                  modret_flg?"":"*", name, type2string(type, TRUE));
      break;
    case SCALARARG :
      fprintf(fd, "%s", indent);
      sprintf(buffer, "%s%s", MW_PFX2, name);
      print_scalar_output(fd, t, buffer, texname);
      fprintf(fd, "%s_mw_Xprintf(%s, buffer);\n", indent, desc);
      break;
    case FLAGARG :
    default :
      INT_ERROR("print_out_arg");
      break;
  }
}


/* ---------------------------------------------------------------------------
   print_opt_optarg_needed_o
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_opt_optarg_needed_o(FILE *fd, Symbol *mws)
#else
print_opt_optarg_needed_o(fd, mws)
FILE *fd;
Symbol *mws;
#endif
{
  Cell *c;
  int nb;
  
  /*
   * WARNING : you must respect the order of creation of each set of entries
   *           in function print_gen_win_func when you output datas.
   */
   
  fprintf(fd, "\n/* Outputs */\n");

  if (GET_NUMBER(optionlist) != 0) {
    /*-----------------------------------------------------------*/
    fprintf(fd, "\n/* Outputs for options */\n");
    /*-----------------------------------------------------------*/
    for (c = GET_FIRST(optionlist); c != NULL; c = GET_NEXT(c)) {
      Mwarg *opt;
      opt = (Mwarg *)GET_ELT(c);

      if (opt->d.o.d.rw == WRITE) {
        if (opt->d.o.d.t != FLAGARG) {
          fprintf(fd, "  if (%s) {\n", opt->name);
          print_out_arg(fd, opt->d.o.d.t, opt->d.o.d.v.p.t, opt->name,
                            opt->texname, opt->desc, opt->type, "    ", FALSE);
          fprintf(fd, "  }\n");
        }
        else
          fprintf(fd, "\n/* Flag var %s */\n", opt->name);
      }
      else
        fprintf(fd, "\n/* Read var %s */\n", opt->name);
    }
  }
  
  /* Variables for optionals arguments  */
  if (GET_NUMBER(optarglist) != 0) {
    Mwarg *opt;
    /*-----------------------------------------------------------*/
    fprintf(fd, "\n/* Outputs for optionals arguments  */\n");
    /*-----------------------------------------------------------*/
    c = GET_FIRST(optarglist);
    opt = (Mwarg *)GET_ELT(c);

    fprintf(fd, "  if (%s) {\n", opt->name);
    for (c = GET_FIRST(optarglist); c != NULL; c = GET_NEXT(c)) 
      {
	opt = (Mwarg *)GET_ELT(c);
	if (opt->d.a.rw == WRITE)
	  /*
	     print_out_arg(fd, opt->d.a.t, opt->d.a.v.p.t, opt->name,
	     opt->texname, opt->desc, opt->type, "    ", FALSE);
	     */
	  /* Modif JF 7/7/97 */
	  { 
	    fprintf(fd, "  if (%s) {\n", opt->name);
	    print_out_arg(fd, opt->d.a.t, opt->d.a.v.p.t, opt->name,
			  opt->texname, opt->desc, opt->type, "    ", FALSE);
	    fprintf(fd, "  }\n");
	  }
	else
	  fprintf(fd, "    ; /* Read var %s */\n", opt->name);
      }
    fprintf(fd, "  }\n");
  }

  for (c = GET_FIRST(neededarglist); c != NULL; c = GET_NEXT(c)) {
    Mwarg *opt;
    opt = (Mwarg *)GET_ELT(c);
    /*-----------------------------------------------------------*/
    fprintf(fd, "\n/* Outputs for needed arguments  */\n");
    /*-----------------------------------------------------------*/ 
    if (opt->d.a.rw == WRITE) {
      char buffer[BUFSIZ];
      int modret_flg;
      /* if (opt == mwfuncret)  { */
      /* Modif JF 1/8/94 */
      if ((opt == mwfuncret) && (opt->d.a.t != SCALARARG))  {
        sprintf(buffer, "%s%s", MW_PFX2, mws->name);
        modret_flg = TRUE;
      }
      else {
        sprintf(buffer, "%s", opt->name);
        modret_flg = FALSE;
      }
      print_out_arg(fd, opt->d.a.t, opt->d.a.v.p.t, buffer,
                        opt->texname, opt->desc, opt->type, "  ", modret_flg);
    }
    else
      fprintf(fd, "  /* Read var %s */\n", opt->name);
  }
}


/* ---------------------------------------------------------------------------
   print_free
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_free(FILE *fd)
#else
print_free(fd)
FILE *fd;
#endif
{
  Cell *c;
  
  fprintf(fd, "\n/* Free Motif function allocations */\n");

  if (GET_NUMBER(optionlist) != 0) {
    fprintf(fd, "  /* Free variables for options */\n");
    for (c = GET_FIRST(optionlist); c != NULL; c = GET_NEXT(c)) {
      Mwarg *opt;
      opt = (Mwarg *)GET_ELT(c);
      if (opt->d.o.d.rw == READ)
        fprintf(fd, "  free(%s);\n", opt->name);
      else
        fprintf(fd, "  /* Not free output (%s) */\n", opt->name);
    }
  }
  
  /* Variables for optionals arguments  */
  if (GET_NUMBER(optarglist)) {
    fprintf(fd, "  /* Free variables for optionals arguments  */\n");
    for (c = GET_FIRST(optarglist); c != NULL; c = GET_NEXT(c)) {
      Mwarg *opt;
      opt = (Mwarg *)GET_ELT(c);
      if (opt->d.a.rw == READ)
        fprintf(fd, "  free(%s);\n", opt->name);
      else
        fprintf(fd, "  /* Not free output (%s) */\n", opt->name);
    }
  }
   
  /* Variables for needed arguments  */
  fprintf(fd, "  /* Free variables for needed arguments */\n");
  for (c = GET_FIRST(neededarglist); c != NULL; c = GET_NEXT(c)) {
    Mwarg *opt;
    opt = (Mwarg *)GET_ELT(c);
    
    if (opt != mwfuncret)
      fprintf(fd, "  free(%s);\n", opt->name);
   }
  
  /* Variables for variables arguments  */
  if (GET_NUMBER(vararglist) != 0) {
    Mwarg *opt;
    fprintf(fd, "  /* Free variables for variable argument */\n");
    c = GET_FIRST(vararglist);
    opt = (Mwarg *)GET_ELT(c);
    fprintf(fd, "  free(%s);\n", opt->name);
    if (opt->d.a.rw == WRITE)
      fprintf(fd, "  free(%s%s_ind);\n", MW_PFX2, opt->name);
  }
}

/* ---------------------------------------------------------------------------
   print_gen_win_func
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_gen_win_func(FILE *fd, char *nb_sys_opt_name, char *mod_name,
                                  char *func_to_call_name)
#else
print_gen_win_func(fd, nb_sys_opt_name, mod_name, func_to_call_name)
FILE *fd;
char *nb_sys_opt_name;
char *mod_name;
char *func_to_call_name;
#endif
{
#ifdef __STDC__
  extern void print_gen_needed_arg_win_func(FILE *);
  extern void print_gen_option_win_func(FILE *);
  extern void print_gen_opt_arg_win_func(FILE *);
  extern void print_gen_var_arg_win_func(FILE *);
#else
  extern print_gen_needed_arg_win_func();
  extern print_gen_option_win_func();
  extern print_gen_opt_arg_win_func();
  extern print_gen_var_arg_win_func();
#endif

  char func_name[BUFSIZ], group_name[BUFSIZ];
  sprintf(func_name, "_mw2_%s", mod_name);

  fprintf(fd, "void %s(argc, argv)\n", func_name);
  fprintf(fd, "int argc;\n");
  fprintf(fd, "char *argv[];\n");
  fprintf(fd, "{\n");
  fprintf(fd, "  xmwfunc = %s;\n",func_name);
  fprintf(fd, "  _mw_BeginCreatePanel(%s);\n", mwfunction->val.text);
  
  /* For System options */
  fprintf(fd, "    /* Call the function which creates window for system opt */\n");
  fprintf(fd, "    %s = _mw_CreateSystemOption();\n", nb_sys_opt_name);

  /* Generate functions which create windows for options */
  print_gen_option_win_func(fd);

  /* Generate functions which create windows for optional arguments */
  print_gen_opt_arg_win_func(fd);

  /* Generate functions which create windows for needed arguments */
  print_gen_needed_arg_win_func(fd);
  
  /* Generate functions which create windows for variable arguments */
  print_gen_var_arg_win_func(fd);
  
  /* Compute the name of the modules's group */
  if ((mwgroup != NULL) && (mwgroup->val.text != NULL))
    remove_dquote(group_name,mwgroup->val.text);
  else strcpy(group_name,"??");
  
  /* End of the function */
  fprintf(fd, "  _mw_EndCreatePanel(%s, \"%s\", \"%s\");\n", 
	  func_to_call_name, mod_name, group_name);
  fprintf(fd, "}\n");
}


#define MAXNAME 255

/* ---------------------------------------------------------------------------
   type2string
--------------------------------------------------------------------------- */

#ifdef __STDC__
char *type2string(Node *n, int quote)
#else
char *type2string(n, quote)
Node *n;
int quote;
#endif
{
  static char buffer[MAXNAME];
  if (quote)
    sprintf(buffer, "\"%s\"", n->val.text);
  else
    sprintf(buffer, "%s", n->val.text);    
  Lowerline(buffer);
  return buffer;
}


/* ---------------------------------------------------------------------------
   scalar2string
--------------------------------------------------------------------------- */

#ifdef __STDC__
char *scalar2string(Type t, Paramvalue p, char *cast)
#else
char *scalar2string(t, p, cast)
short t;
Paramvalue p;
char *cast;
#endif
{
#define _CAST ((cast) ? cast : "")
  static char buffer[BUFSIZ];
  switch(t) {
    case CHAR_T :
      sprintf(buffer, "%s'%c'", _CAST, p.c);
      break;
    case UCHAR_T:
      sprintf(buffer, "%s'%c'", _CAST, p.uc);
      break;
    case SHORT_T :
      sprintf(buffer, "%s%d", _CAST, p.s);
      break;
    case USHORT_T:
      sprintf(buffer, "%s%d", _CAST, p.us);
      break;
    case INT_T :
      sprintf(buffer, "%s%d", _CAST, p.i);
      break;
    case UINT_T:
      sprintf(buffer, "%s%d", _CAST, p.ui);
      break;
    case LONG_T :
      sprintf(buffer, "%s%d", _CAST, p.l);
      break;
    case ULONG_T:
      sprintf(buffer, "%s%d", _CAST, p.ul);
      break;
    case FLOAT_T :
      sprintf(buffer, "%s%g", _CAST, p.f);
      break;
    case DOUBLE_T:
      sprintf(buffer, "%s%g", _CAST, p.d);
      break;
    default:
      INT_ERROR("scalar2string");
      break;
  }
  return buffer;
#undef CAST
}


/* ---------------------------------------------------------------------------
   print_gen_input_scalar
     Generic print of an input scalar function (Option or Argument), of the
     general form
       _mw_Create{String/Char/Integer/Real}{Option/Arg/DefArg}[_Slider]
     Warning : the number of the function's arguments may vary in 2 ways
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_gen_input_scalar(FILE *fd, char *comment, Desc d, char *arg_type, 
			    char *indent)
#else
print_gen_input_scalar(fd, comment, d, arg_type, indent)
FILE *fd;
char *comment;
Desc d;
char *arg_type;        /* Contains "Option" or "Arg" */
char *indent;
#endif
{
  if (d.t == SCALARARG && d.rw == READ) 
    {
      char func_name[BUFSIZ], func_arg[BUFSIZ], buffer[BUFSIZ];
      char s[10];

      strcpy(s,arg_type);
      sprintf(func_name, "%s_mw_Create", indent);
      sprintf(func_arg, "(%s", comment);
      switch(d.v.p.t) 
	{
	case QSTRING_T:              /* _mw_CreateString{Option/Arg} */
	  strcat(func_name, "String");
	  sprintf(buffer, ", (char *)%s", d.v.p.d ? d.v.p.d->q:"\"\"");
	  strcat(func_arg, buffer); 
	  break;
	case CHAR_T:                 /* _mw_CreateChar{Option/Arg/DefArg} */
	case UCHAR_T:               
	  strcat(func_name, "Char");
	  if (d.v.p.d)
	    /* The option or argument has a default value */
	    {
	      sprintf(buffer, ", %s, (char) 1", 
		      scalar2string(d.v.p.t, *(d.v.p.d), "(char)"));
	      if (strcmp(s,"Arg") == 0) strcpy(s,"DefArg");
	    }
	  else
	    /* No default value : don't use it */
	    {
	      if (strcmp(s,"Arg") == 0) sprintf(buffer, "");  
	      else sprintf(buffer, ", '\\0', (char) 0");
	    }
	  strcat(func_arg, buffer); 
	  break;

	case SHORT_T:
	case USHORT_T:
	case INT_T:              /* _mw_Create_Integer_{Option/Arg} */
	case UINT_T: 
	case LONG_T:
	case ULONG_T:          
	  strcat(func_name, "Integer");
	  if (d.v.p.d)
	    /* The option or argument has a default value */
	    {
	      sprintf(buffer, ", %s, (char) 1", 
		      scalar2string(d.v.p.t, *(d.v.p.d), "(long)"));
	      if (strcmp(s,"Arg") == 0) strcpy(s,"DefArg");
	    }
	  else
	    /* No default value : don't use it */	 
	    {
	      if (strcmp(s,"Arg") == 0) sprintf(buffer, "");  
	      else sprintf(buffer, ", 0L, (char) 0");
	    }
	  strcat(func_arg, buffer); 
	  break;

	case FLOAT_T:             /* _mw_Create_Real_{Option/Arg} */
	case DOUBLE_T: 
	  strcat(func_name, "Real");
	  if (d.v.p.d)
	    /* The option or argument has a default value */
	    {
	      sprintf(buffer, ", %s, (char) 1", 
		      scalar2string(d.v.p.t, *(d.v.p.d), "(double)"));
	      if (strcmp(s,"Arg") == 0) strcpy(s,"DefArg");
	    }
	  else
	    /* No default value : don't use it */	 
	    {
	      if (strcmp(s,"Arg") == 0) sprintf(buffer, "");  
	      else sprintf(buffer, ", 0.0, (char) 0");
	    }
	  strcat(func_arg, buffer); 
	  break;

	default:
	  INT_ERROR("print_gen_input_scalar : illegal type");
	  break;
	}

    strcat(func_name, s);

    if (d.v.p.i != NULL) 
      /* Interval or Slider */
      {
	strcat(func_name, "Slider");
	switch(d.v.p.t) {
        case QSTRING_T:
          INT_ERROR
	    ("print_gen_input_scalar: interval not possible for string");
          break;
        case CHAR_T:
        case UCHAR_T:
          sprintf(buffer, ", %s", 
		  scalar2string(d.v.p.t, d.v.p.i->min, "(char)"));
          strcat(func_arg, buffer);
          sprintf(buffer, ", %s", 
		  scalar2string(d.v.p.t, d.v.p.i->max, "(char)"));
          strcat(func_arg, buffer);
          break;
        case SHORT_T:
        case USHORT_T:
        case INT_T:
        case UINT_T:
        case LONG_T:
        case ULONG_T:
          sprintf(buffer, ", %s", 
		  scalar2string(d.v.p.t, d.v.p.i->min, "(long)"));
          strcat(func_arg, buffer);
          sprintf(buffer, ", %s", 
		  scalar2string(d.v.p.t, d.v.p.i->max, "(long)"));
          strcat(func_arg, buffer);
          break;
        case FLOAT_T:
        case DOUBLE_T:
          sprintf(buffer, ", %s", 
		  scalar2string(d.v.p.t, d.v.p.i->min, "(double)"));
          strcat(func_arg, buffer);
          sprintf(buffer, ", %s", 
		  scalar2string(d.v.p.t, d.v.p.i->max, "(double)"));
          strcat(func_arg, buffer);
          break;
        default:
          INT_ERROR("print_gen_input_scalar : illegal type");
          break;
	}
      }

      strcat(func_arg, ")"); 
    
      fprintf(fd, "  %s%s;\n", func_name, func_arg);
    }
  else
    INT_ERROR("illegal call to print_gen_input_scalar");
}

/* ---------------------------------------------------------------------------
print_gen_needed_arg_win_func
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_gen_needed_arg_win_func(FILE *fd)
#else
print_gen_needed_arg_win_func(fd)
FILE *fd;
#endif
{
  if (GET_NUMBER(neededarglist) != 0) {
    Cell *c;
    /*-----------------------------------------------------------*/
    fprintf(fd, "\n/* Generate window for needed argument */\n");
    /*-----------------------------------------------------------*/
    for (c = GET_FIRST(neededarglist); c != NULL; c = GET_NEXT(c)) {
      Mwarg *opt;
      opt = (Mwarg *)GET_ELT(c);
      if (opt != mwfuncret) {
        switch(opt->d.a.t) {
          case FILEARG :
            if (opt->d.a.rw == READ) {
              fprintf(fd, "    _mw_CreateStructArg(%s, %s);\n", opt->desc,
                                               type2string(opt->type, TRUE));
            }
            else if (opt->d.a.rw == WRITE) {
              fprintf(fd, "    /* Structure output for %s : do nothing */\n", opt->name);
	    }
	    else
	      INT_ERROR("print_gen_needed_arg_win_func");
            break;
          case SCALARARG :
            if (opt->d.a.rw == READ) {
              print_gen_input_scalar(fd, opt->desc, opt->d.a, "Arg", "  ");
            }
            else if (opt->d.a.rw == WRITE) {
              fprintf(fd, "    /* Scalar output for %s : do nothing */\n", opt->name);
            }
            else
              INT_ERROR("print_gen_needed_arg_win_func");
            break;
          case FLAGARG :
#ifdef DEBUG
            PRDBG("print_gen_needed_arg_win_func : FLAGARG\n");
#endif
            INT_ERROR("print_gen_needed_arg_win_func");
          default :
#ifdef DEBUG
            PRDBG("print_gen_needed_arg_win_func : %d : unknown type of argument\n", opt->d.o.d.t);
#endif
            fatal_error("%d : unknown type of argument\n", opt->d.a.t);

            break;
        }
      }
      else
        fprintf(fd, "    /* Module return output %s */\n", opt->name);
    }
  }
  else
    INT_ERROR("print_gen_needed_arg_win_func");
}


/* ---------------------------------------------------------------------------
 print_gen_option_win_func
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_gen_option_win_func(FILE *fd)
#else
print_gen_option_win_func(fd)
FILE *fd;
#endif
{
  if (GET_NUMBER(optionlist) != 0) {
    Cell *c;
    fprintf(fd, "\n/* Generate window for option */\n");
    fprintf(fd, "    _mw_BeginSubPanel(\"User's options\");\n");
    for (c = GET_FIRST(optionlist); c != NULL; c = GET_NEXT(c)) {
      Mwarg *opt;
      opt = (Mwarg *)GET_ELT(c);
      switch(opt->d.o.d.t) {
        case FILEARG :
          if (opt->d.o.d.rw == READ) {
            fprintf(fd, "      _mw_CreateStructOption(%s, %s);\n", opt->desc,
                                                 type2string(opt->type, TRUE));
          }
          else if (opt->d.o.d.rw == WRITE) {
            fprintf(fd, "      _mw_CreateFlag(%s);\n", opt->desc);  
	  }
	  else
	    INT_ERROR("print_gen_option_win_func");
          break;
        case SCALARARG :
          if (opt->d.o.d.rw == READ) {
            print_gen_input_scalar(fd, opt->desc, opt->d.o.d, "Option", "    ");
          }
          else if (opt->d.o.d.rw == WRITE) {
            fprintf(fd, "      _mw_CreateFlag(%s);\n", opt->desc);  
          }
          else
            INT_ERROR("print_gen_option_win_func");
          break;
        case FLAGARG :
          fprintf(fd, "      _mw_CreateFlag(%s);\n", opt->desc);
          break;
        default :
#ifdef DEBUG
          PRDBG("print_gen_needed_arg_win_func : %d : unknown type of argument\n", opt->d.o.d.t);
#endif
          fatal_error("%d : unknown type of argument\n", opt->d.o.d.t);

          break;
      }
    }
    fprintf(fd, "    _mw_EndSubPanel();\n");
  }
}


/* ---------------------------------------------------------------------------
   print_gen_opt_arg_win_func
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_gen_opt_arg_win_func(FILE *fd)
#else
print_gen_opt_arg_win_func(fd)
FILE *fd;
#endif
{
  if (GET_NUMBER(optarglist) != 0) {
    Cell *c;
    fprintf(fd, "\n/* Generate window for optional arguments */\n");
    fprintf(fd, "    _mw_BeginSubPanel(\"Optional arguments\");\n");
    for (c = GET_FIRST(optarglist); c != NULL; c = GET_NEXT(c)) {
      Mwarg *opt;
      opt = (Mwarg *)GET_ELT(c);
      switch(opt->d.a.t) {
        case FILEARG :
          if (opt->d.a.rw == READ) {
            fprintf(fd, "      _mw_CreateStructArg(%s, %s);\n", opt->desc,
                                                 type2string(opt->type, TRUE));
          }
	  else if (opt->d.a.rw == WRITE) 
	    {
	      /* fprintf(fd, "      _mw_CreateFlag(%s);\n", opt->desc);*/
	      /* Modif JF 7/7/97 */
	      fprintf(fd, "      _mw_CreateOutputOptArg(%s);\n", opt->desc);
	     }
	  else
	    INT_ERROR("print_gen_opt_arg_win_func");
          break;
        case SCALARARG :
          if (opt->d.a.rw == READ) {
            print_gen_input_scalar(fd, opt->desc, opt->d.a, "Arg", "    ");
          }
          else if (opt->d.a.rw == WRITE) 
	    {
	      /* fprintf(fd, "      _mw_CreateFlag(%s);\n", opt->desc); */
	      /* Modif JF 7/7/97 */
	      fprintf(fd, "      _mw_CreateOutputOptArg(%s);\n", opt->desc);
	    }
          else
            INT_ERROR("print_gen_opt_arg_win_func");
          break;
        case FLAGARG :
#ifdef DEBUG
          PRDBG("print_gen_opt_arg_win_func : FLAGARG\n");
#endif
          INT_ERROR("print_gen_opt_arg_win_func");
        default :
#ifdef DEBUG
          PRDBG("print_gen_opt_arg_win_func : %d : unknown type of argument\n", opt->d.o.d.t);
#endif
          fatal_error("%d : unknown type of argument\n", opt->d.a.t);

          break;
      }
    }
    fprintf(fd, "    _mw_EndSubPanel();\n");
  }
}


/* ---------------------------------------------------------------------------
   print_gen_var_arg_win_func
--------------------------------------------------------------------------- */

#ifdef __STDC__
void print_gen_var_arg_win_func(FILE *fd)
#else
print_gen_var_arg_win_func(fd)
FILE *fd;
#endif
{
  if (GET_NUMBER(vararglist) != 0) {
    Cell *c;
    Mwarg *opt;
    fprintf(fd, "\n/* Generate window for variable arguments */\n");
    c = GET_FIRST(vararglist);
    opt = (Mwarg *)GET_ELT(c);
    switch(opt->d.a.t) {
      case FILEARG :
        if (opt->d.a.rw == READ) {
          fprintf(fd, "    _mw_CreateStructVar(%s, %s);\n", opt->desc,
                                              type2string(opt->type, TRUE));
        }
        else if (opt->d.a.rw == WRITE) {
          fprintf(fd, "    _mw_CreateIntegerOption(%s, 1, 1);\n", opt->desc);
	}
	else
	  INT_ERROR("print_gen_var_arg_win_func");
        break;
      case SCALARARG :
        warning("No i/o scalar are allowed for variable arguments with -X option.\n");
        warning("-> Do nothing\n");
        fprintf(fd, "  /* i/o scalar for %s : do nothing*/\n", opt->name);
        break;
      case FLAGARG :
#ifdef DEBUG
        PRDBG("print_gen_needed_arg_win_func : FLAGARG\n");
#endif
        INT_ERROR("print_gen_var_arg_win_func");
      default :
#ifdef DEBUG
        PRDBG("print_gen_var_arg_win_func : %d : unknown type of argument\n", opt->d.o.d.t);
#endif
        fatal_error("%d : unknown type of argument\n", opt->d.a.t);
        break;
    }
  }
}
#endif
