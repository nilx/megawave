/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* Fichiers d'include */
#include <stdio.h>
#include "bintree.h"
#include "symbol.h"
#include "token.h"
#include "y.tab.h"
#include "io.h"

#ifdef __STDC__
Node * mknode(int name, char *filein, int lineno, Node *left, Node *right)
#else
Node * mknode(name, filein, lineno, left, right)
int name;
char *filein;
int lineno;
Node *left, *right;
#endif
{
  Node *ret;
  if ((ret = MALLOC(Node)) != NULL) {
    ret->name = name;
    ret->left = left;
    if (left != NULL)
      left->father = ret;
    ret->right = right;
    if (right != NULL)
      right->father = ret;
    ret->filein = filein;
    ret->lineno = lineno;
    ret->father = (Node *)NULL;
    ret->yytext = (char *)NULL;
    ret->val.text = (char *)NULL;
  }
  else
    INT_ERROR("mknode");
#ifdef DEBUG
  PRDBG("mknode(%M, %s, %d, 0x%lx, 0x%lx)->node = 0x%lx\n",
                           ret->name, ret->filein, ret->lineno, ret->left,
                                                    ret->right, (unsigned long)ret);
#endif
  return ret;
}

#ifdef __STDC__
void clrnode(Node *n)
#else
clrnode(n)
Node *n;
#endif
{
  if (n != NULL) {
#ifdef DEBUG
    PRDBG("clrnode(0x%lx)\n", (unsigned long)n);
#endif
    if (n->yytext != NULL)
      free(n->yytext);
    if (n->val.text != NULL)
      free(n->val.text);
    free(n);
  }
#ifdef DEBUG
  else
    PRDBG("clrnode(NULL)\n");
#endif
}


#ifdef __STDC__
Node * mkleaf(int name, char *filein, int lineno, void *val)
#else
Node * mkleaf(name, filein, lineno, val)
int name;
char *filein;
int lineno;
char *val;
#endif
{
  Node *ret;
  if ((ret = MALLOC(Node)) != NULL) {
    ret->name = name;
    ret->left = ret->right = ret->father = (Node *)NULL;
    ret->yytext = (char *)NULL;
    ret->filein = filein;
    ret->lineno = lineno;
    switch(name) {
      case INTEGER   : ret->val.integer   = (unsigned long *)val; break;
      case REAL      : ret->val.real      = (double *)val; break;
      case CHARACTER : ret->val.character = (char *)val; break;
      case NAME      : ret->val.text      = (char *)val; break;
      case QSTRING   : ret->val.qstring   = (char *)val; break;
      default        : ret->val.desc      = (void *)val; break;
    }
  }
  else
    INT_ERROR("mkleaf");
#ifdef DEBUG
  switch(ret->name) {
    case NAME :
    case QSTRING:
      PRDBG("mkleaf(%M = %s, %s, %d)->node = 0x%lx\n",
                             ret->name, ret->val.text, ret->filein, ret->lineno,
                                               (unsigned long)ret);
      break;
    case CHARACTER :
    case INTEGER :
    case REAL :
      if (ret->yytext != NULL)
        PRDBG("mkleaf(%M = %s, %s, %d)->node = 0x%lx\n",
                  ret->name, ret->yytext, ret->filein, ret->lineno,
                                                (unsigned long)ret);
      else
        PRDBG("mkleaf(%M, %s, %d)->node = 0x%lx\n", ret->name,
                            ret->filein, ret->lineno, (unsigned long)ret);
      break;
    default :
      PRDBG("mkleaf(%M, %s, %d)->node = 0x%lx\n", ret->name, ret->filein,
                                       ret->lineno, (unsigned long)ret);
      break;
  }
#endif
  return ret;
}


