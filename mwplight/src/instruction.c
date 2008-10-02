/**
 * @file instruction.c
 *
 * analyse some instructions in the body of a megawave module
 *
 * @author Jacques Froment <jacques.froment@univ-ubs.fr> (2005 - 2007), \
 *         Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

/* TODO : enforce ANSI only, drop K&R */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mwplight-defs.h"

#include "io.h"
#include "tree.h"

#include "instruction.h"

#define MSG_ERROR_MEMORY_USERTYPE \
    "Not enough memory to allocate Cuserdatatype"
#define MSG_ERROR_MAX_USERTYPE \
    "Too many C user's data types. Maximum is %d"
#define MSG_ERROR_UNEXPECTED_FUNCTION \
    "Parse error in \"%s\" : unexpected declaration of function while scanning previous block of instructions.\nTip : check if there is no missing declaration of parameter in the previous function."
#define MSG_ERROR_UNEXPECTED_PAREN \
    "Parse error : unexpected ')' outside function"
#define MSG_ERROR_TYPES_NUMBER \
    "Parse error in \"%s\" : number of types (%d) does not match number of parameters (%d) in ANSI function prototype"
#define MSG_ERROR_INTERPRET_SYMBOL \
    "Parse error : I don't know how to interpret symbol(s) \"%s\""
#define MSG_ERROR_INTERPRET_IDENTIFIER \
    "Parse error : I don't know how to interpret C identifier \"%s\""
#define MSG_ERROR_FUNCTION_PAREN \
    "Parse error : cannot interpret instruction \"%s\" as a function (missing right parenthesis)"
#define MSG_ERROR_FUNCTION_ENTRY \
    "Parse error : cannot interpret instruction \"%s\" as a function (nparam<0 : function's entry not found)"
#define MSG_ERROR_PARAM_NB \
    "Parse error : cannot interpret instruction \"%s\" as a function (only %d parameter(s) for %d variable(s)).\nTip : check declaration of last function"
#define MSG_ERROR_INCOMPATIBLE_TYPES \
    "Parse error in \"%s\" : incompatible types in \"%s\""
#define MSG_ERROR_COMP_LONG \
    "Parse error in \"%s\" : unsupported composite long type in \"%s\""
#define MSG_ERROR_COMP_SHORT \
    "Parse error in \"%s\" : unsupported composite short type in \"%s\""
#define MSG_ERROR_COMP_CHAR \
    "Parse error in \"%s\" : unsupported composite char type in \"%s\""
#define MSG_ERROR_COMP_FLOAT \
    "Parse error in \"%s\" : unsupported composite float type in \"%s\""
#define MSG_ERROR_COMP_DOUBLE \
    "Parse error in \"%s\" : unsupported composite double type in \"%s\""
#define MSG_ERROR_INCOMPATIBLE_TYPES \
    "Parse error in \"%s\" : incompatible types in \"%s\""
#define MSG_ERROR_INVALID_ITYPE \
    "Invalid Itype=%d"
#define MSG_ERROR_DOUBLE_STORAGE_CLASS \
    "Parse error in \"%s\" : double C storage class \"%s\" and \"%s\""
#define MSG_ERROR_WTYPE \
    "\"%s\" : W_FUNCPARAM expected for t_token '%s' instead of Wtype=%d"
#define MSG_ERROR_PARAM_MISSING \
    "\"%s\" : %d parameters recorded while %d expected"
#define MSG_ERROR_KR_PARAM_LIST \
    "\"%s\" : cannot find first W_FUNCPARAM in parameter's list"
#define MSG_ERROR_KR_PARAM_DECLARATION \
    "\"%s\" : cannot find type declaration for parameter '%s' (0)"
#define MSG_ERROR_KR_PARAM_NB \
    "\"%s\" : %d parameters recorded while %d expected"
#define MSG_ERROR_FUNCTION_NAME \
    "function's name not found in \"%s\""
#define MSG_ERROR_DUPLICATE_MAIN \
    "Duplicate main function in \"%s\""
#define MSG_ERROR_FUNCTION_MISSING_PARAMS \
    "\"%s\" : no t_token after function's name while nparam=%d"
#define MSG_ERROR_FUNCTION_ITYPE \
    "\"%s\" : not a function Itype=%d"
#define MSG_ERROR_FUNCTION_END \
    "Parse error : unexpected end of file while decoding a block of instructions.\nTip : check if there is no missing '}' nor missing declaration of parameter in a function."
#define MSG_ERROR_UNEXPECTED_ITYPE \
    "Parse error : unexpected type of instruction Itype=%d"

#define MSG_DEBUG_NEW_USERTYPE \
    "New User data type \"%s\" added"
#define MSG_DEBUG_ITYPE_PREVIOUS \
    "Set Itype to previous one"
#define MSG_DEBUG_SET_NPARAM \
    "Set nparam=%d."
#define MSG_DEBUG_SET_NVAR \
    "Set nvar=%d."
#define MSG_DEBUG_WORD\
    "Word \"%s\": "
#define MSG_DEBUG_SKIP_STRING \
    "skip string "
#define MSG_DEBUG_NAME \
    "Name : %s"
#define MSG_DEBUG_SKIP_CHAR \
    "skip char "
#define MSG_DEBUG_NB \
    "Number of parameters=%d  of variables=%d  of datatype=%d"
#define MSG_DEBUG_FUNC \
    "entering %s"
#define MSG_DEBUG_NPARAM \
    "[FillNewFunction] nparam=%d"
#define MSG_DEBUG_CONTINUE \
    "*** Continue Instruction"

/*
 * C keywords sorted by types
 */

/* C data type. Put also most common types defined in include files */
static char * Cdatatype[] = {"char", "double", "float", "int", "long",  \
                             "short", "void", "time_t", "bool", "byte", ""};

