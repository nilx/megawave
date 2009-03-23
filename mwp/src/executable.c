/**
 * @file executable.c
 *
 * generate the executable source for a megawave module
 *
 * @author Jacques Froment <jacques.froment@univ-ubs.fr> (2005 - 2007)
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

/* TODO: deep review */
/* TODO: compact strings, preserve indentation */
/* TODO: add end of if comments */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "definitions.h"

#include "io.h"
#include "usage.h"

#include "executable.h"

#define MSG_ERROR_CONVERSION \
    "[SetScalarConvFunction] No scalar conversion method implemented for C_id=\"%s\" of Ctype=%d (Stype=\"%s\")"
#define MSG_ERROR_PRINT \
    "[print_value_scalar_arg] No scalar print value method implemented for C_id=\"%s\" of Ctype=%d (Stype=\"%s\")"
#define MSG_ERROR_INVALID_DT_FTYPE_AUXVAR \
    "[writedefvar] Invalid declaration type DT_Ftype_auxvar for C_id=\"%s\" of Ctype=%d (Stype=\"%s\", Ftype=\"%s\") : not a pointer"
#define MSG_ERROR_INVALID_DT_FTYPE_AUXVARNULL \
    "[writedefvar] Invalid declaration type DT_Ftype_auxvarnull for C_id=\"%s\" of Ctype=%d (Stype=\"%s\", Ftype=\"%s\") : not a pointer"
#define MSG_ERROR_UNIMPLEMENTED_DECLARATION \
    "[writedefvar] Unimplemented declaration type dt=%d for C_id=\"%s\" of Ctype=%d (Stype=\"%s\", Ftype=\"%s\")"
#define MSG_ERROR_USAGE_FLAG \
    "Invalid usage for C_id=\"%s\" of type \"%s\" : variable of a flag option must be a pointer to a scalar (such as char *)"
#define MSG_ERROR_USAGE_DEFAULT \
    "Invalid usage for C_id=\"%s\" of type \"%s\" : variable of an option with default value must be a pointer to a scalar (such as float *)"
#define MSG_ERROR_USAGE_IC \
    "Invalid usage for C_id=\"%s\" of type \"%s\" : variable of an option with interval checking must be a pointer to a scalar (such as float *)"
#define MSG_ERROR_USAGE_OPTION \
    "Invalid usage for C_id=\"%s\" of type \"%s\" : variable of an option must be a pointer to a scalar, or a megawave internal type"
#define MSG_ERROR_USAGE_INPUT \
    "Invalid usage for C_id=\"%s\" of type \"%s\" : variable of an input needed argument cannot be an explicit pointer"
#define MSG_ERROR_USAGE_OUTPUT \
    "Invalid usage for C_id=\"%s\" of type \"%s\" : variable of an output needed argument must be a pointer to a scalar, a megawave internal type, or the value of the return function"
#define MSG_ERROR_USAGE_RETURN \
    "Invalid usage for C_id=\"%s\" of type \"%s\" : type of the return main function cannot be an explicit pointer (such as float *)"
#define MSG_ERROR_VARIABLE_IC \
    "Invalid usage for C_id=\"%s\" of type \"%s\" : variable of variable arguments with interval checking must be a pointer to a scalar (such as float *)"
#define MSG_ERROR_VARIABLE_INPUT \
    "Invalid usage for C_id=\"%s\" of type \"%s\" : variable of input variable arguments must be a pointer to a scalar, or a megawave internal type"
#define MSG_ERROR_VARIABLE_OUTPUT \
    "Invalid usage for C_id=\"%s\" of type \"%s\" : variable of output variable arguments must be of megawave internal type"
#define MSG_ERROR_VARIABLE_OPTION_DEFAULT \
    "Invalid usage for C_id=\"%s\" of type \"%s\" : variable of an optional argument with default value must be a pointer to a scalar (such as float *)"
#define MSG_ERROR_VARIABLE_OPTION_IC \
    "Invalid usage for C_id=\"%s\" of type \"%s\" : variable of an optional argument with interval checking must be a pointer to a scalar (such as float *)"
#define MSG_ERROR_VARIABLE_OPTION \
    "Invalid usage for C_id=\"%s\" of type \"%s\" : variable of an optional argument must be a pointer to a scalar, or a megawave internal type"
#define MSG_ERROR_VARIABLE_UNUSED \
    "Invalid usage for C_id=\"%s\" of type \"%s\" : variable of a not used argument must be a pointer to a scalar, or a megawave internal type"
#define MSG_ERROR_ILLEGAL_ICTYPE \
    "Unexpected error for C_id=\"%s\" : illegal ICtype=%d"
#define MSG_ERROR_ILLEGAL_ICTYPE2 \
    "Unexpected error for C_id=\"%s\" : illegal ICtype=%d"
#define MSG_ERROR_UNEXPECTED \
    "Unexpected error for C_id=\"%s\" : option is not a FILE, SCALAR or FLAGOPT"
#define MSG_ERROR_UNEXPECTED2 \
    "Unexpected error for C_id=\"%s\" : needed argument is not a FILE nor a SCALAR"
#define MSG_ERROR_USAGE_COMPOSITE \
    "Invalid usage for C_id=\"%s\" : unsupported composite megawave type \"%s\""
#define MSG_ERROR_BUFFER_OVERFLOW \
     "[writeusage] Buffer overflow for usage (increase STRSIZE)"
#define MSG_ERROR_OPEN_FILE \
    "Cannot open A-file '%s' for writing"

