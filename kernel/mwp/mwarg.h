/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#ifndef MWARG_INC
#define MWARG_INC

extern Node *     mwname;
extern Node *   mwauthor;
extern Node * mwfunction;
extern Node *     mwlabo;
extern Node *    mwusage;
extern Node *    mwgroup;
extern Node *  mwversion;

#define MALLOC_LOC(T) ((T *)malloc(sizeof(T)))

union Paramvalue {
  char           * q;
  char             c;
  unsigned char    uc;
  short            s;
  unsigned short   us;
  int              i;
  unsigned int     ui;
  long             l;
  unsigned long    ul;
  float            f;
  double           d;
};
typedef union Paramvalue Paramvalue;
#define MALLOC_PARAMVALUE MALLOC_LOC(Paramvalue)

#ifdef __STDC__
enum IntervaltypE {
  CLOSED,
  MAX_EXCLUDED,
  MIN_EXCLUDED,
  OPEN
};
typedef enum IntervaltypE Intervaltype;
#else
#define CLOSED       0
#define MAX_EXCLUDED 1
#define MIN_EXCLUDED 2
#define OPEN         3
typedef short Intervaltype;
#endif

struct Interval {
#ifdef __STDC__
  Intervaltype     t;
#else
  short            t;
#endif
  union Paramvalue min;
  union Paramvalue max;
};
#define MALLOC_INTERVAL MALLOC_LOC(struct Interval)

#ifdef __STDC__
enum IO {
  READ,
  WRITE
};
typedef enum IO Io;
#else
#define READ  0
#define WRITE 1
typedef short Io;
#endif

struct FiletypE {
  char * d;
};
typedef struct FiletypE Filetype;

#ifdef __STDC__
enum TypE {
  QSTRING_T,
  CHAR_T,
  UCHAR_T,
  SHORT_T,
  USHORT_T,
  INT_T,
  UINT_T,
  LONG_T,
  ULONG_T,
  FLOAT_T,
  DOUBLE_T,
  MW2_T,
  NONE_T
};
typedef enum TypE Type;
#else
#define QSTRING_T 0
#define CHAR_T    1
#define UCHAR_T   2
#define SHORT_T   3
#define USHORT_T  4
#define INT_T     5
#define UINT_T    6
#define LONG_T    7
#define ULONG_T   8
#define FLOAT_T   9
#define DOUBLE_T  10
#define MW2_T     11
#define NONE_T    12
typedef short Type;
#endif

struct ParamtypE {
#ifdef __STDC__
  Type                t;
#else
  short               t;
#endif
  union  Paramvalue * d;
  struct Interval   * i;
};
typedef struct ParamtypE Paramtype;

#ifdef __STDC__
enum ValtypE {
  FILEARG,
  SCALARARG,
  FLAGARG
};
typedef enum ValtypE Valtype;
#else
#define FILEARG   0
#define SCALARARG 1
#define FLAGARG   2
typedef short Valtype;
#endif

struct IostR {
  char * inc;
  char * def;
  char * init;
  char * op;
};
typedef struct IostR Iostr;
#define MALLOC_IOSTR MALLOC_LOC(Iostr)

struct DesC {
#ifdef __STDC__
  Valtype t;
  Io     rw;
#else
  short   t;
  short  rw;
#endif
  union {
    Filetype  f;
    Paramtype p;
  } v;
};
typedef struct DesC Desc;
#define MALLOC_DESC MALLOC_LOC(Desc)

#ifdef __STDC__
enum ArgtypE {
  OPTION,
  NEEDEDARG,
  VARARG,
  OPTIONARG,
  NOTUSEDARG
};
typedef enum ArgtypE Argtype;
#else
#define OPTION     0
#define NEEDEDARG  1
#define VARARG     2
#define OPTIONARG  3
#define NOTUSEDARG 4
typedef short Argtype;
#endif

struct MwarG {
  char          * name;		/* C identifier name of argument */
  Node          * type;		/* C type of argument */
  Node          * access;	/* C access of argument */
  char          * texname;	/* Usage and TeX name of argument */
  char          * desc;		/* Desccription of argument */
  int             lineno;	/* No of line where is specified the argument */
  char          * filein;	/* Nm of file where is specified the argument */
  struct DataIo * iodesc;	/* Io description pointer */
#ifdef __STDC__
  Argtype t;
#else
  short   t;
#endif
  Iostr   s;
  union {
    struct {
      char o;
      Desc d;
    }    o;
    Desc a;
  }       d;
};

typedef struct MwarG Mwarg;

#ifdef __STDC__
extern void fillmwarg(void);
#else
extern fillmwarg();
#endif

#define IS_PTR(N)        ((N)!=NULL && (N)->name==DEREF && (N)->left==NULL)
#define IS_PTR_OF_PTR(N) ((N)!=NULL && (N)->name==DEREF && (N)->left!=NULL &&\
                          (N)->left->name==DEREF && (N)->left->left==NULL)


#ifdef MWARG_DEC
Header *optionlist, *neededarglist, *vararglist, *optarglist, *notusedarglist;
Mwarg *mwfuncret = NULL;
#else
extern Header *optionlist, *neededarglist, *vararglist, *optarglist, *notusedarglist;
extern Mwarg *mwfuncret;
#endif

/* Prefixes for internal variables */

#define	MW_PFX		"_mw_"
#define	MW_PFX2		"_mw2_"

#endif