static char * Cmodifier[]      = {"long", "short", "signed", "unsigned", ""};
static char * Cpointer[]       = {"*", ""};      /* notice : [] not allowed */
static char * Cqualifier[]     = {"const", "volatile", ""};
static char * Cfuncspecifier[] = {"inline", ""}; /* added to c99
                                                  * language specification  */
static char * Cstorage[]       = {"auto", "register", "static", "extern", ""};
static char * Cdeclaretype[]   = {"typedef", ""};
static char * Cstruct[]        = {"enum", "struct", "union", ""};
static char * Cotherkey[]      = {"break", "case", "continue", "default", \
                                  "do", "else", "for", "goto", "if", "return", \
                                  "sizeof", "switch", "while", ""};

/*
 * list all MW2 data type, INCLUDING those without I/O procedures
 * (such as Point_curve)
 */
static char * Cmwdatatype[] = {"Cimage", "Fimage", "Cmovie", "Fmovie",  \
                               "Polygon", "Polygons", "Fpolygon",       \
                               "Fpolygons", "Curve", "Fcurve",          \
                               "Dcurve", "Curves", "Fcurves",           \
                               "Dcurves", "Fsignal","Wtrans1d",         \
                               "Wtrans2d", "Vchain_wmax",               \
                               "Vchains_wmax", "Ccimage", "Cfimage",    \
                               "Modules", "Ccmovie", "Cfmovie",         \
                               "Morpho_line", "Fmorpho_line",           \
                               "Morpho_set", "Morpho_sets",             \
                               "Mimage", "Cmorpho_line",                \
                               "Cfmorpho_line", "Cmorpho_set",          \
                               "Cmorpho_sets", "Cmimage", "Shape",      \
                               "Shapes", "Rawdata", "Flist", "Flists",  \
                               "Dlist", "Dlists", "Wpack2d",            \
                               "Point_curve", "Point_fcurve", "Wframe", \
                               "Color", "Point_plane", "Wpanel", ""};

/* data type defined by the user in the C body */
static char * Cuserdatatype[MAXDT];
/* number of user-defined data type */
static int Nudt = 0;

#define Is_Cdatatype(x)      (Is_Ctype((x), Cdatatype))
#define Is_Cmodifier(x)      (Is_Ctype((x), Cmodifier))
#define Is_Cpointer(x)       (Is_Ctype((x), Cpointer))
#define Is_Cqualifier(x)     (Is_Ctype((x), Cqualifier))
#define Is_Cfuncspecifier(x) (Is_Ctype((x), Cfuncspecifier))
#define Is_Cstorage(x)       (Is_Ctype((x), Cstorage))
#define Is_Cdeclaretype(x)   (Is_Ctype((x), Cdeclaretype))
#define Is_Cstruct(x)        (Is_Ctype((x), Cstruct))
#define Is_Cotherkey(x)      (Is_Ctype((x), Cotherkey))
#define Is_Ckeyword(x)       (Is_Cdatatype(x)      || Is_Cmodifier(x)   || \
                              Is_Cpointer(x)       || Is_Cqualifier(x)  || \
                              Is_Cfuncspecifier(x) || Is_Cstorage(x)    || \
                              Is_Cdeclaretype(x)   || Is_Cotherkey(x))
#define Is_Cmwdatatype(x)    (Is_Ctype((x), Cmwdatatype))
#define Is_Cuserdatatype(x)  (Is_Ctype((x), Cuserdatatype))
#define Is_Canydatatype(x)  (Is_Cdatatype(x)       || Is_Cmwdatatype(x) || \
                             Is_Cuserdatatype(x))

#define MSG_ERROR_INTERPRET_INSTRUCTION \
     "Parse error : cannot interpret instruction \"%s\" as declaration of type"
#define MSG_ERROR_UNEXPECTED_KEYWORD \
     "Parse error : unexpected keyword \"%s\" outside any block of instructions"
#define MSG_ERROR_UNEXPECTED_SEMICOLON \
     "Parse error in \"%s\" : unexpected ';' before end of instructions"

/*
 * init the Cuserdatatype array
 */
void Init_Cuserdatatype(void)
{
     int n;

     Nudt = 0;
     for (n = 0; n < MAXDT; n++)
     {
          Cuserdatatype[n] = (char *) malloc(DTSTRSIZE * sizeof(char));
          if (Cuserdatatype[n] == NULL)
               error(MSG_ERROR_MEMORY_USERTYPE);
     }
     Cuserdatatype[Nudt][0] = '\0';

}

/*
 * free the Cuserdatatype array.
 */
void Free_Cuserdatatype(void)
{
     int n;

     Nudt = 0;
     for (n = 0; n < MAXDT; n++)
          free(Cuserdatatype[n]);
}

/*
 * add string <s> as a new user data type in <Cuserdatatype>.
 */
static void Add_Cuserdatatype(char * s)
{
     Nudt++;
     if (Nudt > MAXDT)
          error(MSG_ERROR_MAX_USERTYPE, MAXDT);
     strcpy(Cuserdatatype[Nudt - 1], s);
     Cuserdatatype[Nudt][0] = '\0';
     if (debug_flag)
          debug(MSG_DEBUG_NEW_USERTYPE, s);
}

/*
 * return 1 if string <s> is of type <type>, 0 elsewhere.
 * <type> is assumed to be a list of C keywords.
 */
static int Is_Ctype(char * s, char * type[])
{
     int i;
     for (i = 0; (type[i][0] != '\0') && (strcmp(s, type[i]) != 0); i++);
     return(type[i][0] != '\0');
}


/*
 * from the chain of words in <c>,
 * analyse the instruction and set the other fields of <c>.
 * return 1 if the instruction is to be continued, 0 elsewhere.
 */
