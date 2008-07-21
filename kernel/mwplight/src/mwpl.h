/*
 * mwpl.h for megawave, section mwplight
 *
 * common header for common definitions
 *
 * author : Jacques Froment <jacques.froment@univ-ubs.fr> (2005)
 * author : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

/* TODO: distribute */

#ifndef _MWPL_H
#define _MWPL_H

#include <sys/types.h>
#include <sys/stat.h>

/*
 * CONSTANTS
 */

/*
 * maximum size of strings for parsing.
 * any strings, lines, sentences,...
 * in the source module must have smaller size.
 */
#define STRSIZE 4096

/* end of header mark */
#define EOH -2

/* number of user-defined types */
#define MAXDT 256
#define DTSTRSIZE 64

/* maximum size for string used in trees */
#define TREESTRSIZE 256

/* common for all types */
#define NONE (-1)

/* argument types */
#define OPTION     0
#define NEEDEDARG  1
#define VARARG     2
#define OPTIONARG  3
#define NOTUSEDARG 4

/*
 * category of 'scalar' C types (i.e. without explicitly pointers to)
 * - VOID_T    : not a scalar-printable type
 * - QSTRING_T : for future use
 * - MW2_T     : any of megawave internal types
 * - USER_T    : user's types (not allowed in main function)
 */
#define VOID_T    0
#define QSTRING_T 1
#define MW2_T     2
#define USER_T    3

/*
 * real C scalar types :
 * any of char, short, int, long, float, double, ...
 * (signed or not)
 */
#define SCALARMIN_T   10
#define CHAR_T    SCALARMIN_T
#define UCHAR_T   11
#define SHORT_T   12
#define USHORT_T  13
#define INT_T     14
#define UINT_T    15
#define LONG_T    16
#define ULONG_T   17
#define FLOAT_T   18
#define DOUBLE_T  19
#define SCALARMAX_T  DOUBLE_T

/* incomplete types (useful while scanning composite types) */
#define UNSIGNED_T 20

/* IO types */
#define READ  0
#define WRITE 1

/* IC (Interval Checking) types */
#define CLOSED       0
#define MAX_EXCLUDED 1
#define MIN_EXCLUDED 2
#define OPEN         3

/* word types for t_token */
#define W_UNDEFINED (-1)
#define W_SEPARATOR 0
#define W_CDECLARETYPE 1
#define W_STRUCT 2
#define W_CSTORAGE 3
#define W_DATATYPE 4
#define W_USERDATATYPE 5
#define W_CMODIFIER 6
#define W_CPOINTER 7
#define W_CMWDATATYPE 8
#define W_NAME 9
#define W_FUNCNAME 10
#define W_FUNCPARAM 11
#define W_CQUALIFIER 12
#define W_CFUNCSPECIFIER 13

/*
 * instruction types
 * for t_statement
 */

#define I_UNDEFINED (-1)

/*
 * declaration of
 * - a new type
 * - a function (may be a prototype) - enter list of parameters
 * - a function (ANSI format) - list of parameters scanned
 * - a function (K&R format) - list of parameters scanned
 * - a function prototype (ANSI format) - list of parameters scanned
 * - a function prototype (K&R format) - list of parameters scanned
 * - a variable
 */
#define I_CDECLARETYPE 0
#define I_FUNC_IN 1
#define I_FUNCDECL_ANSI 2
#define I_FUNCDECL_KR 3
#define I_FUNCPROTO_ANSI 4
#define I_FUNCPROTO_KR 5
#define I_VARDECL 6

/*
 * variable declaration types
 * in A-file
*/

/*
 * variable is declared as Ftype, without auxiliary variable pointing to.
 * - float finput=0;
 * - Cimage Cinput=NULL;
 */
#define DT_Ftype_alone 0

/*
 * variable is declared as Ftype, with an auxiliary variable
 * of type Stype pointing to.
 * - float _mw2u_c= 1.0;    (auxiliary variable)
 * - float * c= & _mw2u_c;
 */
#define DT_Ftype_auxvar 1

/*
 * variable is declared as Ftype, with an auxiliary variable
 * of type Stype but NOT pointing to.
 * - float _mw2u_c= 0;    (auxiliary variable)
 * - float * c= NULL;
 */
#define DT_Ftype_auxvarnull 2

/*
 * variable is declared as Ftype and is the return value of the module
 * - Cimage demohead1_ret;
 */
#define DT_Ftype_ret 3

/*
 * variable is declared as Stype.
 * - int h=0;  (while h is of type int * in the module's main function)
 */
#define DT_Stype 4


/*
 * MACROS
 */

/*
 * t_argument
 */

/*
 * is the arg
 * - an option
 * - a "FLAGARG", i.e. a flag option
 * - a needed argument
 * - a variable argument
 * - an optional argument
 * - a notused argument
 */
#define ISARG_OPTION(x)   ((x)->Atype == OPTION)
#define ISARG_FLAGOPT(x)  (((x)->Atype == OPTION) && ((x)->H_id[0] == '\0'))
#define ISARG_NEEDED(x)   ((x)->Atype == NEEDEDARG)
#define ISARG_VARIABLE(x) ((x)->Atype == VARARG)
#define ISARG_OPTARG(x)   ((x)->Atype == OPTIONARG)
#define ISARG_NOTUSED(x)  ((x)->Atype == NOTUSEDARG)