#define CODE_INPUT_MW_OPTION "\
        if (!_search_filename(_mwoptarg))\n\
        {\n\
          char buffer[BUFSIZ];\n\
          sprintf(buffer, \"cannot find '%%s' in default path\", _mwoptarg);\n\
          mwicmd[mwind].mwuse(buffer);\n\
        }\n"
#define CODE_INPUT_MW_NEEDED "\
  /* input megawave needed argument H_id=%s */\n\
  if (_mwoptind+%d<argc)\n\
  {\n\
    if (!_search_filename(argv[_mwoptind+%d]))\n\
    {\n\
      char buffer[BUFSIZ];\n\
      sprintf(buffer, \"cannot find '%%s' in default path\", \\\n\
              argv[_mwoptind+%d]);\n\
      mwicmd[mwind].mwuse(buffer);\n\
    }\n\
  }\n\
  else\n\
    mwicmd[mwind].mwuse(\"missing '%s'\");\n"
#define CODE_OUTPUT_MW_NEEDED "\
  /* output megawave needed argument H_id=%s */\n\
  if (_mwoptind+%d>=argc)\n\
    mwicmd[mwind].mwuse(\"missing '%s'\");\n"
#define CODE_INPUT_MW_OPTION2 "\
    /* input megawave optional argument H_id=%s */\n\
    if (_mwoptind+%d<argc)\n\
    {\n\
      if (!_search_filename(argv[_mwoptind+%d]))\n\
      {\n\
        char buffer[BUFSIZ];\n\
        sprintf(buffer, \"cannot find '%%s' in default path\", \\\n\
                argv[_mwoptind+%d]);\n\
        mwicmd[mwind].mwuse(buffer);\n\
      }\n\
    }\n\
    else\n\
      mwicmd[mwind].mwuse(\"missing '%s'\");\n"
#define CODE_OUTPUT_MW_OPTION2 "\
    /* output megawave optional argument H_id=%s */\n\
    if (_mwoptind+%d>=argc)\n\
      mwicmd[mwind].mwuse(\"missing '%s'\");\n"
#define CODE_INPUT_SCALAR_NEEDED "\
  /* input scalar needed argument H_id=%s */\n\
  if (_mwoptind+%d>=argc)\n\
    mwicmd[mwind].mwuse(\"missing '%s'\");\n"
#define CODE_INPUT_SCALAR_OPTION "\
    /* input scalar optional argument H_id=%s */\n\
    if (_mwoptind+%d>=argc)\n\
      mwicmd[mwind].mwuse(\"missing '%s'\");\n"
#define CODE_INPUT_SCALAR_ARGUMENT "\
  /* input scalar needed argument with interval checking H_id=%s */\n\
  if (_mwoptind+%d<argc)\n\
  {\n\
    %s\n\
  }\n\
  else\n\
    mwicmd[mwind].mwuse(\"missing '%s'\");\n"
#define CODE_INPUT_SCALAR_OPT_ARGUMENT "\
    /* input scalar optional argument with interval checking H_id=%s */\n\
    if (_mwoptind+%d<argc)\n\
    {\n\
      %s\n\
    }\n\
    else\n\
      mwicmd[mwind].mwuse(\"missing '%s'\");\n"
#define CODE_INPUT_MW_VAR_ARGUMENT "\
  /* input megawave variable argument H_id=%s */\n\
    if (!_search_filename(argv[%si]))\n\
    {\n\
      char buffer[BUFSIZ];\n\
      sprintf(buffer, \"cannot find '%%s' in default path\",  \\\n\
              argv[%si]);\n\", MWPF);\n\
      mwicmd[mwind].mwuse(buffer);\n\
   }\n"
#define CODE_INPUT_SCAL_VAR_ARGUMENT "\
  /* input scalar variable argument with interval checking H_id=%s */\n\
  %s\n"
#define CODE_READ_INPUT "\
  /* Read input options */\n\
  _mwoptind = 1;\n\
  while ((%sc=_mwgetopt(argc, argv, \"%s\")) != -1)\n\
  {\n\
    switch (%sc) {\n"
#define CODE_READ_INPUT_OPTION "\
        _mwload_%s(_mwoptarg, %stype, %scomment, &%s);\n"
#define CODE_READ_INPUT_NEEDED_OPTION "\
  _mwload_%s(argv[_mwoptind + %d], %stype, %scomment, &%s);\n"
#define CODE_READ_INPUT_SCALAR_ARG "\
  %s = %s(argv[_mwoptind + %d]);\n"
#define CODE_READ_INPUT_OPT_ARG "\
    _mwload_%s(argv[_mwoptind + %d], %stype, %scomment, &%s);\n"
#define CODE_READ_INPUT_OPT_ARG2 "\
    %s%s = %s(argv[_mwoptind + %d]);\n"
#define CODE_READ_INPUT_VAR_ARG "\
  if (_mwoptind + %d >= argc)\n\
  {\n\
    %svar_flg = TRUE;\n\
  }\n\
  for (%si = _mwoptind + %d; %si < argc || %svar_flg == TRUE; %si++)\n\
  {\n\
    if (_mwoptind + %d < argc)\n\
    {\n"
#define CODE_READ_INPUT_VAR_ARG2 "\
  _mwload_%s(argv[%si], %stype, %scomment, &%s);\n"
#define CODE_WRITE_OUTPUT_VAR_ARG "\
        if (%s)\n\
        {\n\
          if (_mwsave_%s(argv[%si], %stype, type_force, %scomment, %s) < 0)\n\
            mwerror(FATAL, 1, \"cannot write %%s\\n\", argv[%si]);\n\
        }\n"
#define CODE_WRITE_OUTPUT_OPT "\
  /* Write output options */\n\
  _mwoptind = 1;\n\
  while ((%sc=_mwgetopt(argc, argv, \"%s\")) != -1)\n\
  {\n\
    switch (%sc)\n\
    {\n"
#define CODE_WRITE_OUTPUT_OPT2 "\
        if (%s)\n\
        {\n\
          if (_mwsave_%s(_mwoptarg, %stype, type_force, %scomment, %s) < 0)\n\
            mwerror(FATAL, 1, \"cannot write %%s\\n\", _mwoptarg) ;\n\
        }\n"
#define CODE_WRITE_OUTPUT_NEEDED "\
  if (_mwsave_%s(argv[_mwoptind+%d], \\\n\
                 %stype, type_force, %scomment, %s_ret) < 0)\n"
#define CODE_WRITE_OUTPUT_NEEDED2 "\
  if (_mwsave_%s(argv[_mwoptind+%d], %stype, type_force, %scomment, %s) < 0)\n"
#define CODE_WRITE_OUTPUT_NEEDED3 "\
    mwerror(FATAL, 1, \"cannot write %%s\\n\",  argv[_mwoptind+%d]) ;\n"
#define CODE_OUTPUT_MW_OPTION3 "\
        if (%s)\n\
        {\n\
          if (_mwsave_%s(argv[_mwoptind + %d], \\\n\
                         %stype, type_force, %scomment, %s) < 0)\n\
            mwerror(FATAL, 1, \"cannot write %%s\\n\", \\\n\
                    argv[_mwoptind + %d]) ;\n\
        }\n"
/*
 * prefix for
 * - megawave fixed internal names
 * - megawave user's internal names
 */
#define MWPF "_mw2f_"
#define MWPU "_mw2u_"

/* option list for _mwgetopt() */
static char optb[BUFSIZ];

/*
 * set in <fc> the function name that converts
 * any scalar written in a string to its value
 * in the type of the variable pointed by <a>.
 */

static void SetScalarConvFunction(t_argument * a, /*@out@*/ char * fc)
{
     switch(a->var->Ctype)
     {
     case CHAR_T  :
     case UCHAR_T :
     case SHORT_T :
     case USHORT_T:
     case INT_T   :
     case UINT_T  :
     case LONG_T  :
     case ULONG_T :
          strcpy(fc, "atol");
          break;
     case FLOAT_T :
     case DOUBLE_T:
          strcpy(fc, "atof");
          break;
     default:
          error(MSG_ERROR_CONVERSION, a->C_id, a->var->Ctype, a->var->Stype);
     }
}

/*
 * generate code to print numerical value in <s> assuming type in <a>.
 */
static void  print_value_scalar_arg(FILE * afile, char * s, t_argument * a)
{
     switch(a->var->Ctype)
     {
     case CHAR_T:
          fprintf(afile, "printf(\"%%c\", %s);\n", s);
          break;
     case UCHAR_T:
          fprintf(afile, "printf(\"%%d\", (int) %s);\n", s);
          break;
     case SHORT_T:
          fprintf(afile, "printf(\"%%hd\", %s);\n", s);
          break;
     case USHORT_T:
          fprintf(afile, "printf(\"%%hu\", %s);\n", s);
          break;
     case INT_T:
          fprintf(afile, "printf(\"%%d\", %s);\n", s);
          break;
     case UINT_T:
          fprintf(afile, "printf(\"%%u\", %s);\n", s);
          break;
     case LONG_T:
          fprintf(afile, "printf(\"%%ld\", %s);\n", s);
          break;
     case ULONG_T:
          fprintf(afile, "printf(\"%%lu\", %s);\n", s);
          break;
     case FLOAT_T:
     case DOUBLE_T:
          fprintf(afile, "printf(\"%%g\", %s);\n", s);
          break;
     default:
          error(MSG_ERROR_PRINT, a->C_id, a->var->Ctype, a->var->Stype);
     }
}


/*
 * write the header
 */
static void writeAheader(FILE * afile)
{
    fprintf(afile, "/**\n");
    fprintf(afile, " * Command-line parsing and wrapper,\n");
    fprintf(afile, " * generated by mwp for '%s'.\n", module_name);
    fprintf(afile, " */\n\n");
}

/*
 * write generic declarations
 */
/* TODO: strict inclusion policy */
/* TODO: cleanup, keep only the minimum */
static void writegendecl(FILE * afile)
{
    fprintf(afile, "/*\n");
    fprintf(afile, " * GENERIC DECLARATIONS\n");
    fprintf(afile, " */\n\n");

     /* include files */
     fprintf(afile, "#include <stdio.h>\n");

     /* stdlib.h for atoi() atol() atof() and string.h for strcpy() on Linux */
     fprintf(afile, "#include <stdlib.h>\n");
     fprintf(afile, "#include <string.h>\n");

     fprintf(afile, "#include \"mw.h\"\n");
     fprintf(afile, "#include \"mw-modules.h\"\n");
     fprintf(afile, "#include \"mw-cmdline.h\"\n\n");

     /* TODO : in mw.h */
     fprintf(afile, "#define TRUE  1\n");
     fprintf(afile, "#define FALSE 0\n\n");

     fprintf(afile, "int _%s(int argc, char * argv[]);\n", module_name);
     fprintf(afile, "void usage_%s(char *msg);\n\n", module_name);

     /* external variables */
     fprintf(afile, "extern char type_force[];\n");
     fprintf(afile, "extern char *_mwoptarg;\n");
     fprintf(afile, "extern int _mwoptind;\n");
     fprintf(afile, "extern char _mwoptlist[];\n");
     fprintf(afile, "extern char _mwdefoptbuf[];\n");
     fprintf(afile, "extern int help_flg;\n");
     fprintf(afile, "\n");

     fprintf(afile, "static int mwind = 0;\n");
     fprintf(afile, "static Mwiline mwicmd[] = { { "
             "\"%s\", _%s, usage_%s, \"%s\", \"%s\", \"%s\"} };\n",
             module_name, module_name, module_name,
             group_name, H->Function, usagebuf);
     fprintf(afile, "\n");

     fprintf(afile, "int _%s(int argc, char * argv[])\n", module_name);
     fprintf(afile, "{\n");
     fprintf(afile, "  int %sc; /* for  _mwgetopt()*/\n", MWPF);
     /* NOT SURE IT IS USEFUL */
     /* fprintf(afile, "  char *%sname;\n", MWPF); */
     fprintf(afile, "  char %stype[mw_ftype_size+1]; ", MWPF);
     fprintf(afile, "/* for _mwload_* functions */\n");
     fprintf(afile, "  char %scomment[BUFSIZ]; ", MWPF);
     fprintf(afile, "/* for _mwload_* functions */\n");

     if (H->NbVarArg > 0)
     {
          fprintf(afile, "  int %svar_flg;\n", MWPF);
          /* for argument loop in case of variable arguments */
          fprintf(afile, "  int %si;\n", MWPF);
     }

     fprintf(afile, "\n");
}

/*
 * write definition of a variable using the declaration type
 * given in dt (see DT_* constants).
 * Set the DeclType field of a->var, so we can remember
 * the declaration type.
 */
static void writedefvar(FILE * afile, t_argument * a, int dt)
{
     switch(dt)
     {
     case DT_Ftype_alone:
          if (!ISARG_IMPLICITPOINTER(a))
               fprintf(afile, "  %s %s=0;\n", a->var->Ftype, a->C_id);
          else
               fprintf(afile, "  %s %s=NULL;\n", a->var->Ftype, a->C_id);
          break;
     case DT_Ftype_auxvar:
          if (!ISARG_IMPLICITPOINTER(a))
               error(MSG_ERROR_INVALID_DT_FTYPE_AUXVAR, \
                     a->C_id, a->var->Ctype, a->var->Stype, a->var->Ftype);
          /* introduce a local variable of type Stype with the default value */
          fprintf(afile, "  %s %s%s= %s;\n", \
                  a->var->Stype, MWPU, a->C_id, a->Val);
          /*
           * set pointer to the address of the local variable
           * having the default value
           */
          fprintf(afile, "  %s %s= & %s%s;\n", \
                  a->var->Ftype, a->C_id, MWPU, a->C_id);
          break;
     case DT_Ftype_auxvarnull:
          if (!ISARG_IMPLICITPOINTER(a))
               error(MSG_ERROR_INVALID_DT_FTYPE_AUXVARNULL, \
                     a->C_id, a->var->Ctype, a->var->Stype, a->var->Ftype);
          /* introduce a local variable with default value 0 */
          fprintf(afile, "  %s %s%s= 0;\n", a->var->Stype, MWPU, a->C_id);
          /*
           * set pointer to NULL.
           * This pointer would be set to the address
           * of the local variable only if the option is selected
           */
          fprintf(afile, "  %s %s= NULL;\n", a->var->Ftype, a->C_id);
          break;
     case DT_Ftype_ret:
          fprintf(afile, "  %s %s_ret;\n", a->var->Ftype, a->C_id);
          break;
     case DT_Stype:
          if (ISARG_SCALAR(a))
               fprintf(afile, "  %s %s=0;\n", a->var->Stype, a->C_id);
          else
               fprintf(afile, "  %s %s=NULL;\n", a->var->Stype, a->C_id);
          break;
     default:
          error(MSG_ERROR_UNIMPLEMENTED_DECLARATION, \
                dt, a->C_id, a->var->Ctype, a->var->Stype, a->var->Ftype);
     }
     a->var->DeclType = dt;
}


/*
 * write argument declarations
 */
static void writeargdecl(FILE * afile)
{
     t_argument * a;
     /* to fill optb[] */
     char * popt = NULL;

     fprintf(afile, "/*\n");
     fprintf(afile, " * ARGUMENTS DECLARATION\n");
     fprintf(afile, " */\n\n");

     /* variables for options */
     if (H->NbOption > 0)
     {
          fprintf(afile, "  /* Variables for options */\n");
          popt = optb;
          for (a=H->usage; a; a=a->next)
          {
               if (ISARG_OPTION(a))
               {
                    if (ISARG_FLAGOPT(a) && (!ISARG_POINTERSCALAR(a)))
                         error(MSG_ERROR_USAGE_FLAG, a->C_id, a->var->Ftype);
                    if (ISARG_DEFAULT(a) && (!ISARG_POINTERSCALAR(a)))
                         error(MSG_ERROR_USAGE_DEFAULT, a->C_id, a->var->Ftype);
                    if (ISARG_INTERVAL(a) && (!ISARG_POINTERSCALAR(a)))
                         error(MSG_ERROR_USAGE_IC, a->C_id, a->var->Ftype);
                    if (!ISARG_IMPLICITPOINTER(a))
                         error(MSG_ERROR_USAGE_OPTION, a->C_id, a->var->Ftype);

                    if (ISARG_DEFAULT(a))
                         /* option with default value */
                         writedefvar(afile, a, DT_Ftype_auxvar);
                    else if (ISARG_SCALAR(a))
                         /* scalar option */
                         writedefvar(afile, a, DT_Ftype_auxvarnull);
                         /*
                          * when there is no default value,
                          * initialize pointer to NULL
                          */
                    else if (ISARG_POINTERFILE(a))
                         /*
                          * case of e.g. Cimage * :
                          * define variable as Cimage
                          * rather than Cimage *
                          */
                         writedefvar(afile, a, DT_Stype);
                    else
                         writedefvar(afile, a, DT_Ftype_alone);

                    /* fill optb[] */
                    * popt++ = a->Flag;
                    if  (!ISARG_FLAGOPT(a))
                         * popt++ = ':';
               }
          }
     }
     if (popt)
          * popt = '\0';

     /* variables for needed arguments */
     if (H-> NbNeededArg > 0)
     {
          fprintf(afile, "  /* Variables for needed arguments */\n");
          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_NEEDED(a))
               {
                    if (ISARG_INPUT(a))
                    {
                         if (ISARG_EXPLICITPOINTER(a))
                              error(MSG_ERROR_USAGE_INPUT, \
                                    a->C_id, a->var->Ftype);
                         writedefvar(afile, a, DT_Stype);
                    }
                    else
                    {
                         /* output argument */
                         if (!ISARG_IMPLICITPOINTER(a) && !ISARG_RETURNFUNC(a))
                              error(MSG_ERROR_USAGE_OUTPUT, \
                                    a->C_id, a->var->Ftype);

                         if (ISARG_RETURNFUNC(a))
                         {
                              /* the output argument is the return function */
                              if (ISARG_EXPLICITPOINTER(a))
                                   error(MSG_ERROR_USAGE_RETURN, \
                                         a->C_id, a->var->Ftype);
                              writedefvar(afile, a, DT_Ftype_ret);
                         }
                         else if (ISARG_EXPLICITPOINTER(a))
                              writedefvar(afile, a, DT_Stype);
                         else
                              writedefvar(afile, a, DT_Ftype_alone);
                    }
               }
          }
     }


     /* variables for variable arguments  */
     if (H-> NbVarArg > 0)
     {
          fprintf(afile, "  /* Variables for variable arguments */\n");
          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_VARIABLE(a))
               {
                    if (ISARG_INTERVAL(a) && (!ISARG_POINTERSCALAR(a)))
                         error(MSG_ERROR_VARIABLE_IC, a->C_id, a->var->Ftype);
                    if (ISARG_INPUT(a))
                    {
                         if (!ISARG_IMPLICITPOINTER(a))
                              error(MSG_ERROR_VARIABLE_INPUT, \
                                    a->C_id, a->var->Ftype);
                    }
                    else if (!ISARG_FILE(a))
                         error(MSG_ERROR_VARIABLE_OUTPUT,       \
                               a->C_id, a->var->Ftype);
                    if (ISARG_INPUT(a))
                         writedefvar(afile, a, DT_Ftype_alone);
                    else if (ISARG_EXPLICITPOINTER(a))
                         writedefvar(afile, a, DT_Stype);
                    else
                         writedefvar(afile, a, DT_Ftype_alone);
               }
          }
     }

     /* variables for optionals arguments  */
     if (H->NbOptionArg > 0)
     {
          fprintf(afile, "  /* Variables for optional arguments */\n");
          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_OPTARG(a))
               {
                    if (ISARG_DEFAULT(a) && (!ISARG_POINTERSCALAR(a)))
                         error(MSG_ERROR_VARIABLE_OPTION_DEFAULT, \
                               a->C_id, a->var->Ftype);
                    if (ISARG_INTERVAL(a) && (!ISARG_POINTERSCALAR(a)))
                         error(MSG_ERROR_VARIABLE_OPTION_IC, \
                               a->C_id, a->var->Ftype);
                    if (!ISARG_IMPLICITPOINTER(a))
                         error(MSG_ERROR_VARIABLE_OPTION, \
                               a->C_id, a->var->Ftype);
                    if (ISARG_DEFAULT(a))
                         /* optional arg with default value */
                         writedefvar(afile, a, DT_Ftype_auxvar);
                    else if (ISARG_SCALAR(a))
                         /* scalar optional arg */
                         writedefvar(afile, a, DT_Ftype_auxvarnull);
                    else
                         /*
                          * when there is no default value,
                          * initialize pointer to NULL
                          */
                         writedefvar(afile, a, DT_Ftype_alone);
               }
          }
     }

     /* variables for not used arguments  */
     if (H->NbNotUsedArg > 0)
     {
          fprintf(afile, "  /* Variables for not used arguments */\n");
          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_NOTUSED(a))
               {
                    if (!ISARG_IMPLICITPOINTER(a))
                         error(MSG_ERROR_VARIABLE_UNUSED, \
                               a->C_id, a->var->Ftype);
                    writedefvar(afile, a, DT_Ftype_alone);
               }
          }
     }
     fprintf(afile, "\n");
}

/*
 * print in the A-file the interval and the default
 * value of the argument's value, if applicable.
 */
static void  print_interval_and_defval(FILE * afile, t_argument * a)
{
     char A = ' ', B = ' ';

     if (!(ISARG_INTERVAL(a) || ISARG_DEFAULT(a))) return;

     fprintf(afile, " (");
     if (ISARG_INTERVAL(a))
     {
          switch (a->ICtype)
          {
          case CLOSED:
               A='[';
               B=']';
               break;
          case MAX_EXCLUDED:
               A='[';
               B='[';
               break;
          case MIN_EXCLUDED:
               A=']';
               B=']';
               break;
          case OPEN:
               A=']';
               B='[';
               break;
          default:
               /* unknown interval type */
               error(MSG_ERROR_ILLEGAL_ICTYPE, a->C_id, a->ICtype);
          }
          fprintf(afile, "in %c%s, %s%c", A, a->Min, a->Max, B);
          if (ISARG_DEFAULT(a))
               fprintf(afile, ", ");
     }
     if (ISARG_DEFAULT(a))
          fprintf(afile, "default %s", a->Val);
     fprintf(afile, ")");
}


/*
 * generate code to check that input numerical value in <s>
 * is in the range defined by the interval in <a>.
 */

static void  print_check_interval_arg(FILE * afile, t_argument * a, char * s)
{
     char t_m[3] = "";
     char t_M[3] = "";
     char fc[256] = "";
     char A = ' ', B = ' ';

     switch (a->ICtype)
     {
     case CLOSED:
          strcpy(t_m, "<");
          strcpy(t_M, "<");
          A='[';
          B=']';
          break;
     case MAX_EXCLUDED:
          strcpy(t_m, "<");
          strcpy(t_M, "<=");
          A='[';
          B='[';
          break;
     case MIN_EXCLUDED:
          strcpy(t_m, "<=");
          strcpy(t_M, "<");
          A=']';
          B=']';
          break;
     case OPEN:
          strcpy(t_m, "<=");
          strcpy(t_M, "<=");
          A=']';
          B='[';
          break;
     default:
          /* unknown interval type */
          error(MSG_ERROR_ILLEGAL_ICTYPE2, a->C_id, a->ICtype);
     }

     SetScalarConvFunction(a, fc);

     fprintf(afile, "        if ((%s(%s) %s (%s) %s) || ((%s) %s %s %s(%s)))\n", \
             fc, s, t_m, a->var->Stype, a->Min, \
             a->var->Stype, a->Max, t_M, fc, s);
     fprintf(afile, "{\n");
     if (a->IOtype == READ)
     {
          fprintf(afile, "          char buffer[BUFSIZ];\n");
          fprintf(afile, "          sprintf(buffer, \"input data %%s ");
          fprintf(afile, "converted to type %s is out of %c%s, %s%c\", %s);\n", \
                  a->var->Stype, A, a->Min, a->Max, B, s);
          fprintf(afile, "          mwicmd[mwind].mwuse(buffer);\n");
     }
     else
     {
          fprintf(afile, "          printf(\"output data %s ", s);
          fprintf(afile, "converted to type %s is out of %c%s, %s%c\");\n", \
                  a->var->Stype, A, a->Min, a->Max, B);
     }
     fprintf(afile, "        }\n");
}


static char * str_chkint(/*@ out @*/char * str, t_argument * a, char * s)
{
     char t_m[3] = "";
     char t_M[3] = "";
     char fc[256] = "";
     char tmp[STRSIZE];
     char A = ' ', B = ' ';

     switch (a->ICtype)
     {
     case CLOSED:
          strcpy(t_m, "<");
          strcpy(t_M, "<");
          A='[';
          B=']';
          break;
     case MAX_EXCLUDED:
          strcpy(t_m, "<");
          strcpy(t_M, "<=");
          A='[';
          B='[';
          break;
     case MIN_EXCLUDED:
          strcpy(t_m, "<=");
          strcpy(t_M, "<");
          A=']';
          B=']';
          break;
     case OPEN:
          strcpy(t_m, "<=");
          strcpy(t_M, "<=");
          A=']';
          B='[';
          break;
     default:
          /* unknown interval type */
          error(MSG_ERROR_ILLEGAL_ICTYPE2, a->C_id, a->ICtype);
     }

     SetScalarConvFunction(a, fc);

     sprintf(str,                                                       \
             "        if ((%s(%s) %s (%s) %s) || ((%s) %s %s %s(%s)))\n", \
             fc, s, t_m, a->var->Stype, a->Min,                         \
             a->var->Stype, a->Max, t_M, fc, s);
     strcat(str, "{\n");
     if (a->IOtype == READ)
     {
          strcat(str, "          char buffer[BUFSIZ];\n");
          strcat(str, "          sprintf(buffer, \"input data %s ");
          sprintf(tmp,                                                  \
                  "converted to type %s is out of %c%s, %s%c\", %s);\n", \
                  a->var->Stype, A, a->Min, a->Max, B, s);
          strcat(str, tmp);
          strcat(str, "          mwicmd[mwind].mwuse(buffer);\n");
     }
     else
     {
          sprintf(tmp,                                          \
                  "          printf(\"output data %s ", s);
          strcat(str, tmp);
          sprintf(tmp,                                                  \
                  "converted to type %s is out of %c%s, %s%c\");\n",    \
                  a->var->Stype, A, a->Min, a->Max, B);
          strcat(str, tmp);
     }
     strcat(str, "        }\n");
     return str;
}


/*
 * generate code to check that input files are readable, output
 * files are writable and numerical values are in range
 */

static void print_check_io_arg(FILE * afile)
{
     t_argument * a;
     int n;

     fprintf(afile, "\n/* ~~~ [print_check_io_arg] ~~~*/\n\n");

     /* check Options */
     if (H->NbOption>0)
     {
          fprintf(afile, "  /* Check io options */\n");
          fprintf(afile, "  _mwoptind = 1;\n");
          fprintf(afile, "  while ((%sc=_mwgetopt(argc, argv, \"%s\")) != -1)\n", \
                  MWPF, optb);
          fprintf(afile, "{\n");
          fprintf(afile, "    switch (%sc) {\n", MWPF);

          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_OPTION(a))
               {
                    fprintf(afile, "      case '%c' :\n", a->Flag);
                    if (ISARG_FILE(a))
                    {
                         if (ISARG_INPUT(a))
                              fprintf(afile, CODE_INPUT_MW_OPTION);
                    }
                    else if (ISARG_SCALARNOTFLAG(a))
                    {
                         /* scalar option */
                         if  (ISARG_INTERVAL(a))
                              print_check_interval_arg(afile, a, "_mwoptarg");
                    }
                    else if (!ISARG_FLAGOPT(a))
                         error(MSG_ERROR_UNEXPECTED, a->C_id);

                    fprintf(afile, "        break;\n");
               }
          }
          /* default : unknown option calls usage function */
          fprintf(afile, "      case '?' :\n");
          fprintf(afile, "        mwicmd[mwind].mwuse(NULL);\n");
          fprintf(afile, "        break;\n");
          fprintf(afile, "      default :\n");
          fprintf(afile, "        break;\n");
          fprintf(afile, "    }\n");
          fprintf(afile, "  }\n\n");
     }

     n = 0;
     /* check needed arguments */
     if (H->NbNeededArg>0)
     {
          fprintf(afile, "  /* Check io needed arguments */\n");
          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_NEEDED(a))
               {
                    if (ISARG_FILE(a))
                    {
                         if (ISARG_INPUT(a))
                         {
                              fprintf(afile, CODE_INPUT_MW_NEEDED, \
                                      a->H_id, n, n, n, a->H_id);
                              n++;
                         }
                         else
                         {
                              fprintf(afile, CODE_OUTPUT_MW_NEEDED, \
                                      a->H_id, n, a->H_id);
                              n++;
                         }
                    }
                    else if (ISARG_SCALAR(a))
                    {
                         /* scalar argument */
                         if (ISARG_INPUT(a))
                         {
                              /* input scalar argument */
                              if (ISARG_INTERVAL(a))
                              {
                                   char buffer[BUFSIZ];
                                   char tmp[STRSIZE] = "";
                                   sprintf(buffer,                      \
                                            "argv[_mwoptind+%d]", n);
                                   fprintf(afile, CODE_INPUT_SCALAR_ARGUMENT, \
                                           a->H_id, n, \
                                           str_chkint(tmp, a, buffer), \
                                           a->H_id);
                              }
                              else
                              {
                                   fprintf(afile, CODE_INPUT_SCALAR_NEEDED, \
                                           a->H_id, n, a->H_id);
                              }
                              n++;
                         }
                    }
               }
          }
     }


     /* check optional arguments */
     if (H->NbOptionArg>0)
     {
          fprintf(afile, "  /* Check io optional arguments */\n");
          fprintf(afile, "  if (_mwoptind+%d < argc) {\n", n);
          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_OPTARG(a))
               {
                    if (ISARG_FILE(a))
                    {
                         if (ISARG_INPUT(a))
                         {
                              fprintf(afile, CODE_INPUT_MW_OPTION2, \
                                      a->H_id, n, n, n, a->H_id);
                              n++;
                         }
                         else
                         {
                              fprintf(afile, CODE_OUTPUT_MW_OPTION2, \
                                      a->H_id, n, a->H_id);
                              n++;
                         }
                    }
                    else if (ISARG_SCALAR(a))
                    {
                         /* scalar optional argument */
                         if (ISARG_INPUT(a))
                         {
                              /* input scalar argument */
                              if (ISARG_INTERVAL(a))
                              {
                                   char buffer[BUFSIZ];
                                   char tmp[STRSIZE] = "";
                                   sprintf(buffer,                      \
                                           "argv[_mwoptind+%d]", n);
                                   fprintf(afile, CODE_INPUT_SCALAR_OPT_ARGUMENT, \
                                           a->H_id, n,                  \
                                           str_chkint(tmp, a, buffer),  \
                                           a->H_id);
                              }
                              else
                              {
                                   fprintf(afile, CODE_INPUT_SCALAR_OPTION, \
                                           a->H_id, n, a->H_id);
                              }
                              n++;
                         }
                    }
               }
          }
          fprintf(afile, "  }\n");
     }


     /* check variable arguments */
     if (H->NbVarArg>0)
     {
          fprintf(afile, "  /* Check io variable arguments */\n");
          fprintf(afile, "  if (_mwoptind+%d < argc) {\n", n);
          fprintf(afile, "    for (%si = _mwoptind+%d; %si<argc; %si++) {\n", \
                  MWPF, n, MWPF, MWPF);
          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_VARIABLE(a))
               {
                    if (ISARG_FILE(a))
                    {
                         if (ISARG_INPUT(a))
                              /* input megawave variable argument */
                              fprintf(afile, CODE_INPUT_MW_VAR_ARGUMENT, \
                                      a->H_id, MWPF, MWPF);
                    }
                    else if (ISARG_SCALAR(a))
                    {
                         /* scalar variable argument */
                         if (ISARG_INPUT(a))
                         {
                              /* input scalar variable argument */
                              if (ISARG_INTERVAL(a))
                              {
                                   char buffer[BUFSIZ];
                                   char tmp[STRSIZE] = "";
                                   sprintf(buffer,              \
                                           "argv[%si]", MWPF);
                                   fprintf(afile, CODE_INPUT_SCAL_VAR_ARGUMENT, \
                                           a->H_id, str_chkint(tmp, a, buffer));
                              }
                         }

                    }
                    fprintf(afile, "    }\n");
                    fprintf(afile, "  }\n");
               }
          }
     }

     fprintf(afile, "\n/* ~~~ end of [print_check_io_arg] ~~~*/\n\n");
}


/*
 * generate code to read and set input variables.
 */
static void  print_read_input(FILE * afile)
{
     t_argument * a;
     char buffer[BUFSIZ];
     int n;

     fprintf(afile, "\n/* ~~~ [print_read_input] ~~~*/\n\n");

     /* read input options */
     if (H->NbOption>0)
     {
          fprintf(afile, CODE_READ_INPUT, MWPF, optb, MWPF);

          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_OPTION(a))
               {
                    fprintf(afile, "      case '%c' :\n", a->Flag);
                    if (ISARG_FILE(a))
                    {
                         strcpy(buffer, a->var->Stype);
                         if (lowerstring(buffer) > 1)
                              error(MSG_ERROR_USAGE_COMPOSITE, \
                                    a->C_id, a->var->Stype);
                         if (ISARG_INPUT(a))
                              /* input megawave option */
                              fprintf(afile, CODE_READ_INPUT_OPTION, \
                                      buffer, MWPF, MWPF, a->C_id);
                         else
                              /* output megawave option */
                              fprintf(afile, "        %s = mw_new_%s();\n", \
                                      a->C_id, buffer);
                    }
                    else if (ISARG_SCALARNOTFLAG(a))
                    {
                         /* scalar option */
                         if (ISARG_INPUT(a))
                         {
                              /* input scalar option */
                              SetScalarConvFunction(a, buffer);
                              fprintf(afile, "        %s%s = %s(_mwoptarg);\n", \
                                      MWPU, a->C_id, buffer);
                              fprintf(afile, "        %s = &%s%s;\n", \
                                      a->C_id, MWPU, a->C_id);
                         }
                         else
                              /* output scalar option */
                              fprintf(afile, "        %s = &%s%s;\n", \
                                      a->C_id, MWPU, a->C_id);
                    }
                    else if (ISARG_FLAGOPT(a))
                    {
                         /* flag option */
                         fprintf(afile, "        %s%s = TRUE;\n", MWPU, a->C_id);
                         fprintf(afile, "        %s = &%s%s;\n", \
                                 a->C_id, MWPU, a->C_id);
                    }
                    else
                         error(MSG_ERROR_UNEXPECTED);


                    fprintf(afile, "        strcat(_mwoptlist, \"%c\");\n", \
                            a->Flag);
                    fprintf(afile, "        break;\n");

               }
          }
          /* default : unknown option calls usage function */
          fprintf(afile, "      default :\n");
          fprintf(afile, "        break;\n");
          fprintf(afile, "    }\n");
          fprintf(afile, "  }\n\n");
     }

     n = 0;
     /* read needed arguments */
     if (H->NbNeededArg>0)
     {
          fprintf(afile, "  /* Read needed arguments */\n");
          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_NEEDED(a))
               {
                    if (!ISARG_RETURNFUNC(a))
                    {
                         if (ISARG_FILE(a))
                         {
                              strcpy(buffer, a->var->Stype);
                              if (lowerstring(buffer)>1)
                                   error(MSG_ERROR_USAGE_COMPOSITE, \
                                         a->C_id, a->var->Stype);
                              if (ISARG_INPUT(a))
                                   /* input megawave needed argument */
                                   fprintf(afile, CODE_READ_INPUT_NEEDED_OPTION, \
                                           buffer, n, MWPF, MWPF, a->C_id);
                              else
                                   /* output megawave needed argument */
                                   fprintf(afile, "  %s = mw_new_%s();\n", \
                                           a->C_id, buffer);
                              n++;
                         }
                         else if (ISARG_SCALAR(a))
                              /* scalar needed argument */
                         {
                              if (ISARG_INPUT(a))
                                   /* input scalar argument */
                              {
                                   SetScalarConvFunction(a, buffer);
                                   fprintf(afile, CODE_READ_INPUT_SCALAR_ARG, \
                                           a->C_id, buffer, n);
                                   n++;
                              }
                              /* output scalar argument : nothing to be done */
                         }
                         else
                              error(MSG_ERROR_UNEXPECTED2, a->C_id);
                    }
                    else
                         n++;
               }
          }
     }

     /* read optional arguments */
     if (H->NbOptionArg>0)
     {
          fprintf(afile, "  /* Read optional arguments */\n");
          fprintf(afile, "  if (_mwoptind+%d < argc) {\n", n);
          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_OPTARG(a))
               {
                    if (ISARG_FILE(a))
                    {
                         strcpy(buffer, a->var->Stype);
                         if (lowerstring(buffer)>1)
                              error(MSG_ERROR_USAGE_COMPOSITE, \
                                    a->C_id, a->var->Stype);
                         if (ISARG_INPUT(a))
                         {
                              /* input megawave optional argument */
                              fprintf(afile, CODE_READ_INPUT_OPT_ARG, \
                                      buffer, n, MWPF, MWPF, a->C_id);
                              n++;
                         }
                         else
                              /* output megawave optional argument */
                              fprintf(afile, "    %s = mw_new_%s();\n",\
                                      a->C_id, buffer);
                    }
                    else if (ISARG_SCALAR(a))
                    {
                         /* scalar optional argument */
                         if (ISARG_INPUT(a))
                         {
                              /* input optional argument */
                              SetScalarConvFunction(a, buffer);
                              fprintf(afile, CODE_READ_INPUT_OPT_ARG2, \
                                      MWPU, a->C_id, buffer, n);
                              fprintf(afile, "    %s = &%s%s;\n", \
                                      a->C_id, MWPU, a->C_id);
                              n++;
                         }
                         else
                              /* output optional argument */
                              fprintf(afile, "    %s = &%s%s;\n", \
                                      a->C_id, MWPU, a->C_id);
                    }
                    else
                         error(MSG_ERROR_UNEXPECTED2, a->C_id);
               }
          }
          fprintf(afile, "  }\n");
     }

     /* read variable arguments */
     if (H->NbVarArg > 0)
     {
          fprintf(afile, "  /* Read variable arguments */\n");
          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_VARIABLE(a))
               {
                    fprintf(afile, CODE_READ_INPUT_VAR_ARG, \
                            n, MWPF, MWPF, n, MWPF, MWPF, MWPF, n);

                    if (ISARG_FILE(a))
                    {
                         strcpy(buffer, a->var->Stype);
                         if (lowerstring(buffer)>1)
                              error(MSG_ERROR_USAGE_COMPOSITE, \
                                    a->C_id, a->var->Stype);
                         if (ISARG_INPUT(a))
                              /* input megawave variable argument */
                              fprintf(afile, CODE_READ_INPUT_VAR_ARG2, \
                                      buffer, MWPF, MWPF, MWPF, a->C_id);
                         else
                              /* output megawave variable argument */
                              fprintf(afile, "  %s = mw_new_%s();\n", \
                                      a->C_id, buffer);
                    }
                    else if (ISARG_SCALAR(a))
                    {
                         /* scalar variable argument */
                         if (ISARG_INPUT(a))
                         {
                              /* input scalar variable argument */
                              SetScalarConvFunction(a, buffer);
                              fprintf(afile, "        %s%s = %s(argv[%si]);\n", \
                                      MWPU, a->C_id, buffer, MWPF);
                              fprintf(afile, "        %s = &%s%s;\n", \
                                      a->C_id, MWPU, a->C_id);
                         }
                         else
                              /* output scalar variable argument */
                              fprintf(afile, "        %s = &%s%s;\n", \
                                      a->C_id, MWPU, a->C_id);
                    }
                    else
                         error(MSG_ERROR_UNEXPECTED2, a->C_id);

                    fprintf(afile, " }\n");
                    fprintf(afile, " else {\n");
                    fprintf(afile, "  %s = NULL; \n", a->C_id);
                    fprintf(afile, " }\n");

               } /* end of if (ISARG_VARIABLE(a)) */
          }
     } /* end of if (H->NbVarArg>0) */

     fprintf(afile, "\n/* ~~~ end of [print_read_input] ~~~*/\n\n");
}


/*
 * generate code to write output variables.
 */
static void  print_write_output(FILE * afile)
{
     t_argument * a;
     char buffer[BUFSIZ];
     int n;

     fprintf(afile, "\n/* ~~~ [print_write_output] ~~~*/\n\n");

     /* write variable arguments */
     if (H->NbVarArg > 0)
     {
          fprintf(afile, "  /* Write variable arguments */\n");
          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_VARIABLE(a))
               {
                    if (ISARG_FILE(a))
                    {
                         strcpy(buffer, a->var->Stype);
                         if (lowerstring(buffer) > 1)
                              error(MSG_ERROR_USAGE_COMPOSITE, \
                                    a->C_id, a->var->Stype);
                         if (ISARG_OUTPUT(a))
                              /* output megawave variable argument */
                              fprintf(afile, CODE_WRITE_OUTPUT_VAR_ARG, \
                                      a->C_id, buffer, \
                                      MWPF, MWPF, MWPF, a->C_id, MWPF);
                         /*
                          * input megawave variable argument :
                          * nothing to be done
                          */
                    }
                    else if (ISARG_SCALAR(a))
                    {
                         /* scalar variable argument */
                         if (ISARG_OUTPUT(a))
                         {
                              /* output scalar variable argument */
                              /*
                               * output with interval checking
                               * NOT IMPLEMENTED yet
                               * if (ISARG_INTERVAL(a))
                               * print_check_interval_arg(afile, a, "_mwoptarg");
                               */
                              /* generate print of variable */
                              fprintf(afile, "        printf(\"%s = \");\n", \
                                      a->H_id);
                              fprintf(afile, "        ");
                              sprintf(buffer,                   \
                                      "%s%s", MWPU, a->C_id);
                              print_value_scalar_arg(afile, buffer, a);
                              fprintf(afile, "        printf(\"\\n\");\n");
                         }
                    }
                    fprintf(afile, "    %svar_flg = FALSE;\n", MWPF);
                    fprintf(afile, "  }\n");
               } /* end of if (ISARG_VARIABLE(a)) */
          }
     }/* end of if (H->NbVarArg>0) */

     /* write output options */
     if (H->NbOption>0)
     {
          fprintf(afile, CODE_WRITE_OUTPUT_OPT, MWPF, optb, MWPF);

          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_OPTION(a))
               {
                    fprintf(afile, "      case '%c' :\n", a->Flag);
                    if (ISARG_FILE(a))
                    {
                         strcpy(buffer, a->var->Stype);
                         if (lowerstring(buffer)>1)
                              error(MSG_ERROR_USAGE_COMPOSITE, \
                                    a->C_id, a->var->Stype);
                         if (ISARG_OUTPUT(a))
                              /* output megawave option */
                              fprintf(afile, CODE_WRITE_OUTPUT_OPT2, \
                                      a->C_id, buffer, MWPF, MWPF, a->C_id);
                    }
                    else if (ISARG_SCALARNOTFLAG(a))
                    {
                         /* scalar option */
                         if (ISARG_OUTPUT(a))
                         {
                              /* output scalar option */
                              /*
                               * output with interval checking
                               * NOT IMPLEMENTED yet
                               * if (ISARG_INTERVAL(a))
                               * print_check_interval_arg(afile, a, "_mwoptarg");
                               */
                              /* generate print of variable */
                              fprintf(afile, "        printf(\"%s = \");\n", \
                                      a->H_id);
                              fprintf(afile, "        ");
                              sprintf(buffer,                   \
                                      "%s%s", MWPU, a->C_id);
                              print_value_scalar_arg(afile, buffer, a);
                              fprintf(afile, "        printf(\"\\n\");\n");
                         }
                    }

                    fprintf(afile, "        break;\n");
               }
          }
          /* default : unknown option calls usage function */
          fprintf(afile, "      default :\n");
          fprintf(afile, "        break;\n");
          fprintf(afile, "    }\n");
          fprintf(afile, "  }\n\n");
     }

     n=0;
     /* write needed arguments */
     if (H->NbNeededArg>0)
     {
          fprintf(afile, "  /* Write needed arguments */\n");
          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_NEEDED(a))
               {
                    if (ISARG_FILE(a))
                    {
                         if (ISARG_INPUT(a))
                              n++;
                         else
                         {
                              /* output megawave needed argument */
                              strcpy(buffer, a->var->Stype);
                              if (lowerstring(buffer)>1)
                                   error(MSG_ERROR_USAGE_COMPOSITE, \
                                         a->C_id, a->var->Stype);

                              if (ISARG_RETURNFUNC(a))
                                   fprintf(afile, CODE_WRITE_OUTPUT_NEEDED, \
                                           buffer, n, MWPF, MWPF, a->C_id);
                              else
                                   fprintf(afile, CODE_WRITE_OUTPUT_NEEDED2, \
                                           buffer, n, MWPF, MWPF, a->C_id);
                              fprintf(afile, CODE_WRITE_OUTPUT_NEEDED3, n);
                              n++;
                         }
                    }
                    else if (ISARG_SCALAR(a))
                    {
                         /* scalar needed argument */
                         if (ISARG_INPUT(a))
                              n++;
                         else
                         {
                              /* output scalar argument */
                              if (ISARG_RETURNFUNC(a))
                                   sprintf(buffer,              \
                                            "%s_ret", a->C_id);
                              else
                                   sprintf(buffer,              \
                                            "%s", a->C_id);

                              /*
                               * output with interval checking
                               * NOT IMPLEMENTED yet
                               * if (ISARG_INTERVAL(a))
                               * print_check_interval_arg(afile, a, "_mwoptarg");
                               */
                              /* generate print of variable */
                              fprintf(afile, "  printf(\"%s = \");\n", a->H_id);
                              fprintf(afile, "  ");
                              print_value_scalar_arg(afile, buffer, a);
                              fprintf(afile, "  printf(\"\\n\");\n");
                         }
                    }
                    else
                         error(MSG_ERROR_UNEXPECTED2, a->C_id);
               }
          }
     }

     /* write optional arguments */
     if (H->NbOptionArg>0)
     {
          fprintf(afile, "  /* Write optional arguments */\n");
          fprintf(afile, "  if (_mwoptind+%d < argc) {\n", n);
          for (a = H->usage; a; a = a->next)
          {
               if (ISARG_OPTARG(a))
               {
                    if (ISARG_FILE(a))
                    {
                         strcpy(buffer, a->var->Stype);
                         if (lowerstring(buffer)>1)
                              error(MSG_ERROR_USAGE_COMPOSITE, \
                                    a->C_id, a->var->Stype);
                         if (ISARG_OUTPUT(a))
                              /* output megawave optional argument */
                              fprintf(afile, CODE_OUTPUT_MW_OPTION3, \
                                      a->C_id, buffer, n, MWPF, MWPF, \
                                      a->C_id, n);
                         n++;
                    }
                    else if (ISARG_SCALAR(a))
                    {
                         /* scalar optional argument */
                         if (ISARG_OUTPUT(a))
                         {
                              /* output scalar optional argument */
                              /*
                               * output with interval checking
                               * NOT IMPLEMENTED yet
                               * if (ISARG_INTERVAL(a))
                               * print_check_interval_arg(afile, a, "_mwoptarg");
                               */
                              /* generate print of variable */
                              fprintf(afile, "        printf(\"%s = \");\n", \
                                      a->H_id);
                              fprintf(afile, "        ");
                              sprintf(buffer,                   \
                                       "%s%s", MWPU, a->C_id);
                              print_value_scalar_arg(afile, buffer, a);
                              fprintf(afile, "        printf(\"\\n\");\n");
                         }
                         else
                              n++;
                    }
               }
          }
          fprintf(afile, "  }\n");
     }

     fprintf(afile, "\n/* ~~~ end of [print_write_output] ~~~*/\n\n");
}

/*
 *
 * Generate code to read and set input variables.
 */

static void  print_main_function_call(FILE * afile)
{
     t_argument * a;
     t_variable * p;
     char Address;

     fprintf(afile, "\n/* ~~~ [print_main_function_call] ~~~*/\n\n");

     if (H->retmod != NULL)
          /*
           * the module uses the return function
           * via the argument pointed by H->retmod
           */
          fprintf(afile, "  %s_ret =  (%s) %s (", \
                  H->retmod->C_id, H->retmod->var->Ftype, H->Name);
     else
          /* rhe module does not use the return function */
          fprintf(afile, "  %s (", H->Name);

     for (p = C->mfunc->param; p; p = p->next)
     {
          a = p->arg;
          Address=' ';
          if ((a->var->DeclType == DT_Stype) && (ISARG_EXPLICITPOINTER(a)))
               /*
                * using DeclType field, try to automatically manage all cases
                * where one as to set Address='&' : this is when the variable
                * was declared in the A-file using its Stype while, in the
                * module's main function, the variable is an explicit pointer.
                * for the list of cases, see genmain lines 496-504.
                */
               Address='&';
          if (p != C->mfunc->param)
               fprintf(afile, ", ");
          fprintf(afile, "%c%s", Address, p->Name);
     }
     fprintf(afile, ");");

     fprintf(afile, "\n/* ~~~ end of [print_main_function_call] ~~~*/\n\n");
}


/*
 * write usage function and fill <usagebuf> global variable
 */

/* TODO: cleanup this! */
#define ADDUSAGE1(a)                                               \
     {                                                             \
          char tmp[BUFSIZ];                                        \
          fprintf(afile, a);                                       \
          sprintf(tmp, "%s%s", usagebuf, a);                       \
          strcpy(usagebuf, tmp);                                   \
     }

#define ADDUSAGE2(a, b)                                            \
     {                                                             \
          char tmp[BUFSIZ];                                        \
          fprintf(afile, a, b);                                    \
          sprintf(buffer, a, b);                                   \
          sprintf(tmp, "%s%s", usagebuf, buffer);                  \
          strcpy(usagebuf, tmp);                                   \
     }

static void writeusage(FILE * afile)
{
     char Vers[TREESTRSIZE], Auth[TREESTRSIZE], Func[TREESTRSIZE], \
          Lab[TREESTRSIZE];
     char buffer[STRSIZE];
     t_argument * a;

     fprintf(afile, "/**\n");
     fprintf(afile, " * usage information for the module executable\n");
     fprintf(afile, " */\n");
     fprintf(afile, "void usage_%s(char *msg)\n", H->Name);
     fprintf(afile, "{\n");
     strcpy(Auth, getprintfstring(H->Author));
     strcpy(Vers, getprintfstring(H->Version));
     strcpy(Func, getprintfstring(H->Function));
     strcpy(Lab, getprintfstring(H->Labo));

     module_presentation(afile, H->Name, Vers, Auth, Func, Lab);
     fprintf(afile, "  if (msg != NULL)\n");
     fprintf(afile, "    fprintf(stderr, \"error : %%s\\n\\n\", msg) ;\n");

     /* build usage line */
     fprintf(afile, "  fprintf(stderr, \"usage : %%s%%s");
     sprintf(usagebuf, "usage : %s", H->Name);

     /* options */
     if (H->NbOption>0)
          for (a = H->usage; a ;a = a->next)
               if (ISARG_OPTION(a))
               {
                    ADDUSAGE2(" [-%c", a->Flag);
                    if (a->H_id[0] != '\0')
                         ADDUSAGE2(" %s]", a->H_id);
                    if (a->H_id[0] == '\0')
                         ADDUSAGE1("]");
               }

     /* needed arg */
     if (H->NbNeededArg > 0)
          for (a = H->usage; a; a = a->next)
               if (ISARG_NEEDED(a))
               {
                    if (!(ISARG_SCALAR(a) && ISARG_OUTPUT(a)))
                         ADDUSAGE2(" %s", a->H_id);
                    if ((ISARG_SCALAR(a) || !ISARG_OUTPUT(a)))
                         /*
                          * in case of a scalar output needed argument,
                          * something (as .) has to be given
                          * in the command line to avoid argument shifting.
                          */
                         ADDUSAGE1(" .");
               }

     /* optional argument */
     if (H->NbOptionArg>0)
     {
          /*
           * beware :
           * see if it would be useful to check if the following test
           * is never verified for all optional arg, in order not to write [].
           */
          fprintf(afile, " [");
          for (a = H->usage; a; a = a->next)
               if (ISARG_OPTARG(a))
                    if (!(ISARG_SCALAR(a) && ISARG_OUTPUT(a)))
                         ADDUSAGE2(" %s", a->H_id);
          ADDUSAGE1(" ]");
     }

     /* variable arguments */
     if (H->NbVarArg > 0)
          for (a = H->usage; a; a = a->next)
               if (ISARG_VARIABLE(a))
               {
                    if (!(ISARG_SCALAR(a) && ISARG_OUTPUT(a)))
                         ADDUSAGE1(" [...]");
               }

     fprintf(afile, "\\n\\n\", mwname, (help_flg) ? _mwdefoptbuf : \"\");\n");
     strcat(usagebuf, "\\n");

     /* build lines describing arguments with comments */

     /* options */
     if (H->NbOption>0)
          for (a = H->usage; a; a = a->next)
               if (ISARG_OPTION(a))
               {
                    fprintf(afile, "  fprintf(stderr, \"");
                    ADDUSAGE2("\\t-%c", a->Flag);
                    if (a->H_id[0] != '\0')
                         ADDUSAGE2(" %s", a->H_id);
                    print_interval_and_defval(afile, a);
                    if (ISARG_SCALAR(a) && ISARG_OUTPUT(a))
                         ADDUSAGE1(" screen output");
                    ADDUSAGE2(" :\\t%s\\n", getprintfstring(a->Cmt));
                    fprintf(afile, "\");\n");
               }

     /* needed arg */
     if (H->NbNeededArg > 0)
          for (a = H->usage; a; a=a->next)
               if (ISARG_NEEDED(a))
               {
                    fprintf(afile, "  fprintf(stderr, \"");
                    if (!(ISARG_SCALAR(a) && ISARG_OUTPUT(a)))
                    {
                         ADDUSAGE2("\\t%s", a->H_id);
                         print_interval_and_defval(afile, a);
                         ADDUSAGE2(" :\\t%s\\n", getprintfstring(a->Cmt));
                    }
                    else
                         ADDUSAGE2("\\t. (screen output) :\\t%s\\n", \
                                   getprintfstring(a->Cmt));
                    fprintf(afile, "\");\n");
               }

     /* optional arguments */
     if (H->NbOptionArg > 0)
          for (a = H->usage; a; a = a->next)
               if (ISARG_OPTARG(a))
               {
                    fprintf(afile, "  fprintf(stderr, \"");
                    if (!(ISARG_SCALAR(a) && ISARG_OUTPUT(a)))
                    {
                         ADDUSAGE2("\\t%s", a->H_id);
                         print_interval_and_defval(afile, a);
                         ADDUSAGE2(" :\\t%s\\n", getprintfstring(a->Cmt));
                    }
                    else
                         ADDUSAGE2("\\t. (screen output) :\\t%s\\n", \
                                   getprintfstring(a->Cmt));
                    fprintf(afile, "\");\n");
               }

     /* variable arg */
     if (H->NbVarArg > 0)
          for (a = H->usage; a; a = a->next)
               if (ISARG_VARIABLE(a))
               {
                    fprintf(afile, "  fprintf(stderr, \"");
                    if (!(ISARG_SCALAR(a) && ISARG_OUTPUT(a)))
                         ADDUSAGE2("\\t... :\\t%s\\n", \
                                   getprintfstring(a->Cmt));
                    if ((ISARG_SCALAR(a) || ISARG_OUTPUT(a)))
                         ADDUSAGE2("\\tscreen output... :\\t%s\\n", \
                                   getprintfstring(a->Cmt));
                    fprintf(afile, "\");\n");
               }

     /* FIXME: this function should exit(1), not return(0) */
     fprintf(afile, "  exit(EXIT_FAILURE);\n");
     fprintf(afile, "}\n");
     fprintf(afile, "\n");
}


/*
 * write body of main function ~~~
 */
static void writebody(FILE * afile)
{
    fprintf(afile, "/*\n");
    fprintf(afile, " * BODY OF THE MAIN FUNCTION\n");
    fprintf(afile, " */\n\n");

     fprintf(afile, "  /* FIXME: (sometimes) unused variable */\n");
     fprintf(afile, "  %sc = 0;\n", MWPF);

     fprintf(afile, "  strcpy(%stype, \"?\");\n", MWPF);
     fprintf(afile, "  %scomment[0] = '\\0';\n\n", MWPF);

     if (H->NbVarArg > 0)
          fprintf(afile, "  %svar_flg = FALSE;\n", MWPF);

     /* call to function which do MegaWave default options actions */
     fprintf(afile, "  mwdefopt(\"%s\", mwicmd, mwind);\n\n", H->Version);

     /*
      * generate code to verify that input files are readable, output
      * files are writable and numerical values are in range
      */
     print_check_io_arg(afile);

     /* generate code to read and set input variables */
     print_read_input(afile);

     /* generate call to main function */
     print_main_function_call(afile);

     /* generate code to write output variables */
     print_write_output(afile);

     /* exit from main function */
     fprintf(afile, "return 0;\n");
     fprintf(afile, "}\n");
     fprintf(afile, "\n");
}

/*
 * write main() function of the executable module
 */
static void writemain(FILE * afile)
{
    fprintf(afile, "/**\n");
    fprintf(afile, " * main() function of the executable module\n");
    fprintf(afile, " */\n");
    fprintf(afile, "int main(int argc, char ** argv, char ** envp)\n");
    fprintf(afile, "{\n");
    fprintf(afile, "    return _mw_main(argc, argv, envp, mwicmd, mwind);\n");
    fprintf(afile, "}\n");
}

/*
 * main entry : generate A-file
 */
void gen_exec_file(FILE * afile)
{
     writeAheader(afile);
     writegendecl(afile);
     writeargdecl(afile);
     writebody(afile);
     writeusage(afile);
     writemain(afile);
}