static int AnalyseInstruction(t_statement * c)
{
     t_token * cw, * cw0, * cwb;
     char * w;
     /* number of non-interpreted parenthesis */
     int nipar;

     nipar = 0;
     c->nparam = -1;
     c->nvar   =  0;

     /*
      * if c is the continuation of an instruction,
      * set the instruction type
      */
     if (c->previous)
     {
          c->Itype = c->previous->Itype;
          if (debug_flag)
               debug(MSG_DEBUG_ITYPE_PREVIOUS);
          c->nparam = c->previous->nparam;
          if (debug_flag)
               debug(MSG_DEBUG_SET_NPARAM, c->nparam);
          c->nvar = c->previous->nvar;
          if (debug_flag)
               debug(MSG_DEBUG_SET_NVAR, c->nvar);
     }

     /* scan each word */
     for (cw = c->wfirst, cw0 = NULL; cw; cw0 = cw, cw = cw->next)
     {
          w = cw->Name;
          if (debug_flag)
               debug(MSG_DEBUG_WORD, w);

          /* first, recognize keywords */
          if (Is_Cdeclaretype(w))
          {
               /* declaration of type : typedef */
               cw->Wtype = W_CDECLARETYPE;
               if (debug_flag)
                    debug("W_CDECLARETYPE");
               /*
                * a typedef is not supposed
                * to be encountered in other labelled instructions
                */
               if (c->Itype != I_UNDEFINED)
                    error(MSG_ERROR_INTERPRET_INSTRUCTION, c->phrase);
               c->Itype = I_CDECLARETYPE;
               if (debug_flag)
                    debug("I_CDECLARETYPE");
               continue;
          }
          if (Is_Cstruct(w))
          {
               /* declaration of structure : struct, union, ... */
               cw->Wtype = W_STRUCT;
               if (debug_flag)
                    debug("W_STRUCT");
               continue;
          }
          if (Is_Cstorage(w))
          {
               /* C storage : extern, static, register, ... */
               cw->Wtype=W_CSTORAGE;
               if (debug_flag)
                    debug("W_CSTORAGE");
               continue;
          }
          if (Is_Cqualifier(w))
          {
               /* C qualifier : const, volatile */
               cw->Wtype = W_CQUALIFIER;
               if (debug_flag)
                    debug("W_CQUALIFIER");
               continue;
          }
          if (Is_Cfuncspecifier(w))
          {
               /* C qualifier : const, volatile */
               cw->Wtype = W_CFUNCSPECIFIER;
               if (debug_flag)
                    debug("W_CFUNCSPECIFIER");
               continue;
          }
          if (Is_Cdatatype(w))
          {
               /* data type : char, double, ... */
               cw->Wtype = W_DATATYPE;
               if (debug_flag)
                    debug("W_DATATYPE");
               if (c->Itype == I_FUNC_IN)
                    c->ndatatype++;
               continue;
          }
          if (Is_Cmodifier(w))
          {
               /* C modifier : long, unsigned, ... */
               cw->Wtype = W_CMODIFIER;
               if (debug_flag)
                    debug("W_CMODIFIER");
               continue;
          }
          if (Is_Cpointer(w))
          {
               /* C pointer : *, [] */
               cw->Wtype = W_CPOINTER;
               if (debug_flag)
                    debug("W_CPOINTER");
               continue;
          }
          if (Is_Cmwdatatype(w))
          {
               /* MW type */
               cw->Wtype = W_CMWDATATYPE;
               if (debug_flag)
                    debug("W_CMWDATATYPE");
               if (c->Itype == I_FUNC_IN)
                    c->ndatatype++;
               continue;
          }
          if (Is_Cuserdatatype(w))
          {
               /* User's defined data type */
               cw->Wtype = W_USERDATATYPE;
               if (debug_flag)
                    debug("W_USERDATATYPE");
               if (c->Itype==I_FUNC_IN)
                    c->ndatatype++;
               continue;
          }

          if (Is_Cotherkey(w))
               error(MSG_ERROR_UNEXPECTED_KEYWORD, w);

          /*
           * word is not a keyword
           * try to interpret it using the context
           */
          if (!IsStringCid(w))
          {
               /* word is not a C identifier, such as a name */
               if (strcmp(w, "\"")==0)
               {
                    if ((!cw0)||(strcmp(cw0->Name, "\\")!=0))
                    {
                         /* enter a string : skip it */
                         cw->Wtype = W_SEPARATOR;
                         if (debug_flag)
                              debug("W_SEPARATOR");
                         if (debug_flag)
                              debug(MSG_DEBUG_SKIP_STRING);
                         cw = cw->next;
                         while (cw && ((strcmp(cw->Name, "\"") != 0)))
                         {
                              if (debug_flag)
                                   debug(MSG_DEBUG_NAME, cw->Name);
                              cw = cw->next;
                         }
                    }
                    continue;
               }

               if (strcmp(w, "'") == 0)
               {
                    if ((!cw0) || (strcmp(cw0->Name, "\\") != 0))
                    {
                         /* enter a char 'c' : skip it */
                         cw->Wtype = W_SEPARATOR;
                         if (debug_flag)
                              debug("W_SEPARATOR");
                         if (debug_flag)
                           debug(MSG_DEBUG_SKIP_CHAR);
                         cw = cw->next;
                         while (cw && ((strcmp(cw->Name, "'") !=0 )))
                         {
                              if (debug_flag)
                                   debug(MSG_DEBUG_NAME, cw->Name);
                              cw = cw->next;
                         }
                    }
                    continue;
               }

               if (strcmp(w, ";") == 0)
               {
                    /* should close the instruction */
                    cw->Wtype = W_SEPARATOR;
                    if (debug_flag)
                         debug("W_SEPARATOR");
                    if (cw->next)
                         error(MSG_ERROR_UNEXPECTED_SEMICOLON, c->phrase);
                    continue;
               }

               if ((strcmp(w, "{") == 0) || (strcmp(w, "}") == 0))
               {
                    /* instruction includes a block {} */
                    cw->Wtype = W_SEPARATOR;
                    if (debug_flag)
                         debug("W_SEPARATOR");
                    continue;
               }

               if (strcmp(w, "(") == 0)
               {
                    /* may be a function declaration */
                    cw->Wtype = W_SEPARATOR;
                    if (debug_flag)
                         debug("W_SEPARATOR");
                    if ( cw0 && \
                         /* normal case */
                         ((cw0->Wtype == W_NAME) ||     \
                          /*
                           * case with parenthesis such as  double (*RI) ();
                           * see image/io/cfchgchannels.c
                           * and wave/sconvolve.c
                           */
                          ((strcmp(cw0->Name, ")") == 0) &&    \
                           (cw0->previous) &&                  \
                           (cw0->previous->Wtype == W_NAME) && \
                           (c->Itype == I_UNDEFINED))))
                    {
                         /*
                          * previous word was name
                          * interpret as a function declaration
                          */
                         if (c->Itype != I_UNDEFINED)
                              error(MSG_ERROR_UNEXPECTED_FUNCTION, c->phrase);
                         c->Itype = I_FUNC_IN;
                         if (debug_flag)
                              debug("I_FUNC_IN");
                         c->nparam    = 0;
                         c->ndatatype = 0;
                         if (cw0->Wtype == W_NAME)
                              cwb = cw0;
                         else
                              cwb = cw0->previous;
                         cwb->Wtype = W_FUNCNAME;
                         if (debug_flag)
                              debug(MSG_DEBUG_NAME, cwb->Name);
                         c->nvar--;
                         continue;
                    }
                    else
                    {
                         /* don't know what it is : skip it */
                         nipar++;
                         continue;
                    }
               }
               if (strcmp(w, ")") == 0)
               {
                    cw->Wtype = W_SEPARATOR;
                    if (debug_flag)
                         debug("W_SEPARATOR");
                    if (c->Itype != I_FUNC_IN)
                    {
                         nipar--;
                         if (nipar < 0)
                              error(MSG_ERROR_UNEXPECTED_PAREN);
                         /*
                          * see image/io/cfchgchannels.c
                          * and wave/sconvolve.c
                          */
                         continue;
                    }
                    if ((cw->next) &&                          \
                        ((strcmp(cw->next->Name, ";") == 0) || \
                         (strcmp(cw->next->Name, ",") == 0)))
                    {
                         /* this is a function prototype */
                         if (c->ndatatype > 0)
                         {
                              /*
                               * data type has been used inside
                               * the function parameter list : ANSI
                               */
                              if ((c->nparam>0) && (c->ndatatype != c->nparam))
                                   error(MSG_ERROR_TYPES_NUMBER, \
                                         c->phrase, c->ndatatype, c->nparam);
                              c->Itype = I_FUNCPROTO_ANSI;
                              if (debug_flag)
                                   debug("I_FUNCPROTO_ANSI");
                              c->nparam = c->ndatatype;
                              c->nvar   = c->ndatatype;
                         }
                         else
                         {
                              c->Itype = I_FUNCPROTO_KR;
                              if (debug_flag)
                                   debug("I_FUNCPROTO_KR");
                         }
                    }
                    else
                    {
                         /* this is a function declaration */
                         if (c->ndatatype > 0)
                         {
                              if (c->ndatatype != c->nparam)
                                   error(MSG_ERROR_TYPES_NUMBER, \
                                         c->phrase, c->ndatatype, c->nparam);
                              c->Itype = I_FUNCDECL_ANSI;
                              if (debug_flag)
                                   debug("I_FUNCDECL_ANSI");
                              c->nvar = c->ndatatype;
                         }
                         else
                         {
                              c->Itype = I_FUNCDECL_KR;
                              if (debug_flag)
                                   debug("I_FUNCDECL_KR");
                         }
                    }
                    continue;
               }

               if (strcmp(w, ",") == 0)
               {
                    /* list of variables or parameters */
                    cw->Wtype = W_SEPARATOR;
                    if (debug_flag)
                         debug("W_SEPARATOR");
                    if ((c->Itype == I_FUNCPROTO_ANSI) || \
                        (c->Itype == I_FUNCPROTO_KR))
                         /*
                          * to avoid error in case of several proto
                          * on the same line.
                          * ex. : extern float smean(), snorm();
                          */
                         c->Itype = I_UNDEFINED;
                    continue;
               }

               /* other symbols */
               if (cw0 && (cw0->Wtype == W_NAME))
               {
                    /*
                     * previous word was name :
                     * should be a complex variable(s) definition
                     * we don't want to manage
                     */
                    if (c->Itype == I_UNDEFINED)
                    {
                         c->Itype = I_VARDECL;
                         if (debug_flag)
                              debug("I_VARDECL");
                    }
                    /* FIXME: cw = ??? */
/*                  for (cw; cw && (strcmp(cw->Name, ",") != 0);        \
 *                       cw0 = cw, cw = cw->next);                        */
                    for ( ; cw && (strcmp(cw->Name, ",") != 0); \
                         cw0 = cw, cw = cw->next);
                    if (cw)
                         continue;
                    else
                         goto EndofPass1;
               }

               /*
                * other case ; don't know what it is.
                * may be symbol used in declaration
                * such as '[' in char toto[10].
                * end analysis if the instruction is already interpreted.
                */
               if (c->Itype != I_UNDEFINED)
               {
                 /* FIXME: cw = ??? */
/*                  for (cw; cw && (strcmp(cw->Name, ",") != 0);        \
 *                       cw0=cw, cw=cw->next);                            */
                    for ( ; cw && (strcmp(cw->Name, ",") != 0); \
                         cw0=cw, cw=cw->next);
                    if (cw)
                         continue;
                    else
                         goto EndofPass1;
               }

               error(MSG_ERROR_INTERPRET_SYMBOL, w);
               continue;

          } /* if (!IsStringCid(w)) */

          /* word is a C identifier */

          if (cw0 && ((cw0->Wtype == W_DATATYPE)      ||        \
                      (cw0->Wtype == W_CMWDATATYPE)   ||        \
                      (cw0->Wtype == W_USERDATATYPE)) &&        \
              cw0->previous && (cw0->previous->Wtype == W_CDECLARETYPE))
          {
               /* typedef datatype newdatatype */
               cw->Wtype = W_USERDATATYPE;
               if (debug_flag)
                    debug("W_USERDATATYPE");
               Add_Cuserdatatype(w);
               if (c->Itype == I_FUNC_IN)
                    c->ndatatype++;
               continue;
          }

          if (cw0&&(cw0->Wtype == W_STRUCT))
          {
               /* struct name */
               cw->Wtype = W_USERDATATYPE;
               if (debug_flag)
                    debug("W_USERDATATYPE");
               /*
                * do not add this word as a datatype
                * since it must be preceding
                * by "struct" in order to be recognized as a datatype
                */
               if (c->Itype == I_FUNC_IN)
                    c->ndatatype++;
               continue;
          }

          if (cw0 && ((cw0->Wtype == W_DATATYPE) ||     \
                      (cw0->Wtype == W_CMWDATATYPE) ||  \
                      (cw0->Wtype == W_USERDATATYPE) || \
                      (cw0->Wtype == W_CMODIFIER) ||    \
                      (cw0->Wtype == W_CPOINTER) ||     \
                      (strcmp(cw0->Name, "}") == 0)))
          {
               /*
                * previous word was data type or modifier or pointer
                * or end of block {}.
                * this one may be new variable or function name
                */
               if (c->Itype == I_CDECLARETYPE)
               {
                    /* this name may be a new data type : add it */
                    cw->Wtype = W_USERDATATYPE;
                    if (debug_flag)
                         debug("W_USERDATATYPE");
                    Add_Cuserdatatype(w);
                    if (c->Itype == I_FUNC_IN)
                         c->ndatatype++;
                    continue;
               }
               else if (c->Itype != I_FUNC_IN)
               {
                    /* we are not inside the parameter list */
                    cw->Wtype = W_NAME;
                    /* variable or function name */
                    if (debug_flag)
                         debug("W_NAME");
                    c->nvar++;
                    continue;
               }
          }

          if ((c->Itype == I_FUNC_IN) && \
              ((!cw->next) || (strcmp(cw->next->Name, ",") == 0) || \
               (strcmp(cw->next->Name, ")") == 0)))
          {
               /* a parameter of the function */
               cw->Wtype = W_FUNCPARAM;
               /* function's parameter */
               if (debug_flag)
                    debug("W_FUNCPARAM");
               c->nparam++;
               continue;
          }

          if (cw0 && (strcmp(cw0->Name, ",") == 0))
          {
               /* Cid in a list : assume it is a variable or function name */
               cw->Wtype = W_NAME;
               if (debug_flag)
                    debug("W_NAME");
               c->nvar++;
               continue;
          }

          if (cw->next && (strcmp(cw->next->Name, "(") == 0))
          {
               /* next word is "(" : assume it is a variable or function name */
               cw->Wtype = W_NAME;
               if (debug_flag)
                    debug("W_NAME");
               c->nvar++;
               continue;
          }

          if ((cw->next) && ((strcmp(cw->next->Name, "*") == 0) ||      \
                             ((!Is_Ckeyword(cw->next->Name)) &&         \
                              (!Is_Canydatatype(cw->next->Name)) &&     \
                              (IsStringCid(cw->next->Name)))))
          {
               /*
                * if next word is a name or *,
                * assume current one is an unknown type and add it.
                */
               cw->Wtype = W_USERDATATYPE;
               if (debug_flag)
                    debug("W_USERDATATYPE");
               Add_Cuserdatatype(w);
               if (c->Itype == I_FUNC_IN)
                    c->ndatatype++;
               continue;
          }

          /*
           * don't know what it is.
           * end analysis if the instruction is already interpreted.
           */
          if (c->Itype != I_UNDEFINED)
          {
/* FIXME: cw = ??? */
/*             for (cw; cw && (strcmp(cw->Name, ",") != 0);     \
 *                  cw0 = cw, cw = cw->next);                     */
               for ( ; cw && (strcmp(cw->Name, ",") != 0);      \
                    cw0 = cw, cw = cw->next);
               if (cw)
                    continue;
               else
                    goto EndofPass1;
          }

          error(MSG_ERROR_INTERPRET_IDENTIFIER, w);
          continue;

     } /* end of for (cw=c->wfirst, ...) */

     if (cw0 && ((cw0->Wtype == W_NAME) ||                              \
                 ((strcmp(cw0->Name, ";") == 0) &&                      \
                  (cw0->previous) && (cw0->previous->Wtype == W_NAME))))
     {
          /*
           * last word was name
           * word cw0 is a simple variable definition
           */
          if (c->Itype == I_UNDEFINED)
          {
               c->Itype = I_VARDECL;
               if (debug_flag)
                    debug("I_VARDECL");
          }
     }

EndofPass1:

     if (c->Itype == I_UNDEFINED)
          /*
           * first pass couldn't interpret the instruction
           * assume it does not contain
           * anything interesting
           */
          return(0);

     if (c->Itype == I_FUNC_IN)
          error(MSG_ERROR_FUNCTION_PAREN, c->phrase);

     if ((ISCI_FUNCTION(c)) && (c->nparam<0))
          error(MSG_ERROR_FUNCTION_ENTRY, c->phrase);


     /*
      * TODO: replace the counter with a parameter=variable identification
      */
     if (debug_flag)
          debug(MSG_DEBUG_NB, c->nparam, c->nvar, c->ndatatype);

     if (c->nparam>=0)
     {
          /* a function has been found */
          if (c->nparam < c->nvar)
               error(MSG_ERROR_PARAM_NB, \
                     c->phrase, c->nparam, c->nvar);
          if (c->nparam>c->nvar)
               return 1;
          else
               return 0;
     }
     return 0;
}