/*
 * is the arg
 * - an input
 * - an output
 */
#define ISARG_INPUT(x)  ((x)->IOtype == READ)
#define ISARG_OUTPUT(x) ((x)->IOtype == WRITE)

/*
 * does the arg haave
 * - a default value
 * - an interfal checking
 */
#define ISARG_DEFAULT(x)  ((x)->Val[0] != '\0')
#define ISARG_INTERVAL(x) ((x)->ICtype != NONE)

/*
 * is the arg
 * - of an implicit pointer type (such as char * or Cimage)
 *   -> may be set to NULL.
 *   (does not work with user's types that are pointer to something)
 * - of an explicit pointer type (such as char ** but not Cimage)
 *   -> may be set to NULL.
 */
#define ISARG_IMPLICITPOINTER(x) (((x)->var->Ctype == MW2_T) ||     \
				  ((x)->var->Ctype == QSTRING_T) || \
				  ((x)->var->PtrDepth > 0))
#define ISARG_EXPLICITPOINTER(x) ((x)->var->PtrDepth > 0)

/*
 * is the arg
 * - a "SCALARARG", i.e. of scalar type (such as float)
 * - a "SCALARARG" and not a flag
 * - of type pointer to a scalar (such as char *)
 * - a "FILEARG" (i.e. of megawave internal type)
 * - of type pointer to a megawave  internal type (such as Cimage *)
 * - the return function of the module
 */
#define ISARG_SCALAR(x)        (((x)->var->Ctype >= SCALARMIN_T) &&	\
				((x)->var->Ctype <= SCALARMAX_T))
#define ISARG_SCALARNOTFLAG(x) ((ISARG_SCALAR(x)) && (!(ISARG_FLAGOPT(x))))
#define ISARG_POINTERSCALAR(x) ((ISARG_SCALAR(x)) && ((x)->var->PtrDepth == 1))
#define ISARG_FILE(x)          ((x)->var->Ctype == MW2_T)
#define ISARG_POINTERFILE(x)   ((ISARG_FILE(x)) && ((x)->var->PtrDepth == 1))
#define ISARG_RETURNFUNC(x)    ((x) == H->retmod)

/*
 * t_token
 */

/*
 * is the cword a data type
 * does the  cword participate to
 * - the 'scalar' data type (i.e. not explicitely a pointer)
 * - the full data type
 */
#define ISCWORD_TYPE(x)       (((x)->Wtype == W_DATATYPE) ||     \
                               ((x)->Wtype == W_USERDATATYPE) || \
			       ((x)->Wtype == W_CMWDATATYPE))
#define ISCWORD_SCALARTYPE(x) ((ISCWORD_TYPE(x)) || \
			       ((x)->Wtype == W_CMODIFIER))
#define ISCWORD_FULLTYPE(x)   ((ISCWORD_SCALARTYPE(x)) || \
			       ((x)->Wtype == W_CPOINTER) )

/*
 * t_statement
 */

/*
 * is the cinstruction
 * - a function declaration
 * - a function prototype
 * - a function
 */
#define ISCI_FUNCDECL(x)  (((x)->Itype == I_FUNCDECL_ANSI) ||	\
			   ((x)->Itype == I_FUNCDECL_KR))
#define ISCI_FUNCPROTO(x) (((x)->Itype == I_FUNCPROTO_ANSI) ||	\
			   ((x)->Itype == I_FUNCPROTO_KR))
#define ISCI_FUNCTION(x)  (ISCI_FUNCDECL(x) || ISCI_FUNCPROTO(x))


/*
 * CUSTOM TYPES/STRUCTURES
 */

/* TODO : rename */

/* an argument of a statement */
typedef struct s_argument
{
  int Atype;                  /* argument type (OPTION,NEEDEDARG,...)   */
  int IOtype;                 /* I/O type (READ, WRITE)                 */
  int ICtype;                 /* interval checking type (CLOSED, ...)   */

  char Flag;                  /* flag of the option ('c')               */
  char H_id[TREESTRSIZE];     /* name of the argument for Human being   */
  char C_id[TREESTRSIZE];     /* name of the argument in C body         */
  char Cmt[TREESTRSIZE];      /* comment's string                       */

  char Val[TREESTRSIZE];      /* default input value                    */
  char Min[TREESTRSIZE];      /* min value in case of interval checking */
  char Max[TREESTRSIZE];      /* man value in case of interval checking */

  struct s_variable * var;     /* pointer to the corresponding variable
			       * in the main function (t_varfunc)         */

  struct s_argument * previous;     /* pointer to the previous argument       */
  struct s_argument * next;         /* pointer to the next argument           */
} t_argument;

