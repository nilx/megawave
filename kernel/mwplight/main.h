/*~~~~~~~~~~~~~~ mwplight - The MegaWave2 Light Preprocessor ~~~~~~~~~~~~~~~~~~~

 Include of mwplight

 Author : Jacques Froment
 Date : 2005
 Version : 0.1
 Versions history :
   0.1 (August 2005, JF) initial internal release
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~  This file is part of the MegaWave2 light preprocessor ~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef mwpl_main

#define mwpl_main

/*~~~~~ Constants ~~~~~ */

/* Maximum size of strings for parsing.
   Any strings, lines, sentences,... in the source module must have smaller size.
*/
#define STRSIZE 4096

/* End of header mark */
#define EOH -2

/*~~~~~ Include files ~~~~~ */

#include <sys/types.h>
#include <sys/stat.h>
#include "mwpl_tree.h"

/*~~~~~ Types ~~~~~ */

/*~~~~~ Macros ~~~~~ */

/* To get a debug, define the DEBUG symbol and re-compile all */
/*#define DEBUG*/

#define MESSAGE 0
#define WARNING 1
#define ERROR 2
#define Message(x...) message(MESSAGE,stdout,x)
#define Warning(x...) message(WARNING,stderr,x)
#define Error(x...) message(ERROR,stderr,x)

/* Test for a C id character (= valid char for C variable name) */
#define IsCharCid(x) ( (((x)>='A')&&((x)<='Z')) || (((x)>='a')&&((x)<='z')) \
                    || (((x)>='0'&&((x)<='9'))) || ((x)=='_') )

/*~~~~~ Global variables ~~~~~*/

/* Useful environment variables */
extern char MEGAWAVE2[];
extern char MY_MEGAWAVE2[];
extern char CWD[]; /* Current working Directory as returned by getcwd() or /bin/pwd */
extern char PWD[]; /* Current working Directory as returned by shell built-in pwd */

extern int adm_mode; /* Administrator mode. If set, module are compiled in
		        $MEGAWAVE2. If not, in $MY_MEGAWAVE2.       	*/
extern Header *H; /* The header tree */
extern Cbody *C;  /* The C body tree */
extern FILE *fs; /* file pointer on the source module (NULL is not opened) */
extern struct stat module_fstat; /* info about the module file */
extern FILE *fa; /* file pointer on the argument analysis file 
		    (A-file = source of run-time) */
extern FILE *fm; /* file pointer on the source of modified module to be put in library
		    (M-file) */
extern FILE *ft; /* file pointer on the texfile (T-file) */
extern FILE *fi; /* file pointer on the intfile (I-file) */

extern char Afile[]; /* name of the output A-file */
extern char Mfile[]; /* name of the output M-file */
extern char Tfile[]; /* name of the output T-file */
extern char Ifile[]; /* name of the output I-file */

extern int inside_comment; /* 1 if parsing inside a comment, 0 elsewhere */
extern int inside_header;  /* 0 if inside C body; > 0 if inside header, the number
			      being the ID number of the header.
			   */
extern int inside_optionarg; /* 1 if parsing optional arguments in the usage but the last
				one; 2 if parsing the last one; 0 if not yet inside optional
				arguments; -1 if no more inside but one list was encountered. 
			     */

extern char module_file[]; /* name of the input module file (with path and extension*/
extern char module_name[]; /* name of the input module file (without path or extension) */
extern char group_name[];  /* name of the group (from the current working dir.) */
extern char usagebuf[];    /* buffer storing the module's usage */
extern char protobuf[];    /* buffer storing the module's main function prototype (in K&R format) */

/* mwpl_declaration */
/* C keywords sorted by types */
extern char *Cdatatype[];
extern char *Cmodifier[];
extern char *Cqualifier[];
extern char *Cstorage[];
extern char *Cdeclare[];
extern char *Cotherkey[];
extern char *Cmwdatatype[];

/* Data type defined by the user in the C body */
#define MAXDT 256
#define DTSTRSIZE 64
extern char *(Cuserdatatype[MAXDT]);
extern int Nudt; /* Number of user-defined data type */

/*~~~~~ Function prototyped definition (from gcc -aux-info) ~~~~~*/

#ifdef __STDC__
#include "mwpl_proto.h"
#endif

#endif