/*
 * set types in v->Stype and v->Ctype from cw,
 * assuming this word contains a type.
 */
static void SetType(t_variable * v, t_statement * c, t_token * cw)
{
     strcat(v->Stype, cw->Name);
     strcat(v->Stype, " ");
     switch(cw->Wtype)
     {
     case W_DATATYPE:
     case W_CMODIFIER:
          if ((v->Ctype != NONE) &&                                     \
              ((v->Ctype < SCALARMIN_T) || (v->Ctype > UNSIGNED_T)))
               /* allow e.g. short int */
               error(MSG_ERROR_INCOMPATIBLE_TYPES, c->phrase, v->Stype);
          if (strcmp(cw->Name, "void") == 0)
               v->Ctype = VOID_T;
          else
          {
               /*
                * have to deal with composite types
                * such as unsigned long int.
                * beware : not all composite types in C
                * are allowed, but only those listed in SCALAR C
                * types #define (see tree.h).
                */
               if (strcmp(cw->Name, "unsigned") == 0)
                    v->Ctype = UNSIGNED_T;
               else if (strcmp(cw->Name, "long") == 0)
                    {
                         if (v->Ctype == UNSIGNED_T)
                              v->Ctype = ULONG_T;
                         else
                         {
                              if (v->Ctype != NONE)
                                   error(MSG_ERROR_COMP_LONG, \
                                         c->phrase, v->Stype);
                              v->Ctype = LONG_T;
                         }
                    }
               else if (strcmp(cw->Name, "short") == 0)
               {
                    if (v->Ctype == UNSIGNED_T)
                         v->Ctype = USHORT_T;
                    else
                    {
                         if (v->Ctype != NONE)
                              error(MSG_ERROR_COMP_SHORT,               \
                                    c->phrase, v->Stype);
                         v->Ctype = SHORT_T;
                    }
               }
               else if (strcmp(cw->Name, "char") == 0)
               {
                    if (v->Ctype == UNSIGNED_T)
                         v->Ctype = UCHAR_T;
                    else
                    {
                         if (v->Ctype != NONE)
                              error(MSG_ERROR_COMP_CHAR,                \
                                    c->phrase, v->Stype);
                         v->Ctype = CHAR_T;
                    }
               }
               else
                    if (strcmp(cw->Name, "int") == 0)
                    {
                         if (v->Ctype==UNSIGNED_T)
                              v->Ctype=UINT_T;
                         else if ((v->Ctype != USHORT_T) &&             \
                                  (v->Ctype != ULONG_T) &&              \
                                  (v->Ctype != SHORT_T) &&              \
                                  (v->Ctype != LONG_T))
                              /*
                               * to deal with
                               * e.g. unsigned short int
                               */
                              v->Ctype=INT_T;
                    }
                    else if (strcmp(cw->Name, "float") == 0)
                    {
                         if (v->Ctype != NONE)
                              error(MSG_ERROR_COMP_FLOAT,               \
                                    c->phrase, v->Stype);
                         v->Ctype = FLOAT_T;
                    }
                    else if (strcmp(cw->Name, \
                                    "double") == 0)
                    {
                         if (v->Ctype != NONE)
                              error(MSG_ERROR_COMP_DOUBLE,              \
                                    c->phrase, v->Stype);
                         v->Ctype = DOUBLE_T;
                    }
          }
          break;

     case W_USERDATATYPE:
          if (v->Ctype != NONE)
               error(MSG_ERROR_INCOMPATIBLE_TYPES, c->phrase, v->Stype);
          v->Ctype = USER_T;
          break;

     case W_CMWDATATYPE:
          if (v->Ctype != NONE)
               error(MSG_ERROR_INCOMPATIBLE_TYPES, c->phrase, v->Stype);
          v->Ctype = MW2_T;
          break;
     }
}