#ifdef __STDC__
static void clrleaf(Node *n)
#else
static void clrleaf(n)
Node *n;
#endif
{
  if (n != NULL) {
#ifdef DEBUG
    PRDBG("clrleaf(0x%lx)\n", (unsigned long)n);
#endif
    if (n->yytext != NULL)
      free(n->yytext);
    switch(n->name) {
      case INTEGER   :
        if (n->val.integer != NULL)
          free(n->val.integer);
        break;
      case REAL      :
        if (n->val.real != NULL)
          free(n->val.real);
        break;
      case CHARACTER :
        if (n->val.character != NULL)
          free(n->val.character);
        break;
      case NAME      :
        if (n->val.text != NULL)
          free(n->val.text);
        break;
      case QSTRING   :
        if (n->val.qstring != NULL)
          free(n->val.qstring);
        break;
      default        :
        if (n->val.desc != NULL)
          free(n->val.desc);
        break;
    }
    free(n);
  }
#ifdef DEBUG
  else
    PRDBG("clrleaf(NULL)\n");
#endif
}


#ifdef __STDC__
int nodecmp(Node *n1, Node *n2)
#else
int nodecmp(n1, n2)
Node *n1, *n2;
#endif
{
  int ret;
#ifdef DEBUG
  PRDBG("nodecmp(0x%lx, 0x%lx)\n", (unsigned long)n1, (unsigned long)n2);
#endif
  if (n1 == n2 ) {
    ret = TRUE;
#ifdef DEBUG
  PRDBG("nodecmp : n1==n2=0x%lx -> TRUE\n", (unsigned long)n1);
#endif
  } else if ((n1 != NULL && n2 == NULL) || (n1 == NULL && n2 != NULL)) {
    ret = FALSE;
#ifdef DEBUG
  PRDBG("nodecmp : n1=0x%lx && n2=0x%lx -> FALSE\n", (unsigned long)n1,
                                                    (unsigned long)n2);
#endif
  } else if (n1->name != n2->name) {
    ret = FALSE;
#ifdef DEBUG
  PRDBG("nodecmp : n1->name=%M != n2->name=%M -> FALSE\n", n1->name, n2->name);
#endif
  } else {
    ret = nodecmp(n1->left, n2->left) && nodecmp(n1->right, n2->right);
#ifdef DEBUG
  PRDBG("nodecmp : nodecmp(0x%lx, 0x%lx) && nodecmp(0x%lx, 0x%lx) -> %s\n", 
                                    (unsigned long)n1, (unsigned long)n2,
                                    (unsigned long)n1, (unsigned long)n2,
                                    ret ? "TRUE" : "FALSE");
#endif
  }
  return ret;
}


#ifdef __STDC__
int nodeispresent(Node *n, int nm)
#else
int nodeispresent(n, nm)
Node *n;
int nm;
#endif
{
  int flg;
  for(flg = FALSE; n != NULL && flg == FALSE; n = n->left)
    flg = n->name == nm;
  return flg;
}



#ifdef __STDC__
char *getnameintree(Node *n)
#else
char *getnameintree(n)
Node *n;
#endif
{
  char *ret;
  for(ret = NULL; n != NULL && ret == NULL; n = n->left)
    ret = n->name == NAME ? n->val.text : NULL;
  return ret;
}



#ifdef __STDC__
Node *cpnode(Node *n)
#else
Node *cpnode(n)
Node *n;
#endif
{
  Node *ret;
  if (n != NULL) {
    if (n->left == NULL && n->right == NULL)
      ret = mkleaf(n->name, n->filein, n->lineno, &n->val);
    else
      ret = mknode(n->name, n->filein, n->lineno, cpnode(n->left), 
                                                          cpnode(n->right));
  }
  else
    ret = NULL;
  
  return ret;
}


#ifdef __STDC__
void clrtree(Node *n)
#else
void clrtree(n)
Node *n;
#endif
{
  if (n != NULL) {
#ifdef DEBUG
    PRDBG("clrtree(0x%lx)\n", (unsigned long)n);
#endif
    if (n->left == NULL && n->right == NULL)
      clrleaf(n);
    else {
      clrtree(n->left);
      clrtree(n->right);
      clrnode(n);
    }
  }
#ifdef DEBUG
  else
    PRDBG("clrtree(NULL)\n");
#endif
  return;
}