typedef struct s_header
{
  char Name[TREESTRSIZE];     /* name of the C module
			       * = name of the main function            */
  char Author[TREESTRSIZE];   /* list of author(s)                      */
  char Version[TREESTRSIZE];  /* version number                         */
  char Function[TREESTRSIZE]; /* explain the function of the module     */
  char Labo[TREESTRSIZE];     /* name of the lab (optional)             */
  char Group[TREESTRSIZE];    /* group name (optional)                  */
  t_argument *usage;                 /* pointer on the first argument in usage */
  t_argument *retmod;                /* pointer to the argument of
			       * the return main function, if any       */
  int NbOption;               /* number of option                       */
  int NbNeededArg;            /* number of needed arguments             */
  int NbVarArg;               /* number of variable arguments           */
  int NbOptionArg;            /* number of optional arguments           */
  int NbNotUsedArg;           /* number of notused arguments            */
} t_header;

/* a variable */
typedef struct s_variable
{
  char Name[TREESTRSIZE];     /* name of the variable in C body         */
  int Ctype;                  /* category of scalar C type of name
			       * (SHORT_T, MW2_T, ...)                  */
  char Stype [TREESTRSIZE];   /* type of the parameter in C body.
			       * examples : unsigned char, Cimage.
			       * beware : pointer is not indicated here
			       * v(if v is declared by char *v;
			       * the type is reported as char
			       * and not char *.
			       * Use PtrDepth to now
			       * that the full type is char *.)          */
  char Ftype [TREESTRSIZE];   /* (full) Type of the parameter in C body.
			       * examples : unsigned char *, Cimage *.   */
  int PtrDepth;               /* pointer depth (0 if not a pointer, 1
			       * if e.g. char *, 2 if e.g. char **).
			       * beware : do not count when a type is
			       * already a pointer (such as a megawave
			       * structure  or a function) : e.g. 0 if
			       * Cimage, 1 if Cimage *,...               */
  int DeclType;               /* variable declaration type in A-file.
			       * See DT_* constants.                     */
  char Cstorage[TREESTRSIZE]; /* C storage, if any                       */

  struct s_argument * arg;          /* variable of the main function only :
			       * pointer to the corresponding argument
			       * in the header                           */

  struct s_variable * previous; /* pointer to the previous variable       */
  struct s_variable * next;     /* pointer to the next variable           */
} t_variable;

/* a variable or a function */
typedef struct s_varfunc
{
  int Itype;                  /* instruction type                        */
  t_variable * v;               /* description of the variable or function */
  t_variable * param;           /* pointer on the first parameter
			       * (NULL if a variable)                    */
  long l0;                    /* location of the beginning of the
			       * varfunc declaration in the input module
			       * file                                    */
  long l1;                    /* location of the end of the varfunc
			       * declaration                             */

  struct s_varfunc *previous;  /* pointer to the previous varfunc         */
  struct s_varfunc *next;      /* pointer to the next varfunc             */

} t_varfunc;


/* C body description of the module */
typedef struct s_body
{
  t_varfunc * varfunc;          /* Pointer on the first variable or function */
  t_varfunc * mfunc;            /* Pointer on the module's function          */
} t_body;

/*
 * a C word
 * Used during parsing of C body while exact meaning is not yet determined.
 */
typedef struct s_token
{
  char Name[TREESTRSIZE];     /* name (the word itself)                  */
  int Wtype;                  /* word type                               */

  struct s_token * previous;   /* pointer to the previous t_token           */
  struct s_token * next;       /* pointer to the next t_token               */
} t_token;

/* a C instruction (until next ;) as obtained during parsing of C body */
typedef struct s_statement
{
  char phrase[STRSIZE]; /* the instruction text                          */
  int Itype;            /* instruction type                              */
  int nparam;           /* number of parameters in the function
			 * (-1 if no function)                           */
  int ndatatype;        /* number of data type in the function
			 * (-1 if no function)                           */
  int nvar;             /* number of declared variables                  */
  t_token * wfirst;       /* pointer of the first word in the instruction  */

  /*
   * the following NOT NULL only if the instruction has been
   *falsely separated by getinstruction() and has to be continued.
   */
  struct s_statement * previous;
  struct s_statement * next;  /* pointer to the next instruction
				 * to be continued                       */

} t_statement;


/*
 * GLOBAL (EXTERN) VARIABLES
 */

FILE * sfile_global;

unsigned char debug_flag;
t_header * H;
t_body * C;
struct stat module_fstat;

/* 
 * - 1 if parsing inside a comment, 0 elsewhere
 * - 0 if inside C body, > 0 if inside header
 *     (the number being the ID number of the header)
 * - 1 if parsing optional arguments in the usage but the last one;
 *   2 if parsing the last one;
 *   0 if not yet inside optional arguments;
 *   -1 if no more inside but one list was encountered.
 */
int inside_comment;
int inside_header;
int inside_optionarg;

/*
 * names of
 * - the input module file (with path and extension)
 * - the input module file (without path or extension)
 * - the group (from the current working dir.)
 */
char sfile_name[BUFSIZ];
char module_name[BUFSIZ];
char group_name[BUFSIZ];

/*
 * buffers storing
 * - the module's usage
 * - the module's main function prototype (in K&R format)
 */
char usagebuf[STRSIZE];
char protobuf[STRSIZE];

#endif /* !_MWPL_H */