/*
 * fill the variable <v> using the content of the t_statement chain <c>
 */
static void FillNewVariable(t_varfunc * v, t_statement * c)
{
     t_token * cw;

     if (debug_flag)
          debug(MSG_DEBUG_FUNC, __func__);
     if (c->Itype != I_VARDECL)
          error(MSG_ERROR_INVALID_ITYPE, c->Itype);
     v->Itype = c->Itype;

     cw = c->wfirst;
     while (1)
     {
          while (cw && !ISCWORD_FULLTYPE(cw))
          {
               if (cw->Wtype == W_CSTORAGE)
               {
                    if (v->v->Cstorage[0] != '\0')
                         error(MSG_ERROR_DOUBLE_STORAGE_CLASS, \
                               c->phrase, v->v->Cstorage, cw->Name);
                    strcpy(v->v->Cstorage, cw->Name);
               }
               cw = cw->next;
          }
          if (!cw)
               break;
          while (ISCWORD_FULLTYPE(cw))
          {
               strcat(v->v->Ftype, cw->Name);
               strcat(v->v->Ftype, " ");

               if (cw->Wtype == W_CPOINTER)
                    v->v->PtrDepth++;
               else
                    SetType(v->v, c, cw);
               cw = cw->next;
          }
          do
          {
               strcat(v->v->Name, cw->Name);
               strcat(v->v->Name, "");
               cw = cw->next;
          } while (cw);
     }
     RemoveTerminatingSpace(v->v->Stype);
     RemoveTerminatingSpace(v->v->Ftype);
}

