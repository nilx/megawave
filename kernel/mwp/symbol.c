/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#else
#include <malloc.h>
#endif
#include <string.h>

#define SYMBOL_DEC
#include "bintree.h"
#include "symbol.h"
#include "io.h"

#ifdef __STDC__
Symbol *newsymb(char *name, int obj)
#else
Symbol *newsymb(name, obj)
char *name;
int obj;
#endif
{
  Symbol *ret;
  if (ret = MALLOC(Symbol)) {
    char *nm;
    if (nm = (char *)malloc(strlen(name) + 1)) {
      SET_SNAME(ret, strcpy(nm, name));
      SET_OBJ(ret, obj);
      SET_STORAGE(ret, NULL);
      SET_SFATHER(ret, NULL);
      SET_TYPE(ret, NULL);
      SET_ACCESS(ret, NULL);
      SET_SDESC(ret, NULL);
      SET_VALUE(ret, NULL);
    } else
      ret = NULL;
  }
#ifdef DEBUG
  PRDBG("newsymb(%s, %M)->0x%lx\n", name, obj, (unsigned long)ret);
#endif
  return ret;
}


#ifdef __STDC__
void freesymb(Symbol *s)
#else
freesymb(s)
Symbol *s;
#endif
{
   free(s);
}


#ifdef __STDC__
Cell * newcell(void *s)
#else
Cell * newcell(s) 
void *s;
#endif
{
  Cell *ret;
  if (ret = MALLOC(Cell)) {
    SET_ELT(ret, s);
    SET_NEXT(ret, NULL);
    SET_PREV(ret, NULL);
    SET_CHILD(ret, NULL);
    SET_HEADER(ret, NULL);
    SET_MARK(ret, NULL);
  }
  return ret;
}


#ifdef __STDC__
void freecell(Cell *c)
#else
freecell(c)
Cell *c;
#endif
{
  free(c);
}


#ifdef __STDC__
Cell * addcell(Header h[], Cell *c0, Cell *c)
#else
Cell * addcell(h, c0, c)
Header h[];
Cell *c0, *c;
#endif
{
  if (GET_NUMBER(h) == 0) {
    SET_FIRST(h, c);
    SET_LAST(h, c);
    SET_NUMBER(h, 1);
  }
  else {
    if (c0 == NULL) {
      SET_NEXT(c, GET_FIRST(h));
      SET_PREV(GET_NEXT(c), c);
      SET_FIRST(h, c);
      SET_HEADER(c, h);
      SET_NUMBER(h, GET_NUMBER(h) + 1);
    } else if (c0 == h->last) {
      SET_NEXT(c0, c);
      SET_PREV(c, c0);
      SET_NEXT(c, NULL);
      SET_LAST(h, c);
      SET_HEADER(c, h);
      SET_NUMBER(h, GET_NUMBER(h) + 1);
    } else {
      SET_NEXT(c, GET_NEXT(c0));
      SET_PREV(GET_NEXT(c), c);
      SET_PREV(c, c0);
      SET_NEXT(c0, c);
      SET_HEADER(c, h);
      SET_NUMBER(h, GET_NUMBER(h) + 1);
    }
  }
  return c;
}


#ifdef __STDC__
Cell * delcell(Header h[], Cell *c)
#else
Cell * delcell(h, c)
Header h[];
Cell *c;
#endif
{
  if (GET_NUMBER(h) > 0) {
    if (c == GET_FIRST(h)) {
      SET_FIRST(h, GET_NEXT(c));
      SET_PREV(GET_NEXT(c), NULL);
      SET_NUMBER(h, GET_NUMBER(h) - 1);
    } else if (c == GET_LAST(h)) {
      SET_LAST(h, GET_PREV(c));
      SET_NEXT(GET_PREV(c), NULL);
      SET_NUMBER(h, GET_NUMBER(h) - 1);
    } else {
      SET_NEXT(GET_PREV(c), GET_NEXT(c));
      SET_PREV(GET_NEXT(c), GET_PREV(c));
      SET_NUMBER(h, GET_NUMBER(h) - 1);
    }
  }
  return c;
}

#ifdef __STDC__
static int hash(char *s)
#else
static int hash(s)
char *s;
#endif
{
  int ret;
  for (ret = 0; *s != '\0'; s++)
    ret = (ret + (int)*s) % HASH_ENTRY;
  return ret;
}

