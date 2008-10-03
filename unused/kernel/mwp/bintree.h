/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef BINTREE_INC
#define BINTREE_INC 1

typedef union {
    unsigned long * integer;
    double *        real;
    char *          character;
    char *          text;
    char *          qstring;
    void *          desc;
  } Val;


struct NodE {
  int name;
  struct NodE *left;
  struct NodE *right;
  struct NodE *father;
  char        *yytext;
  char        *filein;
  int         lineno;
  Val         val;
};

typedef struct NodE Node;

#ifdef __STDC__
Node * mknode(int, char *, int, Node *, Node *);
Node * mkleaf(int, char *, int, void *);
int    nodecmp(Node *, Node *);
int    nodeispresent(Node *, int);
char * getnameintree(Node *);
Node * cpnode(Node *);
void   clrtree(Node *);
#else
Node * mknode();
Node * mkleaf();
int    nodecmp();
int    nodeispresent();
char * getnameintree();
Node * cpnode();
void   clrtree();
#endif

#endif