/*
 * fill parameters of the function <f>
 * using the content of the t_statement chain <c>.
 * case of a ANSI function declaration.
 * ex: int local_func_decl_ansi(signed toto *a, double p)
 */
static void FillParam_Funcdecl_Ansi(t_varfunc * f, t_statement * c, \
                                    t_token * cwb)
{
     t_token * cw;
     t_variable * p;
     int np;

     if (debug_flag)
          debug(MSG_DEBUG_FUNC, __func__);
     p = f->param;
     cw = cwb;
     np = 0;
     while (1)
     {
          while (cw && !ISCWORD_FULLTYPE(cw))
               cw = cw->next;
          if (!cw)
               break;
          while (ISCWORD_FULLTYPE(cw))
          {
               strcat(p->Ftype, cw->Name);
               strcat(p->Ftype, " ");
               if (cw->Wtype == W_CPOINTER)
                    p->PtrDepth++;
               else
                    SetType(p, c, cw);
               cw = cw->next;
          }
          if (cw->Wtype != W_FUNCPARAM)
               error(MSG_ERROR_WTYPE, c->phrase, cw->Name, cw->Wtype);
          strcpy(p->Name, cw->Name);

          RemoveTerminatingSpace(p->Stype);
          RemoveTerminatingSpace(p->Ftype);

          p = p->next;
          np++;
     }

     if (np != c->nparam)
          error(MSG_ERROR_PARAM_NB, c->phrase, np, c->nparam);
}

