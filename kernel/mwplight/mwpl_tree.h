/*~~~~~~~~~~~~~~ mwplight - The MegaWave2 Light Preprocessor ~~~~~~~~~~~~~~~~~~~

 Include for tree structures

 Author : Jacques Froment
 Date : 2005-2007
 Version : 1.0
 Versions history :
   0.1 (August 2005, JF) initial internal release
   1.0 (May 2007, JF) W_CQUALIFIER and W_CFUNCSPECIFIER added
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~  This file is part of the MegaWave2 light preprocessor ~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef mwpl_tree

#define mwpl_tree

/*~~~~~ Constants ~~~~~ */

/* Maximum size for string used in trees */
#define TREESTRSIZE 256

/* Common for all types */
#define NONE (-1)

/* Argument types */
#define OPTION     0
#define NEEDEDARG  1
#define VARARG     2
#define OPTIONARG  3
#define NOTUSEDARG 4

/* Category of 'scalar' C types (i.e. without explicitly pointers to) */
        /* Not a scalar-printable type */
#define VOID_T    0
        /* for future use */
#define QSTRING_T 1
        /* Any of MegaWave2 internal types */
#define MW2_T     2
        /* User's types (not allowed in main function) */
#define USER_T    3
        /* Here begins SCALAR C types : any of char, short, int, long, float, double, ... (signed or not) */
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
         /* Incomplete types (useful while scanning composite types) */
#define UNSIGNED_T 20

/* IO types */
#define READ  0
#define WRITE 1

/* IC (Interval Checking) types */
#define CLOSED       0
#define MAX_EXCLUDED 1
#define MIN_EXCLUDED 2
#define OPEN         3

/* Word types for Cword */
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

/*~~ Instruction types for Cinstruction ~~*/

#define I_UNDEFINED (-1)
/* Declaration of a new type */
#define I_CDECLARETYPE 0 
/* Declaration or prototype (not yet defined) of a function - enter list of parameters */
#define I_FUNC_IN 1
/* Declaration of a new function - list of parameters scanned -ANSI format recognized */
#define I_FUNCDECL_ANSI 2
/* idem with K&R format recognized */
#define I_FUNCDECL_KR 3
/*  Function prototype - list of parameters scanned - ANSI format recognized */
#define I_FUNCPROTO_ANSI 4
/* idem with K&R format recognized */
#define I_FUNCPROTO_KR 5
/* Declaration of a variable */
#define I_VARDECL 6

/*~~ Variable declaration types in A-file ~~*/

   /* variable is declared as Ftype, without auxiliary variable pointing to.
      Examples :  float finput=0;
                  Cimage Cinput=NULL; 
*/
#define DT_Ftype_alone 0

   /* variable is declared as Ftype, with an auxiliary variable of type Stype pointing to.
      Example :   float _mw2u_c= 1.0;    (auxiliary variable)
                  float * c= & _mw2u_c;
   */
#define DT_Ftype_auxvar 1

   /* variable is declared as Ftype, with an auxiliary variable of type Stype but
      NOT pointing to.
      Example :   float _mw2u_c= 0;    (auxiliary variable)
                  float * c= NULL;
   */
#define DT_Ftype_auxvarnull 2

   /* variable is declared as Ftype and is the return value of the module
      Example :  Cimage demohead1_ret;
    */
#define DT_Ftype_ret 3

   /* variable is declared as Stype.
      Example :   int h=0;  (while h is of type int * in the module's main function) 
    */
#define DT_Stype 4

/*~~~~~ Types ~~~~~ */

/* An argument of a statement */

typedef struct _Arg 
{
  int Atype;   /* Argument type (OPTION,NEEDEDARG,...) */
  int IOtype;  /* I/O type (READ, WRITE) */
  int ICtype;  /* Interval checking type (CLOSED, ...) */
  
  char Flag;              /* Flag of the option ('c') */
  char H_id[TREESTRSIZE];  /* Name of the argument for Human being */ 
  char C_id[TREESTRSIZE];  /* Name of the argument in C body */ 
  char Cmt[TREESTRSIZE];   /* Comment's string */  

  char Val[TREESTRSIZE];  /* Default input value */  
  char Min[TREESTRSIZE];  /* Min value in case of interval checking */  
  char Max[TREESTRSIZE];  /* Min value in case of interval checking */  

  struct _Variable *var;  /* pointer to the corresponding variable 
			     in the main function (VarFunc) */

  struct _Arg *previous;  /* pointer to the previous argument */
  struct _Arg *next;      /* pointer to the next argument */
} Arg;

