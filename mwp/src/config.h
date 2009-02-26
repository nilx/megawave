/**
 * @file config.h
 *
 * common configuration for mwp
 *
 * @author Jacques Froment <jacques.froment@univ-ubs.fr> (2005-2009), \
 *         Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

/* TODO: distribute */

#ifndef _CONFIG_H
#define _CONFIG_H

/*
 * CONSTANTS
 */

/*
 * maximum size of strings for parsing.
 * any strings, lines, sentences,...
 * in the source module must have smaller size.
 */
#define STRSIZE 8192

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

#endif /* !_CONFIG_H */