/*
 * fill parameters of the function <f>
 * using the content of the t_statement chain <c>.
 * case of a K&R function declaration.
 * example : void frthre(A, B, noise) Fimage A, B;float *noise;
 */

static void FillParam_Funcdecl_KR(t_varfunc * f, t_statement * c, t_token * cwb)
{
     t_token * cw, * cw0, * cw1, * cwn;
     t_variable * p;
     int np, commapassed, nbcomma;

     if (debug_flag)
          debug(MSG_DEBUG_FUNC, __func__);
     p = f->param;
     cwn = cwb;
     np = 0;

     /* seek first variable name */
     while (cwn && (cwn->Wtype!=W_FUNCPARAM))
          cwn = cwn->next;
     if (!cwn)
          error(MSG_ERROR_KR_PARAM_LIST, c->phrase);
     do
     {
          /* for each cwn (W_FUNCPARAM), search for the data type */
          cw0 = cwn->next;
          if (!cw0)
               error(MSG_ERROR_KR_PARAM_DECLARATION, \
                     c->phrase, cwn->Name);
          cw1 = cw0->next;
          /* printf("\n**** Search for '%s'\n", cwn->Name); */
          while (cw1 && (strcmp(cw1->Name, cwn->Name)!=0))
               cw1 = cw1->next;
          if (!cw1)
               error(MSG_ERROR_KR_PARAM_DECLARATION, \
                     c->phrase, cwn->Name);
          /*
           * go backward and set cw to the first item
           * that may participate to the full type.
           * be aware of situations
           * such as "toto(a, b, c) float a, *b, *c;"
           * that require for c to get "float *".
           * thus, we have to scan types from the word after ')'
           * and to count the number of commas to know
           * if the '*' is for this variable.
           */
          for (cw = cw1, nbcomma = 0; \
               (cw != cw0) && (strcmp(cw->Name, ")") != 0) &&   \
               (strcmp(cw->Name, ";") != 0); \
               cw=cw->previous)
               if (strcmp(cw->Name, ",")==0)
                    nbcomma++;
          cw = cw->next;
          commapassed = 0;
          /* printf("Search from '%s' to '%s' with nbcomma=%d\n", cw->Name, cw1->Name, nbcomma); */
           /* data types are in cw ... cw1 */
          while (cw != cw1->next)
          {
               if (strcmp(cw->Name, ",") == 0)
                    commapassed++;
               else
                    if (ISCWORD_SCALARTYPE(cw) ||   \
                        ((cw->Wtype==W_CPOINTER) && \
                         (commapassed==nbcomma)))
                    {
                         /* printf("commapassed=%d. Add '%s' to Ftype\n", commapassed, cw->Name);*/
                         strcat(p->Ftype, cw->Name);
                         strcat(p->Ftype, " ");
                         if (cw->Wtype == W_CPOINTER)
                              p->PtrDepth++;
                         else
                              SetType(p, c, cw);
                    }
               cw = cw->next;
          }
          RemoveTerminatingSpace(p->Stype);
          RemoveTerminatingSpace(p->Ftype);

          strcpy(p->Name, cwn->Name);
          p = p->next;
          np++;

          cwn = cwn->next;
          while (cwn && (cwn->Wtype!=W_FUNCPARAM))
               cwn = cwn->next;
     } while (cwn);

     if (np != c->nparam)
          error(MSG_ERROR_KR_PARAM_NB, c->phrase, np, c->nparam);
}

/*
 *
 * fill parameters of the function <f>
 * using the content of the t_statement chain <c>.
 * case of a ANSI function prototype.
 */
/* void FillParam_Funcproto_Ansi(t_varfunc * f, t_statement * c, t_token * cwb)
 * {
 * }
 */

/*
 * fill parameters of the function <f>
 * using the content of the t_statement chain <c>.
 * case of a K&R function prototype.
 */
/* void FillParam_Funcproto_KR(t_varfunc * f, t_statement * c, t_token * cwb)
 * {
 * }
 */

/*
 * Fill the function <f> (and possibly the followings)
 * using the content of the t_statement chain <c>.
 * set also C->mfunc is <f> is the module's function .
 */