#ifdef __STDC__
Symbol * lookup(Header tab[], char * name)
#else
Symbol * lookup(tab, name)
Header tab[];
char * name;
#endif
{
  Header *entry;

  entry = &tab[hash(name)];

#ifdef DEBUG
  PRDBG("LOOK FOR \"%s\" in symbols table :\n", name);
#endif

  if (GET_NUMBER(entry) != 0) {
    Cell *i;
    for (i = GET_FIRST(entry); i != NULL; i = GET_NEXT(i)) {
      int n;
      n = strcmp(name, GET_SNAME((Symbol *)GET_ELT(i)));
      if (n < 0) {  /* On suppose que les noms sont classes par ordre alpha */
#ifdef DEBUG
        PRDBG(" -> not found\n");
#endif
        return NULL;
      } else if (n == 0) {
#ifdef DEBUG
        PRDBG(" -> found\n");
#endif
        return (Symbol *)GET_ELT(i);
      }
    }
#ifdef DEBUG
    PRDBG(" -> not found\n");
#endif
    return NULL;
  } else {
#ifdef DEBUG
    PRDBG(" -> not found\n");
#endif
    return NULL;
  }
}

#ifdef __STDC__
Symbol * add(Header tab[], char * name, int obj)
#else
Symbol * add(tab, name, obj)
Header tab[];
char * name;
int obj;
#endif
{
  Header *entry;
  Symbol *s;
#ifdef DEBUG
  extern int yydebug;
#endif

  entry = &tab[hash(name)];
  s = newsymb(name, obj);

  if (GET_NUMBER(entry) == 0) {
    addcell(entry, NULL, newcell(s));
  } else {
    int n;
    Cell *i;
    for (i = GET_FIRST(entry);
         i != NULL && (n=strcmp(name, GET_SNAME((Symbol *)GET_ELT(i)))) >= 0;
         i = GET_NEXT(i)) ;
    if (i == NULL)
      (void) addcell(entry, GET_LAST(entry), newcell(s));
    else if (n < 0) {
      if (GET_FIRST(entry) == i)
        (void) addcell(entry, 0, newcell(s));
      else
        (void) addcell(entry, GET_PREV(i), newcell(s));
    }
  }
#ifdef DEBUG
  PRDBG("ADD in symbols table (\"%s\", %M)\n", name, obj);
#endif
  return s;
}

#ifdef __STDC__
void del(Header tab[], char * name)
#else
del(tab, name)
Header tab[];
char * name;
#endif
{
  Header *entry;

  entry = &tab[hash(name)];

  if (GET_NUMBER(entry) != 0) {
    int n;
    Cell *i;
    for (i = GET_FIRST(entry);
         i != NULL && (n=strcmp(name, GET_SNAME((Symbol *)GET_ELT(i)))) > 0;
         i = GET_NEXT(i));
    if (i != NULL && n == 0)
      delcell(entry, i);
  }
}


#ifdef __STDC__
Header *newlist(void *s)
#else
Header *newlist(s)
void *s;
#endif
{
  Header * ret;
  if ((ret = MALLOC(Header)) != NULL) {
    SET_FIRST(ret, NULL);
    SET_LAST(ret, NULL);
    SET_NUMBER(ret, 0);
    if (s != NULL)
      (void) addcell(ret, NULL, newcell(s));
  }
  return ret;
}

#ifdef __STDC__
Cell * addlist(Header *h, void *s)
#else
Cell * addlist(h, s)
Header *h;
void *s;
#endif
{
#ifdef DEBUG
  PRDBG("addlist(0x%lx, 0x%lx)\n", (unsigned long)h, (unsigned long)s);
#endif
  return addcell(h, GET_LAST(h), newcell(s));
}

static unsigned long internalnameind = 1;


#ifdef __STDC__
char * genname(void)
#else
char * genname()
#endif
{
  char *ret;
  if (internalnameind == 0)
    INT_ERROR("genname");
  ret = (char *) malloc( 8 *sizeof(char));	/* 8 = 1 char + 6 digits + '\0' */
  if (ret == NULL)
    INT_ERROR("genname");
  sprintf(ret, "%c%6.6lu", INTERNAL_NAME_MARK, internalnameind);
  internalnameind++;
  return ret;
}