typedef struct _Header
{
  char Name[TREESTRSIZE];     /* Name of the C module = name of the main function */
  char Author[TREESTRSIZE];   /* List of author(s) */
  char Version[TREESTRSIZE];  /* Version number */    
  char Function[TREESTRSIZE]; /* Explain the function of the module */
  char Labo[TREESTRSIZE];     /* Name of the lab (optional) */
  char Group[TREESTRSIZE];    /* Group name (optional) */
  Arg *usage;                 /* Pointer on the first argument in usage */
  Arg *retmod;                /* Pointer to the argument of the return main function, if any */
  int NbOption;               /* Number of options */
  int NbNeededArg;            /* Number of needed arguments */
  int NbVarArg;               /* Number of variable arguments */
  int NbOptionArg;            /* Number of optional arguments */
  int NbNotUsedArg;           /* Number of notused arguments */
} Header;

/* A variable */

typedef struct _Variable 
{
  char Name[TREESTRSIZE];     /* Name of the variable in C body */ 
  int Ctype;                  /* Category of scalar C type of name (SHORT_T, MW2_T, ...) */   
  char Stype [TREESTRSIZE];   /* Type of the parameter in C body. Examples : unsigned char, Cimage.
			         Beware : pointer is not indicated here (if v is declared by char *v;
				 the type is reported as char and not char *. Use PtrDepth to now
			         that the full type is char *.)
			      */ 
  char Ftype [TREESTRSIZE];   /* (full) Type of the parameter in C body. Examples : unsigned char *, Cimage *. */
  int PtrDepth;               /* Pointer depth (0 if not a pointer, 1 if e.g. char *, 2 if e.g. char **).
				 Beware : do not count when a type is already a pointer (such as a 
				 MegaWave2 structure or a function) : e.g. 0 if Cimage, 1 if Cimage *,...
			      */
  int DeclType;               /* Variable declaration type in A-file. See DT_* constants. */
  char Cstorage[TREESTRSIZE]; /* C storage, if any */

  struct _Arg *arg;           /* Variable of the main function only : 
				 pointer to the corresponding argument in the header */

  struct _Variable *previous;  /* pointer to the previous variable */
  struct _Variable *next;      /* pointer to the next variable */
} Variable;

/* A variable or a function */

typedef struct _VarFunc
{
  int Itype;                  /* instruction type */
  Variable *v;                /* description of the variable or function */ 
  Variable *param;            /* Pointer on the first parameter (NULL if a variable) */
  long l0;                    /* Location of the beginning of the varfunc declaration
				 in the input module file
			      */
  long l1;                    /* Location of the end of the varfunc declaration */
  
  struct _VarFunc *previous;  /* pointer to the previous varfunc */
  struct _VarFunc *next;      /* pointer to the next varfunc */
  
} VarFunc;


/* C body description of the module */

typedef struct _Cbody
{
  VarFunc *varfunc;   /* Pointer on the first variable or function */
  VarFunc *mfunc;     /* Pointer on the module's function */
} Cbody;

/* A C word. Used during parsing of C body while exact meaning is not yet determined.
 */

typedef struct _Cword
{
  char Name[TREESTRSIZE];     /* Name (the word itself) */
  int Wtype;                  /* word type */

  struct _Cword *previous;    /* pointer to the previous Cword */
  struct _Cword *next;        /* pointer to the next Cword */
} Cword;

/*
  A C instruction (until next ;) as obtained during parsing of C body 
*/

typedef struct _Cinstruction
{
  char phrase[STRSIZE]; /* the instruction text */
  int Itype;            /* instruction type */  
  int nparam;           /* number of parameters in the function (-1 if no function) */
  int ndatatype;        /* number of data type in the function (-1 if no function) */
  int nvar;             /* number of declared variables */
  Cword *wfirst;        /* pointer of the first word in the instruction */

  /* The following NOT NULL only if the instruction has been
     falsely separated by getinstruction() and has to be continued.
  */
  struct _Cinstruction *previous; 
  struct _Cinstruction *next;  /* pointer to the next instruction to be continued */
  
} Cinstruction;


/*~~~~~ Macros ~~~~~ */

/* Arg */