static void FillNewFunction(t_varfunc * f, t_statement * c)
{
     t_token * cw, * cwfn;
     t_variable * p, * p0;
     int np;

     if (debug_flag)
          debug(MSG_DEBUG_NPARAM, c->nparam);
     f->Itype = c->Itype;

     /* allocate the chain of parameters */
     for (np = 1, p0 = NULL; np <= c->nparam; np++)
     {
          p = new_variable();
          if (!f->param)
               f->param = p;
          else
               p0->next = p;
          p->previous = p0;
          p0 = p;
     }
     np = 0;

     /* first, seek for function's name so that we can set cwfn */
     cwfn = NULL;
     for (cw = c->wfirst; cw; cw = cw->next)
          if (cw->Wtype == W_FUNCNAME)
          {
               strcpy(f->v->Name, cw->Name);
               cwfn = cw;
               break;
          }
     if (!cwfn)
          error(MSG_ERROR_FUNCTION_NAME, c->phrase);

     /* seek for C storage of function */
     for (cw = c->wfirst; cw != cwfn; cw = cw->next)
     {
          if (cw->Wtype == W_CSTORAGE)
          {
               if (f->v->Cstorage[0] != '\0')
                    error(MSG_ERROR_DOUBLE_STORAGE_CLASS, \
                          c->phrase, f->v->Cstorage, cw->Name);
               strcpy(f->v->Cstorage, cw->Name);
          }
          if (ISCWORD_FULLTYPE(cw))
          {
               strcat(f->v->Ftype, cw->Name);
               strcat(f->v->Ftype, " ");
               if (cw->Wtype == W_CPOINTER)
                    f->v->PtrDepth++;
               else
                    SetType(f->v, c, cw);
          }
     }
     RemoveTerminatingSpace(f->v->Stype);
     RemoveTerminatingSpace(f->v->Ftype);

     /* check for main function */
     if ( ((f->Itype == I_FUNCDECL_ANSI) ||             \
           (f->Itype == I_FUNCDECL_KR)) &&              \
          (strcmp(f->v->Name, module_name) == 0))
     {
          /* this is the main function */
          if (C->mfunc)
               error(MSG_ERROR_DUPLICATE_MAIN, c->phrase);
          C->mfunc = f;
     }


     if (c->nparam == 0)
          /* if the function doesn't contain any parameter, we have finished */
          return;
     if (!cwfn->next)
          error(MSG_ERROR_FUNCTION_MISSING_PARAMS, c->phrase, c->nparam);

     switch(f->Itype)
     {
     case I_FUNCDECL_ANSI:
          FillParam_Funcdecl_Ansi(f, c, cwfn->next);
          break;
     case I_FUNCDECL_KR:
          FillParam_Funcdecl_KR(f, c, cwfn->next);
          break;
/* not implemented */
     case I_FUNCPROTO_ANSI:
          break;
     case I_FUNCPROTO_KR:
          break;

     default:
          error(MSG_ERROR_FUNCTION_ITYPE, c->phrase, c->Itype);
     }
}

/*
 * get from input module's file <sfile>
 * the next instruction and set the corresponding structure.
 * recognized instructions :
 * - Declarations of
 *   + global variables;
 *   + functions.
 * copy C body from <sfile> to <fm> and change variable / function
 * declarations.
 * return 1 if one instruction found (even if not recorded in C), 0
 * elsewhere (end of file)
 * assumptions :
 * - Enter in the beginning of a line.
 */
int GetNextInstruction(FILE * sfile)
{
     char s[STRSIZE];
     char w[TREESTRSIZE];
     long lb, lbeg, lend;
     int i, i0;
     t_statement * c, * c0, * cfirst;
     t_token * cw, * cw0;
     t_varfunc * f, * f0;

     if (sfile == NULL)
          return(0);
     cfirst = NULL;
     c0 = NULL;
     c  = NULL;
     lbeg = -1;

     /* build the t_statement chain */
ContinueInstruction:
     if ((getinstruction(sfile, s, &lb, &lend) == EOF) && (s[0] == '\0'))
     {
          if (c0 && (c0==c))
               error(MSG_ERROR_FUNCTION_END);
          return 0;
     }
     if (lbeg == -1)
          /* to get the beginning of instruction with continuing inst. */
          lbeg=lb;

     i0 = 0;
     cw = NULL;
     c = new_cinstruction();
     if (!cfirst)
          cfirst = c;
     strcpy(c->phrase, s);
     if (c0)
     {
          /* c = next instruction to continue c0 */
          c0->next = c;
          c->previous = c0;
          c0 = NULL;
     }
     while ((i = getword(&s[i0], w)) != 0)
     {
          cw0 = cw;
          cw = new_cword();
          if (!c->wfirst)
               c->wfirst = cw;
          else
          {
               cw0->next = cw;
               cw->previous = cw0;
          }

          strcpy(cw->Name, w);
          i0 += i;
     }
     if (AnalyseInstruction(c) == 1)
     {
          /* the instruction has to be continued */
          c0 = c;
          if (debug_flag)
               debug(MSG_DEBUG_CONTINUE);
          goto ContinueInstruction;
     }

     /*complete the t_body with the content of the t_statement chain */
     c = cfirst;
     merge_cinstruction(c);

     switch(c->Itype)
     {
     case I_UNDEFINED:
     case I_CDECLARETYPE:
          /* not an instruction to be recorded in C */
          return(1);

     case  I_VARDECL:
          f = new_varfunc();
          for (f0 = C->varfunc; f0 && f0->next; f0 = f0->next);
          if (!f0)
               C->varfunc = f;
          else
               f0->next = f;
          f->previous = f0;
          FillNewVariable(f, c);
          f->l0 = lbeg;
          f->l1 = lend;
          delete_cinstruction(c);
          if (debug_flag)
          {
               char dump[STRSIZE] = "";
               strdump_varfunc(dump, f);
               debug(dump);
          }
          break;

     case I_FUNCDECL_ANSI:
     case I_FUNCDECL_KR:
     case I_FUNCPROTO_ANSI:
     case I_FUNCPROTO_KR:
          f = new_varfunc();
          for (f0 = C->varfunc; f0 && f0->next; f0 = f0->next);
          if (!f0)
               C->varfunc = f;
          else
               f0->next = f;
          f->previous = f0;
          FillNewFunction(f, c);
          f->l0 = lbeg;
          f->l1 = lend;
          delete_cinstruction(c);
          if (debug_flag)
          {
               char dump[STRSIZE] = "";
               strdump_varfunc(dump, f);
               debug(dump);
          }
          break;

     default:
          error(MSG_ERROR_UNEXPECTED_ITYPE, c->Itype);

     }
     return 1;
}
