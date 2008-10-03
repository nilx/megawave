/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*****************************************************/
/* Structure de donnees et methodes pour les symbols */
/*****************************************************/

#ifndef SYMBOLS_INC
#define SYMBOLS_INC 1

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif 

struct SymboL {
  char * name;		/* Name of the symbol */
  char * rename;	/* New name of the symbol */
  int    obj;		/* Type of object (see following member "desc") */
  int    storage;	/* Storage of symbol */
  void * father;	/* Father of symbol (depend on usage) */
  void * type;		/* Pointer to type of symbol */
  void * access;	/* How to access to symbol location */
  void * desc;		/* Pointer to symbol description */
  void * value;		/* Pointer to initialisation of symbol */
};

typedef struct SymboL Symbol;

#define SET_SNAME(S, N)   ((S)->name = (char *)(N))
#define GET_SNAME(S)      ((S)->name)

#define SET_RENAME(S, N)  ((S)->rename = (char *)(N))
#define GET_RENAME(S)     ((S)->rename)

#define SET_OBJ(S, O)     ((S)->obj = (int)(O))
#define GET_OBJ(S)        ((S)->obj)

#define SET_STORAGE(S, Q) ((S)->storage = (int)(Q))
#define GET_STORAGE(S)    ((S)->storage)

#define SET_SFATHER(S, F) ((S)->father = (void *)(F))
#define GET_SFATHER(S)    ((S)->father)

#define SET_TYPE(S, T)    ((S)->type = (void *)(T))
#define GET_TYPE(S)       ((S)->type)

#define SET_ACCESS(S, A)  ((S)->access = (void *)(A))
#define GET_ACCESS(S)     ((S)->access)

#define SET_SDESC(S, D)   ((S)->desc = (void *)(D))
#define GET_SDESC(S)      ((S)->desc)

#define SET_VALUE(S, V)   ((S)->value = (void *)(V))
#define GET_VALUE(S)      ((S)->value)

/* Methodes */
#ifdef __STDC__
Symbol *newsymb(char *, int);
void    freesymb(Symbol *);
#else
extern Symbol *newsymb();
extern freesymb();
#endif

/****************************************************************/
/* Structure de donnees et methodes pour les listes de symboles */
/****************************************************************/

/* Structure de donnees */

struct HeadeR {
  struct CelL * first;			/* First cell of the list */
  struct CelL * last;			/* Last cell of the list */
  int           n;			/* Number of cells */
};

struct CelL {
  void *          elt;			/* Pointer to associate element */
  struct HeadeR * child;		/* Pointer to child list */
  struct CelL *   next;			/* Pointer to next cell */
  struct CelL *   prev;			/* Pointer to previous cell */
  struct HeadeR * head;			/* Pointer to head of list */
  int             mark;			/* Mark used in call tree travel */
};

#define SET_FIRST(H, C)        ((H)->first = (C))
#define SET_LAST(H, C)         ((H)->last = (C))
#define SET_NUMBER(H, N)       ((H)->n = (N))
#define GET_FIRST(H)           ((H)->first)
#define GET_LAST(H)            ((H)->last)
#define GET_NUMBER(H)          ((H)->n)

#define SET_CHILD(C, CHILD)    ((C)->child = (CHILD))
#define SET_ELT(C, E)          ((C)->elt = (void *)(E))
#define SET_PREV(C, P)         ((C)->prev = (P))
#define SET_NEXT(C, N)         ((C)->next = (N))
#define SET_HEADER(C, H)       ((C)->head = (H))
#define SET_MARK(C, M)         ((C)->mark = ((int)(M)))
#define GET_CHILD(C)           ((C)->child)
#define GET_ELT(C)             ((C)->elt)
#define GET_PREV(C)            ((C)->prev)
#define GET_NEXT(C)            ((C)->next)
#define GET_HEADER(C)          ((C)->head)
#define GET_MARK(C)            ((C)->mark)

typedef struct CelL   Cell;
typedef struct HeadeR Header;

/* Methodes */
#ifdef __STDC__
Cell * newcell(void *); 
void   freecell(Cell *);
Cell * addcell(Header *, Cell *, Cell *);
Cell * delcell(Header *, Cell *);
#else
extern Cell * newcell(); 
extern freecell();
extern Cell * addcell();
extern Cell * delcell();
#endif

#ifdef __STDC__
char   * genname(void);
Symbol * lookup(Header *, char *);
Symbol * add(Header *, char *, int);
void del(Header *, char *);
#else
extern char   * genname();
extern Symbol * lookup();
extern Symbol * add();
extern del();
#endif

#ifdef __STDC__
Header * newlist(void *);
Cell   * addlist(Header *, void *);
#else
Header * newlist();
Cell   * addlist();
#endif

#ifdef __STDC__
void mkdepend(Node *, Node *);
#else
void mkdepend();
#endif

#define LOOKUP(N)       lookup(symbtab, (N))
#define ADD(N, O)       add(symbtab, (N), (O))
#define DEL(N)          del(symbtab, (N))

#define LOOKUPAGG(N)    lookup(aggtab, (N))
#define ADDAGG(N, O)    add(aggtab, (N), (O))
#define DELAGG(N)       del(aggtab, (N))

#define MALLOC(T)       ((T *)malloc(sizeof(T)))


/*******************************/
/* Symbols table and call tree */
/*******************************/

#define HASH_ENTRY 17

#ifdef SYMBOL_DEC
Header symbtab[HASH_ENTRY];
Header aggtab[HASH_ENTRY];
Header calltree;
#else
extern Header symbtab[];
extern Header aggtab[];
extern Header calltree;
#endif

#define INTERNAL_NAME_MARK '#'
#define isinternalname(S) (*(S) == INTERNAL_NAME_MARK)
#define RENAME -1
#endif