/* Say if arg is an option */
#define ISARG_OPTION(x) ((x)->Atype==OPTION)
/* Say if arg is a "FLAGARG", i.e. a flag option */
#define ISARG_FLAGOPT(x) (((x)->Atype==OPTION)&&((x)->H_id[0]=='\0'))
/* Say if arg is a needed argument */
#define ISARG_NEEDED(x) ((x)->Atype==NEEDEDARG)
/* Say if arg is a variable argument */
#define ISARG_VARIABLE(x) ((x)->Atype==VARARG)
/* Say if arg is an optional argument */
#define ISARG_OPTARG(x) ((x)->Atype==OPTIONARG)
/* Say if arg is a notused argument */
#define ISARG_NOTUSED(x) ((x)->Atype==NOTUSEDARG)

/* Say if arg is in input */
#define ISARG_INPUT(x) ((x)->IOtype==READ)
/* Say if arg is in output */
#define ISARG_OUTPUT(x) ((x)->IOtype==WRITE)

/* Say if arg has a default value */
#define ISARG_DEFAULT(x) ((x)->Val[0]!='\0')

/* Say if arg has an interval checking */
#define ISARG_INTERVAL(x) ((x)->ICtype!=NONE)

/* Say if arg is of an implicit pointer type (such as char * or Cimage) : may be set to NULL. 
   Does not work with user's types that are pointer to something.
*/
#define ISARG_IMPLICITPOINTER(x) (((x)->var->Ctype==MW2_T)||((x)->var->Ctype==QSTRING_T)||((x)->var->PtrDepth>0))

/* Say if arg is of an explicit pointer type (such as char ** but not Cimage) : may be set to NULL. 
*/
#define ISARG_EXPLICITPOINTER(x) ((x)->var->PtrDepth>0)

/* Say if arg is a "SCALARARG", i.e. of scalar type (such as float) */
#define ISARG_SCALAR(x) (((x)->var->Ctype>=SCALARMIN_T)&&((x)->var->Ctype<=SCALARMAX_T))

/* Say if arg is a "SCALARARG" and not a flag */
#define ISARG_SCALARNOTFLAG(x) ((ISARG_SCALAR(x)) && (!(ISARG_FLAGOPT(x))))

/* Say if arg is of type pointer to a scalar (such as char *) */
#define ISARG_POINTERSCALAR(x) ((ISARG_SCALAR(x))&&((x)->var->PtrDepth==1))

/* Say if arg is a "FILEARG" (i.e. of MegaWave2 internal type) */
#define ISARG_FILE(x) ((x)->var->Ctype==MW2_T)

/* Say if arg is of type pointer to a MegaWave2 internal type (such as Cimage *) */
#define ISARG_POINTERFILE(x) ((ISARG_FILE(x))&&((x)->var->PtrDepth==1))

/* Say if arg is the return function of the module */
#define ISARG_RETURNFUNC(x) ((x)==H->retmod)


/* Cword */

/* Say if cword is a data type */
#define ISCWORD_TYPE(x) ( ((x)->Wtype==W_DATATYPE)||((x)->Wtype==W_USERDATATYPE)||((x)->Wtype==W_CMWDATATYPE))

/* Say if cword participates to the 'scalar' data type (i.e. not explicitely a pointer) */
#define ISCWORD_SCALARTYPE(x) ( (ISCWORD_TYPE(x)) || ((x)->Wtype== W_CMODIFIER) )

/* Say if cword participates to the full data type */
#define ISCWORD_FULLTYPE(x) ( (ISCWORD_SCALARTYPE(x)) || ((x)->Wtype== W_CPOINTER) )



/* Cinstruction */

/* Say if Cinstruction is a function declaration */
#define ISCI_FUNCDECL(x) ( ((x)->Itype==I_FUNCDECL_ANSI) || ((x)->Itype==I_FUNCDECL_KR))

/* Say if Cinstruction is a function prototype */
#define ISCI_FUNCPROTO(x) ( ((x)->Itype==I_FUNCPROTO_ANSI) || ((x)->Itype==I_FUNCPROTO_KR))

/* Say if Cinstruction is a function */
#define ISCI_FUNCTION(x) ( ISCI_FUNCDECL(x) ||  ISCI_FUNCPROTO(x) )


/*~~~~~ Global variables ~~~~~*/


/*~~~~~ Function prototyped definition (from gcc -aux-info) ~~~~~*/


#endif
