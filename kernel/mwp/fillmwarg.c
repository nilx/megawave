/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* Fill the internal MegaWave2 data 
   V 1.02

   Main changes :
   V 1.01 (JF 15/07/2002) : more explicit error message when name is missing
   V 1.02 (JF 23/02/2006) : 
     - change in mw_bzero() and memset() calls to conform to Linux 2.6.12 gcc version 4.0.2 
     - Forward declaration of function iotype() removed from local functions.
*/




/* Fichiers d'include */
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef __STDC__

#include <float.h>

#else
/* In <float.h> */
#ifndef FLT_MIN
#define FLT_MIN 1.17549435e-38
#endif
#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38
#endif
#ifndef DBL_MIN
#define DBL_MIN 2.2250738585072014e-308
#endif
#ifndef DBL_MAX
#define DBL_MAX 1.7976931348623157e+308
#endif

#include <malloc.h>

#endif

#include "bintree.h"
#include "symbol.h"
#include "token.h"
#include "io.h"
#define MWARG_DEC
#include "mwarg.h"
#include "data_io.h"
#include "y.tab.h"

struct Ioinfo {
#ifdef __STDC__
  Valtype vt;
  Type     t;
#else
  short vt, t;
#endif
};
#define MALLOC_IOINFO MALLOC_LOC(struct Ioinfo)



#ifdef __STDC__
static void           set_scalar_default(char *, Node *, Paramtype *);
static void *         set_scalar_value(char *, Type, Node *, Paramvalue *);
static void           set_file_default(char *, Node *, Filetype *);
static void           set_interval(char *, Node *, Paramtype *);
#else
static                set_scalar_default();
static char  *         set_scalar_value();
static                set_file_default();
static                set_interval();
#endif

#define mw_bzero(s,n)  memset((s),0,(n))

#ifdef __STDC__
static void set_desc(char *s, struct Ioinfo info, Desc *d, Node *n)
#else
static      set_desc(s, info, d, n)
char          * s;
struct Ioinfo   info;
Desc          * d;
Node          * n;
#endif
{
#ifdef DEBUG
  PRDBG("set_desc(\"%s\", ..., 0x%x, 0x%x)\n", s, d, n);
#endif
  /* Type or i/o dir */
  d->t = info.vt;
  switch(n->name) {
    case RARROW :
#ifdef DEBUG
      PRDBG("set_desc : d->v.rw = READ\n");
#endif
      d->rw = READ;
      break;
    case LARROW :
#ifdef DEBUG
      PRDBG("set_desc : d->v.rw = WRITE\n");
#endif
      d->rw = WRITE;
      break;
    default :
#ifdef DEBUG
      PRDBG("set_desc : d->v.rw = UNKNOWN\n");
#endif
      INT_ERROR("set_desc");
      break;
  }
  switch(d->t) {
    case SCALARARG :
#ifdef DEBUG
      PRDBG("set_desc : SCALARARG\n");
#endif
  /* Type or i/o dir */
      d->v.p.t = info.t;
      break;
    case FLAGARG :
#ifdef DEBUG
      PRDBG("set_desc : FLAGARG\n");
#endif
      d->v.p.t = info.t;
      break;
    case FILEARG :
#ifdef DEBUG
      PRDBG("set_desc : FILEARG\n");
#endif
      break;
    default :
      INT_ERROR("set_desc");
      break;
  }

  /* Default (if needed) */
  if (n->left->name == NAME)
    switch (d->t) {
      case SCALARARG :
#ifdef DEBUG
        PRDBG("set_desc : SCALARARG, d->v.p.d = NULL\n");
#endif
        d->v.p.d = NULL;
        break;
      case FLAGARG :
#ifdef DEBUG
        PRDBG("set_desc : FLAGARG, d->v.p.d = NULL\n");
#endif
        d->v.p.d = NULL;
        break;
      case FILEARG :
#ifdef DEBUG
        PRDBG("set_desc : FILEARG, d->v.f.d = NULL\n");
#endif
        d->v.f.d = NULL;
        break;
      default :
        INT_ERROR("set_desc");
        break;
    }
  else if (n->left->name == '=' && n->left->left != NULL &&
           n->left->left->name == NAME)
    switch (d->t) {
      case SCALARARG :
#ifdef DEBUG
        PRDBG("set_desc : SCALARARG, set_scalar_default(...)\n");
#endif
        set_scalar_default(s, n->left->right, &(d->v.p));
        break;
      case FLAGARG :
#ifdef DEBUG
        PRDBG("set_desc : FLAGARG, set default\n");
#endif
        d->v.p.d = MALLOC_PARAMVALUE;
        mw_bzero((void *) d->v.p.d, sizeof(Paramvalue));
        switch (d->v.p.t) {
          case QSTRING_T :
            d->v.p.d->q = NULL;
            break;
          case CHAR_T :
            d->v.p.d->c = '\0';
            break;
          case UCHAR_T :
            d->v.p.d->uc = '\0';
            break;
          case SHORT_T :
            d->v.p.d->s = 0;
            break;
          case USHORT_T :
            d->v.p.d->us = 0;
            break;
          case INT_T :
            d->v.p.d->i = 0;
            break;
          case UINT_T :
            d->v.p.d->ui = 0;
            break;
          case LONG_T:
            d->v.p.d->l = 0;
            break;
          case ULONG_T :
            d->v.p.d->ul = 0;
            break;
          case FLOAT_T :
            d->v.p.d->f = 0.0;
            break;
          case DOUBLE_T :
            d->v.p.d->d = 0.0;
            break;
          default :
            INT_ERROR("set_desc");
            break;
        }
        break;
      case FILEARG :
#ifdef DEBUG
        PRDBG("set_desc : FILEARG, set_file_default\n");
#endif
        set_file_default(s, n->left->right, &(d->v.f));
        break;
      default :
        INT_ERROR("set_desc");
        break;
    }
  else
    INT_ERROR("set_desc");

  /* Interval (if needed) */
  switch (n->right->right->name) {
    case CLOSED_INTERVAL :
    case MAX_EXCLUDED_INTERVAL :
    case MIN_EXCLUDED_INTERVAL :
    case OPEN_INTERVAL :
      switch (d->t) {
        case SCALARARG :
          set_interval(s, n->right->right, &(d->v.p));
          break;
        case FLAGARG :
          error("'%s' : interval is not allowed for flag argument\n", s);
          break;
        case FILEARG :
          error("'%s' : interval is not allowed for file argument\n", s);
          break;
        default :
#ifdef DEBUG
          PRDBG("set_desc : d->t = %d\n", d->t);
#endif
          INT_ERROR("set_desc");
          break;
      }
      break;
    case QSTRING :
#ifdef GABU
      switch (d->t) {
        case SCALARARG :
          d->v.p.i = NULL;
          break;
        case FILEARG :
          break;
        case FLAGARG :
          break;
        default :
#ifdef DEBUG
          PRDBG("set_desc : d->t = %d\n", d->t);
#endif
          INT_ERROR("set_desc");
          break;
      }
#endif
      break;
    default:
#ifdef DEBUG
      PRDBG("set_desc : n->right->right->name = %d\n", n->right->right->name);
#endif
      INT_ERROR("set_desc");
      break;
  }
#ifdef DEBUG
  PRDBG("set_desc : d = {\n");
  PRDBG("set_desc :   rw = %s\n", d->rw == READ ?
                                        "READ"          :
                                        (d->rw == WRITE ?
                                          "WRITE"           :
                                          "UNKNOWN") );
  switch(d->t) {
    case FILEARG :
  PRDBG("set_desc :   t = FILEARG\n");
  PRDBG("set_desc :   v.f.d = %s\n", d->v.f.d == NULL ? "NULL" : d->v.f.d);
      break;
    case FLAGARG :
  PRDBG("set_desc :   t = FLAGARG\n");
      break;
    case SCALARARG :
  PRDBG("set_desc :   t = SCALARARG\n");
      break;
    default :
  PRDBG("set_desc :   t = UNKNOWN\n");
      break;
  }
  
  PRDBG("set_desc :     }\n");
  PRDBG("set_desc : END\n");
#endif
}


#ifdef __STDC__
static struct Ioinfo *iotype(char *, Node *, Node *, short);
#else
static struct Ioinfo *iotype();
#endif

#ifdef __STDC__
static void set_option(Node *arg, Mwarg *e)
#else
static      set_option(arg, e)
Node *arg;
Mwarg *e;
#endif
{
  Symbol *s;
  struct Ioinfo *info;

#ifdef DEBUG
   PRDBG("set_option(0x%X, 0x%X)\n", arg, e);
#endif

  /* Argtype = OPTION */
  e->t = OPTION;
  /* Name and description of argument */
  switch (arg->right->right->left->name) {
    case NAME :
      e->name = arg->right->right->left->val.text;
      e->desc = arg->right->right->right->val.qstring;
      break;
    case '#' :
      e->name = arg->right->right->left->left->val.text;
      e->desc = arg->right->right->left->right->val.qstring;
      break;
    default :
#ifdef DEBUG
      PRDBG("set_option : arg->right->right->left->name = %d\n", arg->right->right->left->name);
#endif
      INT_ERROR("set_option");
      break;
  }

  /* Verify if C symbol is declared */
  if ((s = LOOKUP(e->name)) != NULL) {
    SET_RENAME(s, GET_SNAME(s));
    e->type = GET_TYPE(s);
    e->access = GET_ACCESS(s);
    SET_SFATHER(s, e);
  }
  else {
    e->type = NULL;
    e->access = NULL;
    fatal_error("[set_option] '%s' is not declared or main function not found\n", e->name);
  }
  /* Option character */
  e->d.o.o = *(arg->left->val.character);
  /* Tex name */
  if (arg->right->left->name == NAME)
    e->texname = arg->right->left->val.text;
  else if (arg->right->left->name == '=' &&
           arg->right->left->left != NULL &&
           arg->right->left->left->name == NAME)
    e->texname = arg->right->left->left->val.text;
  else {
#ifdef DEBUG
    if (arg->right->left->name != '#')
      PRDBG("set_option : arg->right->left->name = %M\n", arg->right->left->name);
    else {
      PRDBG("set_option : arg->right->left->name = '#'\n");
      if (arg->right->left->left == NULL)
        PRDBG("set_option : arg->right->left->left = NULL\n");
      else
        PRDBG("set_option : arg->right->left->left->name = %M\n",
                               arg->right->left->left->name);
    }
#endif
    INT_ERROR("set_option");
  }
  /* Set description */
  if ((info = iotype(e->name, e->type, e->access, 0)) != NULL)
    set_desc(e->name, *info, &(e->d.o.d), arg->right);
  else
    error2(NULL, 0, "'%s' : unknown i/o interface\n", e->name);

#ifdef DEBUG
   PRDBG("set_option : END\n");
#endif
}


#ifdef __STDC__
static void set_flag(Node *arg, Mwarg *e)
#else
static      set_flag(arg, e)
Node *arg;
Mwarg *e;
#endif
{
  Symbol *s;
  struct Ioinfo *info;

#ifdef DEBUG
   PRDBG("set_flag(0x%X, 0x%X)\n", arg, e);
#endif

  /* Argtype = OPTION */
  e->t = OPTION;
  /* Name and description of argument */
  e->name = arg->right->left->val.text;
  e->desc = arg->right->right->val.qstring;

  /* Verify if C symbol is declared */
  if ((s = LOOKUP(e->name)) != NULL) {
    SET_RENAME(s, GET_SNAME(s));
    e->type = GET_TYPE(s);
    e->access = GET_ACCESS(s);
    SET_SFATHER(s, e);
  }
  else {
    e->type = NULL;
    e->access = NULL;
    fatal_error("[set_flag] '%s' is not declared or main function not found\n", 
		e->name);
  }

  /* Option character */
  e->d.o.o = *(arg->left->val.character);

  /* Tex name */
  e->texname = NULL;

  /* Set description */
  if ((info = iotype(e->name, e->type, e->access, 0)) != NULL) {
    e->d.o.d.t = FLAGARG;
    e->d.o.d.v.p.t = info->t;
    e->d.o.d.v.p.d = NULL;
    e->d.o.d.v.p.i = NULL;
  }
  else
    fatal_error("'%s' : unknown i/o interface\n", e->name);

#ifdef DEBUG
   PRDBG("set_flag : END\n");
#endif
}


#ifdef __STDC__
static void set_neededarg(Node *arg, Mwarg *e)
#else
static      set_neededarg(arg, e)
Node *arg;
Mwarg *e;
#endif
{
  Symbol *s;
  struct Ioinfo *info;
#ifdef DEBUG
   PRDBG("set_neededarg(0x%x, 0x%x)\n", arg, e);
#endif
  /* Argtype == NEEDEDARG */
  e->t = NEEDEDARG;

  if (arg->right->name == '#') 
    {
      if (arg->right->left->name == NAME) 
	{
	  e->name = arg->right->left->val.text;
	  e->desc = arg->right->right->val.qstring;
#ifdef DEBUG
	  PRDBG("set_neededarg : e->name = %s\n", e->name);
	  PRDBG("set_neededarg : e->desc = %s\n", e->desc);
#endif
	}
      else 
	if (arg->right->left->name == '#' &&
	    arg->right->left->left->name == NAME) 
	  {
	    e->name = arg->right->left->left->val.text;
	    e->desc = arg->right->left->right->val.qstring;
#ifdef DEBUG
	    PRDBG("set_neededarg : e->name = %s\n", e->name);
	    PRDBG("set_neededarg : e->desc = %s\n", e->desc);
#endif
	  }
	else 
	  {
#ifdef DEBUG
	    if (arg->right->left->name != '#')
	      PRDBG("set_neededarg : arg->right->left->name = %M\n",
		    arg->right->left->name);
	    else 
	      {
		PRDBG("set_neededarg : arg->right->left->name = #\n");
		PRDBG("set_neededarg : arg->right->left->left->name = %M\n",
		      arg->right->left->left->name);
	      }
#endif
	    INT_ERROR("set_neededarg");
	  } 
    }  /* end of  if (arg->right->name == '#') */
  else 
    {
#ifdef DEBUG
      PRDBG("set_neededarg : arg->right->name = %M\n", arg->right->name);
#endif
      INT_ERROR("set_neededarg");
    }

  if ((s = LOOKUP(e->name)) != NULL) 
    {
      SET_RENAME(s, GET_SNAME(s));
      e->type = GET_TYPE(s);
      e->access = GET_ACCESS(s); 
      SET_SFATHER(s, e);
    }
  else 
    {
      e->type = NULL;
      e->access = NULL; 
      fatal_error("[set_neededarg] '%s' is not declared or main function not found\n",
		  e->name);
    }

  if (arg->left->name == NAME)
    e->texname = arg->left->val.text;
  else if (arg->left->name == '=' &&
           arg->left->left != NULL &&
           arg->left->left->name == NAME)
    e->texname = arg->left->left->val.text;
  else {
#ifdef DEBUG
    if (arg->left->name != '=')
      PRDBG("set_neededarg : arg->left->name = %M\n", arg->left->name);
    else {
      PRDBG("set_neededarg : arg->left->name = =");
      if (arg->left->left != NULL)
        PRDBG("set_neededarg : arg->left->left->name = %M\n",
                         arg->left->left->name);
      else
        PRDBG("set_neededarg : arg->left->left = NULL\n", arg->left->left);
    }
    PRDBG("set_neededarg : arg->left->name = %M\n", arg->left->name);
#endif
    INT_ERROR("set_neededarg");
  }

  /* Set description */
  if ((info = iotype(e->name, e->type, e->access, 0)) != NULL)
    set_desc(e->name, *info, &(e->d.a), arg);
  else
    fatal_error("'%s' : unknown i/o interface\n", e->name);

#ifdef DEBUG
  print_mwarg("set_neededarg", e);
  PRDBG("set_neededarg : END\n");
#endif
}


#ifdef __STDC__
static void set_optarg(Node *arg, Mwarg *e)
#else
static      set_optarg(arg, e)
Node *arg;
Mwarg *e;
#endif
{
  Symbol *s;
  struct Ioinfo *info;
#ifdef DEBUG
   PRDBG("set_optarg(0x%X, 0x%X)\n", arg, e);
#endif
  /* Argtype == OPTIONARG */
  e->t = OPTIONARG;

  if (arg->right->name == '#') {
    if (arg->right->left->name == NAME) {
      e->name = arg->right->left->val.text;
      e->desc = arg->right->right->val.qstring;
    }
    else if (arg->right->left->name == '#' &&
             arg->right->left->left->name == NAME) {
      e->name = arg->right->left->left->val.text;
      e->desc = arg->right->left->right->val.qstring;
    }
    else {
#ifdef DEBUG
      if (arg->right->left->name != '#')
        PRDBG("set_optarg : arg->right->left->name = %M\n",
                        arg->right->left->name);
      else {
        PRDBG("set_optarg : arg->right->left->name = #\n");
        PRDBG("set_optarg : arg->right->left->left->name = %M\n",
                    arg->right->left->left->name);
      }
#endif
      INT_ERROR("set_optarg");
    } 
  }
  else {
#ifdef DEBUG
    PRDBG("set_optarg : arg->right->name = %M\n", arg->right->name);
#endif
    INT_ERROR("set_optarg");
  }

  if ((s = LOOKUP(e->name)) != NULL) {
    SET_RENAME(s, GET_SNAME(s));
    e->type = GET_TYPE(s);
    e->access = GET_ACCESS(s); 
    SET_SFATHER(s, e);
  }
  else {
    e->type = NULL;
    e->access = NULL; 
    fatal_error("'[set_optarg] %s' is not declared or main function not found\n",
		e->name);
  }

  if (arg->left->name == NAME)
    e->texname = arg->left->val.text;
  else if (arg->left->name == '=' &&
           arg->left->left != NULL &&
           arg->left->left->name == NAME)
    e->texname = arg->left->left->val.text;
  else {
#ifdef DEBUG
    if (arg->left->name != '=')
      PRDBG("set_optarg : arg->left->name = %M\n", arg->left->name);
    else {
      PRDBG("set_optarg : arg->left->name = =");
      if (arg->left->left != NULL)
        PRDBG("set_optarg : arg->left->left->name = %M\n",
                         arg->left->left->name);
      else
        PRDBG("set_optarg : arg->left->left = NULL\n", arg->left->left);
    }
    PRDBG("set_optarg : arg->left->name = %M\n", arg->left->name);
#endif
    INT_ERROR("set_optarg");
  }

  /* Set description */
  if ((info = iotype(e->name, e->type, e->access, 0)) != NULL)
    set_desc(e->name, *info, &(e->d.a), arg);
  else
    fatal_error("'%s' : unknown i/o interface\n", e->name);

#ifdef DEBUG
   PRDBG("set_optarg : END\n");
#endif
}


#ifdef __STDC__
static void set_vararg(Node *arg, Mwarg *e)
#else
static      set_vararg(arg, e)
Node *arg;
Mwarg *e;
#endif
{
  Symbol *s;
  struct Ioinfo *info;

  /* Argtype = VARARG */
  e->t = VARARG;
  /* Name and description of argument */
  if (arg->right->name == '#' && arg->right->right->name == QSTRING) {
    e->name = arg->right->left->val.text;
    e->desc = arg->right->right->val.qstring;
  }
  else if (arg->right->name == '#' &&
           (arg->right->right->name == CLOSED_INTERVAL ||
            arg->right->right->name == MAX_EXCLUDED_INTERVAL ||
            arg->right->right->name == MIN_EXCLUDED_INTERVAL ||
            arg->right->right->name == OPEN_INTERVAL)) {
    e->name = arg->right->left->left->val.text;
    e->desc = arg->right->left->right->val.qstring;
  }
  else
    INT_ERROR("set_vararg");
  /* Verify if C symbol is declared */
  if ((s = LOOKUP(e->name)) != NULL) {
    SET_RENAME(s, GET_SNAME(s));
    e->type = GET_TYPE(s);
    e->access = GET_ACCESS(s);
    SET_SFATHER(s, e);
  }
  else {
    e->type = NULL;
    e->access = NULL;
    fatal_error("[set_vararg] '%s' is not declared or main function not found\n", 
		e->name);
  }
  /* TeX name */
  e->texname = "...";
  /* Set description */
  if ((info = iotype(e->name, e->type, e->access, 0)) != NULL) {
    e->d.a.t = info->vt;
    switch(arg->name) {
      case RARROW :
        e->d.a.rw = READ;
        break;
      case LARROW :
        e->d.a.rw = WRITE;
        break;
      default :
        INT_ERROR("set_vararg");
        break;
    }
    switch (e->d.a.t) {
      case FILEARG:
#ifdef DEBUG
        PRDBG("set_vararg : FILEARG, e->d.a.v.f.d = NULL\n");
#endif
        e->d.a.v.f.d  = NULL;
        break;
      case SCALARARG :
        switch(arg->name) {
          case RARROW :
            e->d.a.v.p.t = info->t;
            e->d.a.v.p.d = NULL;
            switch(arg->right->right->name){
              case CLOSED_INTERVAL :
              case MAX_EXCLUDED_INTERVAL :
              case MIN_EXCLUDED_INTERVAL :
              case OPEN_INTERVAL :
                set_interval(e->name, arg->right->right, &(e->d.a.v.p));
                break;
            }
            break;
          case LARROW :
            error("'%s' : you can't output constant\n", e->name);
            break;
          default :
            INT_ERROR("set_vararg");
            break;
        }
        break;
      default :
        INT_ERROR("set_vararg");
        break;
    }
  }
  else
    fatal_error("'%s' : unknown i/o interface\n", e->name);
}

#ifdef __STDC__
static void set_notusedarg(Node *arg, Mwarg *e)
#else
static      set_notusedarg(arg, e)
Node *arg;
Mwarg *e;
#endif
{
  Symbol *s;
  struct Ioinfo *info;

  /* Argtype = NOTUSEDARG */
  e->t = NOTUSEDARG;
  /* Name and description of argument */
  e->name = arg->right->left->val.text;
  e->desc = arg->right->right->val.qstring;
  /* Verify if C symbol is declared */
  if ((s = LOOKUP(e->name)) != NULL) {
    SET_RENAME(s, GET_SNAME(s));
    e->type = GET_TYPE(s);
    e->access = GET_ACCESS(s);
    SET_SFATHER(s, e);
  }
  else {
    e->type = NULL;
    e->access = NULL;
    fatal_error("[set_notusedarg] '%s' is not declared or main function not found\n",
		e->name);
  }
  /* TeX name */
  e->texname = e->name;
}


#ifdef __STDC__
void initmwarg_iodesc(Mwarg *);
#else
initmwarg_iodesc();
#endif


#ifdef __STDC__
static void set_mwfunc(Node *arg, Mwarg *e)
#else
static      set_mwfunc(arg, e)
Node *arg;
Mwarg *e;
#endif
{
  Symbol *s;
  char buffer[BUFSIZ];
  e->t = NEEDEDARG;
  sprintf(buffer, "%s_ret", mwname->val.text);
  if ((e->name = malloc((strlen(buffer)+1)*sizeof(char)))!= NULL) {
    strcpy(e->name, buffer);
  }
  else
    INT_ERROR("set_mwfunc");
  if ((s = LOOKUP(mwname->val.text)) != NULL) {
    e->type = GET_TYPE(s);
    e->access = /*GET_ACCESS(s)*/ NULL;
  }
  else
    INT_ERROR("set_mwfunc");
  e->d.a.rw = WRITE;
  if (arg->left->name == NAME) {
    e->texname = arg->left->val.text;
#ifdef DEBUG
    PRDBG("set_mwfunc : e->d.a.v.f.d = NULL\n");
#endif
    e->d.a.v.f.d = (char *) NULL;
  }
  else {
    arg = arg->left;
    e->texname = arg->left->val.text;
    e->d.a.v.f.d = arg->right->val.qstring;
#ifdef DEBUG
    PRDBG("set_mwfunc : arg->right->val.qstring = %s\n", arg->right->val.qstring);
#endif
  }
  mwfuncret = e;
}


#ifdef __STDC__
static void troptarg(Node *n, Header *poptarglist)
#else
static void troptarg(n, poptarglist)
Node *n;
Header *poptarglist;
#endif
{
  Mwarg *e;
  Node *name;

  e = (Mwarg *)malloc(sizeof(Mwarg));
  mw_bzero((void *)e, sizeof(Mwarg));
  switch(n->name) {
    case RARROW :
    case LARROW :
#ifdef DEBUG
      PRDBG("troptarg : case %M\n", n->name);
#endif
      if (n->left->name == NAME)
        name = n->left;
      else if (n->left->name == '=' &&
               n->left->left != NULL &&
               n->left->left->name == NAME)
        name = n->left->left;
      else {
#ifdef DEBUG
        PRDBG("troptarg : n->left->name = %M\n", n->left->name);
#endif
        INT_ERROR("troptarg");
      }
      if (strcmp(name->val.text, mwname->val.text))
        set_optarg(n, e);
      else
        error("MegaWave function name \"%s\" cannot be used in optional argument\n",
                                                             mwname->val.text);
      /* Init of IO description */
      initmwarg_iodesc(e);
#ifdef DEBUG
      print_mwarg("troptarg", e);
#endif
      break;

    default :
#ifdef DEBUG
      PRDBG("troptarg : default %M\n", n->name);
#endif
      INT_ERROR("troptarg");
      break;
  }
  e->lineno = 0;
  e->filein = "";
  addlist(poptarglist, (void *)e);
}


#ifdef __STDC__
static void troptarglist(Node *n, Header *poptarglist)
#else
static void troptarglist(n, poptarglist)
Node *n;
Header *poptarglist;
#endif
{
  for (; n->name == ','; n = n->right)
    troptarg(n->left, poptarglist);
  troptarg(n, poptarglist);
}


#ifdef __STDC__
void initmwarg_iodesc(Mwarg *e)
#else
initmwarg_iodesc(e)
Mwarg *e;
#endif
{
#ifdef __STDC__
  Io rw;
#else
  short rw;
#endif
  Cell *c;
#ifdef DEBUG
  PRDBG("initmwarg_iodesc(e = 0x%x)\n", (unsigned long)e);
#endif
  switch(e->t) {
    case OPTION:
       rw = e->d.o.d.rw;
      switch(e->d.o.d.t) {
        case FILEARG :
        case SCALARARG :
          break;
        case FLAGARG :
#ifdef DEBUG
          PRDBG("initmwarg_iodesc : no io function for flag option\n");
#endif
          e->iodesc = NULL;
          return;
          break;
        default :
#ifdef DEBUG
          PRDBG("initmwarg_iodesc : Valtype unknown\n");
#endif
          INT_ERROR("initmwarg_iodesc");
          break;
      }
      break;
    case NEEDEDARG:
    case VARARG :
    case OPTIONARG :
      rw = e->d.a.rw;
      switch(e->d.a.t) {
        case FILEARG :
        case SCALARARG :
          break;
        case FLAGARG :
#ifdef DEBUG
          PRDBG("initmwarg_iodesc : no flag are allowed in %s\n",
                e->t==NEEDEDARG ? "NEEDEDARG" : (e->t==VARARG ? "VARARG":"OPTIONARG"));
#endif
          e->iodesc = NULL;
          return;
          break;
        default :
#ifdef DEBUG
          PRDBG("initmwarg_iodesc : Valtype unknown\n");
#endif
          INT_ERROR("initmwarg_iodesc");
          break;
      }
      break;
    case NOTUSEDARG :
      break;
  }
  if (e->t != NOTUSEDARG) {
    if ((c = (Cell *)LOOKUP_DATA_IO(e, &rw)) != NULL) {
      if((e->iodesc = (DataIo *)GET_ELT(c)) == NULL) {
#ifdef DEBUG
        PRDBG("initmwarg_iodesc : io type unknown\n");
#endif
        INT_ERROR("initmwarg_iodesc");
      }
    }
    else {
#ifdef DEBUG
      PRDBG("initmwarg_iodesc : io type unknown\n");
#endif
      INT_ERROR("initmwarg_iodesc");
    }
  }
  else
    e->iodesc = NULL;
#ifdef DEBUG
  PRDBG("initmwarg_iodesc : %s, e->iodesc = 0x%x\n", e->name, (unsigned long)e->iodesc);
#endif
}


#ifdef __STDC__
void trarg(Node *arg, Header *poptionlist, Header *pneededarglist, Header *pvararglist,
           Header *poptarglist, Header *pnotusedarglist)
#else
trarg(arg, poptionlist, pneededarglist, pvararglist, poptarglist, pnotusedarglist)
Node *arg;
Header *poptionlist, *pneededarglist, *pvararglist, *poptarglist, *pnotusedarglist;
#endif
{
  Mwarg *e;

  switch(arg->name) {
    case ':' :
#ifdef DEBUG
      PRDBG("trarg : case %M\n", arg->name);
#endif
      e = (Mwarg *)malloc(sizeof(Mwarg)); mw_bzero((void *)e, sizeof(Mwarg));
      e->lineno = 0;
      e->filein = "";
      e->iodesc = NULL;
#ifdef DEBUG
      PRDBG("trarg : set_option 1\n");
#endif
      set_option(arg, e);
      addlist(poptionlist, (void *)e);
      /* Init of IO description */
      initmwarg_iodesc(e);
#ifdef DEBUG
      print_mwarg("trarg", e);
#endif
      break;

    case RARROW :
#ifdef DEBUG
      PRDBG("trarg : case %M\n", arg->name);
#endif
      e = (Mwarg *)malloc(sizeof(Mwarg));
      mw_bzero((void*)e, sizeof(Mwarg));
      e->lineno = 0;
      e->filein = "";
      e->iodesc = NULL;
      if (arg->left->name == CHARACTER) {
        set_flag(arg, e);
        addlist(poptionlist, (void *)e);
      }
      else if (arg->left->name == NAME) {
        Node *n;
        if (arg->right->name == '#') {
          if (arg->right->left->name == NAME)
            n = arg->right->left;
          else if (arg->right->left->name == '#')
            n = arg->right->left->left;
          else
            INT_ERROR("trarg");
        }
        else
          INT_ERROR("trarg");

        if (strcmp(n->val.text, mwname->val.text)) {
          set_neededarg(arg, e);
          addlist(pneededarglist, (void *)e);
        }
        else
          error("MegaWave function name \"%s\" cannot be used as input argument\n",
                                                             mwname->val.text);
      }
      else if (arg->left->name == ELLIPSIS) {
        set_vararg(arg, e);
        addlist(pvararglist, (void *)e);
      }
      else if (arg->left->name == NOTUSED) {
        set_notusedarg(arg, e);
        addlist(pnotusedarglist, (void *)e);
      }
      else
        INT_ERROR("trarg");
      /* Init of IO description */
      initmwarg_iodesc(e);
#ifdef DEBUG
      print_mwarg("trarg", e);
#endif
      break;

    case LARROW :
#ifdef DEBUG
      PRDBG("trarg : case %M\n", arg->name);
#endif
      e = (Mwarg *)malloc(sizeof(Mwarg));
      mw_bzero((void*)e, sizeof(Mwarg));
      e->lineno = 0;
      e->filein = "";
      e->iodesc = NULL;
      if (arg->left->name == CHARACTER) {
        INT_ERROR("trarg");
      }
      else if (arg->left->name == NAME) {
        Node *n;
        if (arg->right->name == '#') {
          if (arg->right->left->name == NAME)
            n = arg->right->left;
          else if (arg->right->left->name == '#')
            n = arg->right->left->left;
          else
            INT_ERROR("trarg");
        }
        else
          INT_ERROR("trarg");

        if (strcmp(n->val.text, mwname->val.text)) {
          set_neededarg(arg, e);
          addlist(pneededarglist, (void *)e);
        }
        else {
          if (mwfuncret == NULL) {
            /*set_mwfunc(arg, e);*/
            set_neededarg(arg, e);
            mwfuncret = e;
            addlist(pneededarglist, (void *)e);
          }
          else
            error("Megawave function name \"%s\" is used more than one time to return result\n", mwname->val.text);
        }
      }
      else if (arg->left->name == ELLIPSIS) {
        set_vararg(arg, e);
        addlist(pvararglist, (void *)e);
      }
      else
        INT_ERROR("trarg");
      /* Init of IO description */
      initmwarg_iodesc(e);
#ifdef DEBUG
      print_mwarg("trarg", e);
#endif
      break;

    case BLOC :
#ifdef DEBUG
      PRDBG("trarg : case %M\n", arg->name);
#endif
      if (GET_NUMBER(poptarglist) == 0)
        troptarglist(arg->right, poptarglist);
      else
        INT_ERROR("trarg");
      break;

    default :
#ifdef DEBUG
      PRDBG("trarg : default %M\n", arg->name);
#endif
      INT_ERROR("trarg");
      break;
  }
}


static short iotab [10][24] = {
/*              0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23*/
/* TYPEDEF_A*/{ 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
/*    CHAR_A*/{ 2, -1, -1, -1, -1, -1, -1, -1, 12, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
/*   SHORT_A*/{ 3, -1, -1, -1, -1, -1, -1, -1, 13, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
/*     INT_A*/{ 4, -1, -1, 10, -1, 11, -1, -1, 14, 18, -1, -1, -1, 20, -1, 21, -1, 22, -1, 23, -1, -1, -1, -1},
/*    LONG_A*/{ 5, -1, -1, -1, -1, -1, -1, -1, 15, 19, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
/*   FLOAT_A*/{ 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
/*  DOUBLE_A*/{ 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
/*  SIGNED_A*/{ 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
/*UNSIGNED_A*/{ 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
/*   ERROR_A*/{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
};
static short state = 0;
#ifdef __STDC__
enum Ioentry {
  TYPEDEF_A = 0,
     CHAR_A,
    SHORT_A,
      INT_A,
     LONG_A,
    FLOAT_A,
   DOUBLE_A,
   SIGNED_A,
 UNSIGNED_A,
    ERROR_A
};
typedef enum Ioentry Ioentry;
#else
#define  TYPEDEF_A  0
#define     CHAR_A  1
#define    SHORT_A  2
#define      INT_A  3
#define     LONG_A  4
#define    FLOAT_A  5
#define   DOUBLE_A  6
#define   SIGNED_A  7
#define UNSIGNED_A  8
#define    ERROR_A  9
#endif

#ifdef __STDC__
static struct Ioinfo *iotype(char *name, Node *type, Node *access, short state)
#else
static struct Ioinfo *iotype(name, type, access, state)
char *name;

/* Node *type, access;  Modif JF 21/3/94 */
Node *type, *access;

short state;
#endif
{
  Node *n;
  Symbol *s;
  struct Ioinfo *ret;

#ifdef DEBUG
  PRDBG("iotype(\"%s\", %N, %N, %d)\n", name, type, access, (int)state);
#endif

  for(n = type; n != NULL && state >= 0; n = n->right) {
    Node *node;
#ifdef DEBUG
    PRDBG("iotype : state    = %d\n", state);
    PRDBG("iotype : n->name  = %M\n", n->name);
#endif
    if (n->name == '#')
      node = n->left;
    else
      node = n;
#ifdef DEBUG
    PRDBG("iotype : node     = %N\n", node);
#endif
    switch (node->name) {
      case USRTYPID :
        if ((s = LOOKUP(n->val.text)) != NULL) {
          if (LOOKUP_DATA_IO_FOR_NODE(n, access, NULL) != NULL)
            state = iotab[TYPEDEF_A][state];
          else {
            Node *newaccess, *n;
            newaccess = cpnode(GET_ACCESS(s));
            for (n = newaccess; n->left != NULL; n = n->left);
            n->left = cpnode(access);
            if ((ret = iotype(name, GET_TYPE(s), newaccess, state)) == NULL)
              error2(NULL, 0, "'%s' : unknown i/o interface\n", n->val.text);
            clrnode(newaccess);
          }
        }
        else
          INT_ERROR("iotype");
        break;
      case CHAR :
        state = iotab[CHAR_A][state];
        break;
      case SHORT :
        state = iotab[SHORT_A][state];
        break;
      case INT :
        state = iotab[INT_A][state];
        break;
      case LONG :
        state = iotab[LONG_A][state];
        break;
      case FLOAT :
        state = iotab[FLOAT_A][state];
        break;
      case DOUBLE :
        state = iotab[DOUBLE_A][state];
        break;
      case SIGNED :
        state = iotab[SIGNED_A][state];
        break;
      case UNSIGNED :
        state = iotab[UNSIGNED_A][state];
        break;
      default :
        state = iotab[ERROR_A][state];
        break;
    }
  }

#ifdef DEBUG
  PRDBG("iotype : final state = %d\n", state);
#endif

  if (state < 0)
    ret = NULL;
  else {
    if ((ret = MALLOC_IOINFO) != NULL) {
      mw_bzero((void *) ret, sizeof(struct Ioinfo));
      switch (state) {
        case  1 :
#ifdef DEBUG
          PRDBG("iotype : FILEARG\n");
#endif
          ret->vt = FILEARG;
          ret->t  = NONE_T;
          break;
        case  2 :		/* CHAR */
        case 12 :		/* SIGNED CHAR */
#ifdef DEBUG
          PRDBG("iotype : SCALARARG [CHAR_T]\n");
#endif
          ret->vt    = SCALARARG;

#ifdef __STDC__
          if (access == NULL)
#else
          if ((void *) access == NULL)
#endif
            ret->t   = CHAR_T;
          else if (IS_PTR(access))
            ret->t   = QSTRING_T;
          else
            INT_ERROR("iotype");
          break;
        case  3 :		/* SHORT */
        case 10 :		/* SHORT INT */
        case 13 :		/* SIGNED SHORT */
        case 20 :		/* SIGNED SHORT INT */
#ifdef DEBUG
          PRDBG("iotype : SCALARARG [SHORT_T]\n");
#endif
          ret->vt    = SCALARARG;
          ret->t     = SHORT_T;
          break;
        case  4 :		/* INT */
        case  8 :		/* SIGNED */
        case 14 :		/* SIGNED INT */
#ifdef DEBUG
          PRDBG("iotype : SCALARARG [INT_T]\n");
#endif
          ret->vt    = SCALARARG;
          ret->t     = INT_T;
          break;
        case  5 :		/* LONG */
        case 11 :		/* LONG INT */
        case 15 :		/* SIGNED LONG */
        case 21 :		/* SIGNED LONG INT */
#ifdef DEBUG
          PRDBG("iotype : SCALARARG [LONG_T]\n");
#endif
          ret->vt    = SCALARARG;
          ret->t     = LONG_T;
          break;
        case  6 :		/* FLOAT */
#ifdef DEBUG
          PRDBG("iotype : SCALARARG [FLOAT_T]\n");
#endif
          ret->vt    = SCALARARG;
          ret->t     = FLOAT_T;
          break;
        case  7 :		/* DOUBLE */
#ifdef DEBUG
          PRDBG("iotype : SCALARARG [DOUBLE_T]\n");
#endif
          ret->vt    = SCALARARG;
          ret->t     = DOUBLE_T;
          break;
        case  9 :		/* UNSIGNED */
        case 18 :		/* UNSIGNED INT */
#ifdef DEBUG
          PRDBG("iotype : SCALARARG [UINT_T]\n");
#endif
          ret->vt    = SCALARARG;
          ret->t     = UINT_T;
          break;
        case 16 :		/* UNSIGNED CHAR */
#ifdef DEBUG
          PRDBG("iotype : SCALARARG [UCHAR_T]\n");
#endif
          ret->vt    = SCALARARG;
          ret->t     = UCHAR_T;
          break;
        case 17 :		/* UNSIGNED SHORT */
        case 22 :		/* UNSIGNED SHORT INT */
#ifdef DEBUG
          PRDBG("iotype : SCALARARG [USHORT_T]\n");
#endif
          ret->vt    = SCALARARG;
          ret->t     = USHORT_T;
          break;
        case 19 :		/* UNSIGNED LONG */
        case 23 :		/* UNSIGNED LONG INT */
#ifdef DEBUG
          PRDBG("iotype : SCALARARG [ULONG_T]\n");
#endif
          ret->vt    = SCALARARG;
          ret->t     = ULONG_T;
          break;
        default :
#ifdef DEBUG
          PRDBG("iotype : unknown state\n");
#endif
          free(ret);
          INT_ERROR("iotype");
          break;
      }
    }
    else {
      error2(NULL, 0, "Not enought memory\n");
    }
  }
  return ret;
}


#ifdef __STDC__
static void * in_range(Type t, Node *n, short msg_flg, char *s)
#else
static char * in_range(t, n, msg_flg, s)
short t;
Node *n;
short msg_flg;
char *s;
#endif
{
  switch (n->name) {
    case QSTRING :
      switch (t) {
        case QSTRING_T :
#ifdef __STDC__
          return (void *)n->val.qstring;
#else
          return (char *)n->val.qstring;
#endif
          break;
        case CHAR_T :
        case UCHAR_T :
        case SHORT_T :
        case USHORT_T :
        case INT_T :
        case UINT_T :
        case LONG_T :
        case ULONG_T :
        case FLOAT_T :
        case DOUBLE_T :
          if (msg_flg) error2(NULL, 0,
    "'%s' default : You can't put quoted string into numerical variable\n", s);
          return NULL;
          break;
        default :
          INT_ERROR("in_range");
          break;
      }
      break;
    case INTEGER :
      switch (t) {
        case QSTRING_T :
          if (msg_flg) error2(NULL, 0,
    "'%s' default : You can't put integer into quoted string variable\n", s);
          return NULL;
          break;
        case CHAR_T :
          if ((long)*(n->val.integer) >= CHAR_MIN &&
              (long)*(n->val.integer) <= CHAR_MAX)
#ifdef __STDC__
            return (void *)n->val.integer;
#else
            return (char *)n->val.integer;
#endif
          else {
            if (msg_flg) error2(NULL, 0,
    "'%s' default : constant is out of range for character\n", s);
            return NULL;
          }
          break;
        case UCHAR_T :
          if ((unsigned long)*(n->val.integer) >= 0 &&
              (unsigned long)*(n->val.integer) <= UCHAR_MAX)
#ifdef __STDC__
            return (void *)n->val.integer;
#else
            return (char *)n->val.integer;
#endif
          else {
            if (msg_flg) error2(NULL, 0,
    "'%s' default : constant is out of range for unsigned character\n", s);
            return NULL;
          }
          break;
        case SHORT_T :
          if ((long)*(n->val.integer) >= SHRT_MIN &&
              (long)*(n->val.integer) <= SHRT_MAX)
#ifdef __STDC__
            return (void *)n->val.integer;
#else
            return (char *)n->val.integer;
#endif
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for short integer\n", s);
            return NULL;
          }
          break;
        case USHORT_T :
          if ((unsigned long)*(n->val.integer) >= 0 &&
              (unsigned long)*(n->val.integer) <= USHRT_MAX)
#ifdef __STDC__
            return (void *)n->val.integer;
#else
            return (char *)n->val.integer;
#endif
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for unsigned short integer\n", s);
            return NULL;
          }
          break;
        case INT_T :
          if ((long)*(n->val.integer) >= INT_MIN &&
              (long)*(n->val.integer) <= INT_MAX)
#ifdef __STDC__
            return (void *)n->val.integer;
#else
            return (char *)n->val.integer;
#endif
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for integer\n", s);
            return NULL;
          }
          break;
        case UINT_T :
          if ((unsigned long)*(n->val.integer) >= 0 &&
              (unsigned long)*(n->val.integer) <= UINT_MAX)
#ifdef __STDC__
            return (void *)n->val.integer;
#else
            return (char *)n->val.integer;
#endif
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for unsigned integer\n", s);
            return NULL;
          }
          break;
        case LONG_T :
#ifdef __STDC__
          return (void *)n->val.integer;
#else
          return (char *)n->val.integer;
#endif
          break;
        case ULONG_T :
#ifdef __STDC__
          return (void *)n->val.integer;
#else
          return (char *)n->val.integer;
#endif
          break;

/* Correction JF 20/5/94 
        case FLOAT_T :
          if (
	      ( ((double)*(n->val.integer) >= - FLT_MAX) &&
                ((double)*(n->val.integer) <= - FLT_MIN) ) ||
              ( ((double)*(n->val.integer) >=  FLT_MIN) &&
                ((double)*(n->val.integer) <=  FLT_MAX) )
	      )
*/
        case FLOAT_T :
          if (
	      ( ((double)*(n->val.integer) >= - FLT_MAX) &&
                ((double)*(n->val.integer) <= 0.0) ) ||
              ( ((double)*(n->val.integer) >=  0.0) &&
                ((double)*(n->val.integer) <=  FLT_MAX) )
	      )


#ifdef __STDC__
            return (void *)n->val.integer;
#else
            return (char *)n->val.integer;
#endif
          else {
printf("(double)*(n->val.integer) = %lg\n",(double)*(n->val.integer));

            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for float\n", s);
            return NULL;
          }
          break;
        case DOUBLE_T :
/* Correction JF 20/5/94 
          if ((long)*(n->val.integer) <= -DBL_MIN &&
              (long)*(n->val.integer) >= DBL_MIN)
*/
          if ((long)*(n->val.integer) <= -DBL_MIN &&
              (long)*(n->val.integer) >= DBL_MIN)

#ifdef __STDC__
            return (void *)n->val.integer;
#else
            return (char *)n->val.integer;
#endif
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for double\n", s);
            return NULL;
          }
          break;
        default :
          INT_ERROR("in_range");
          break;
      }
      break;
    case REAL :
      switch (t) {
        case QSTRING_T :
          if (msg_flg) error2(NULL, 0,
"'%s' default : You can't put floating-point number into quoted string variable\n", s);
          return NULL;
          break;
        case CHAR_T :
          if ((long)*(n->val.real) >= CHAR_MIN &&
              (long)*(n->val.real) <= CHAR_MAX) {
            if (msg_flg) warning2(NULL, 0,
	  "'%s' default : you may lose precision with conversion\n", s);
#ifdef __STDC__
            return (void *)n->val.real;
#else
            return (char *)n->val.real;
#endif
          }
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for character\n", s);
            return NULL;
          }
          break;
        case UCHAR_T :
          if ((unsigned long)*(n->val.real) >= 0 &&
              (unsigned long)*(n->val.real) <= UCHAR_MAX) {
            if (msg_flg) warning2(NULL, 0,
     "'%s' default : you may lose precision with conversion\n", s);
#ifdef __STDC__
            return (void *)n->val.real;
#else
            return (char *)n->val.real;
#endif
          }
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for unsigned character\n", s);
            return NULL;
          }
          break;
        case SHORT_T :
          if ((long)*(n->val.real) >= SHRT_MIN &&
              (long)*(n->val.real) <= SHRT_MAX) {
            if (msg_flg) warning2(NULL, 0,
     "'%s' default : you may lose precision with conversion\n", s);
#ifdef __STDC__
            return (void *)n->val.real;
#else
            return (char *)n->val.real;
#endif
          }
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for short integer\n", s);
            return NULL;
          }
          break;
        case USHORT_T :
          if ((unsigned long)*(n->val.real) >= 0 &&
              (unsigned long)*(n->val.real) <= USHRT_MAX) {
            if (msg_flg) warning2(NULL, 0,
     "'%s' default : you may lose precision with conversion\n", s);
#ifdef __STDC__
            return (void *)n->val.real;
#else
            return (char *)n->val.real;
#endif
          }
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for unsigned short\n", s);
            return NULL;
          }
          break;
        case INT_T :
          if ((long)*(n->val.real) >= INT_MIN &&
              (long)*(n->val.real) <= INT_MAX) {
            if (msg_flg) warning2(NULL, 0,
     "'%s' default : you may lose precision with conversion\n", s);
#ifdef __STDC__
            return (void *)n->val.real;
#else
            return (char *)n->val.real;
#endif
          }
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for integer\n", s);
            return NULL;
          }
          break;
        case UINT_T :
          if ((unsigned long)*(n->val.real) >= 0 &&
              (unsigned long)*(n->val.real) <= UINT_MAX) {
            if (msg_flg) warning2(NULL, 0,
     "'%s' default : you may lose precision with conversion\n", s);
#ifdef __STDC__
            return (void *)n->val.real;
#else
            return (char *)n->val.real;
#endif
          }
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for unsigned int\n", s);
            return NULL;
          }
          break;
        case LONG_T :
          if (*(n->val.real) >= LONG_MIN &&
              *(n->val.real) <= LONG_MAX) {
            if (msg_flg) warning2(NULL, 0,
     "'%s' default : you may lose precision with conversion\n", s);
#ifdef __STDC__
            return (void *)n->val.real;
#else
            return (char *)n->val.real;
#endif
          }
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for long\n", s);
            return NULL;
          }
          break;
        case ULONG_T :
          if (*(n->val.real) >= 0 &&
              *(n->val.real) <= ULONG_MAX) {
            if (msg_flg) warning2(NULL, 0,
     "'%s' default : you may lose precision with conversion\n", s);
#ifdef __STDC__
            return (void *)n->val.real;
#else
            return (char *)n->val.real;
#endif
          }
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for unsigned long\n", s);
            return NULL;
          }
          break;

/* Correction JF 20/5/94 
        case FLOAT_T :
          if ((*(n->val.real) >= -FLT_MAX &&
               *(n->val.real) <= -FLT_MIN) || 
              (*(n->val.real) >=  FLT_MIN &&
               *(n->val.real) <=  FLT_MAX))
*/
        case FLOAT_T :
          if ((*(n->val.real) >= -FLT_MAX &&
               *(n->val.real) <= 0.0) || 
              (*(n->val.real) >=  0.0 &&
               *(n->val.real) <=  FLT_MAX))


#ifdef __STDC__
            return (void *)n->val.real;
#else
            return (char *)n->val.real;
#endif
          else {

printf("(double)*(n->val.real) = %lg\n",(double)*(n->val.real));

            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for float\n", s);
            return NULL;
          }
          break;
        case DOUBLE_T :
#ifdef __STDC__
          return (void *)n->val.real;
#else
          return (char *)n->val.real;
#endif
          break;
        default :
          INT_ERROR("in_range");
          break;
      }
      break;
    case CHARACTER :
      switch (t) {
        case QSTRING_T :
	/*
          if (msg_flg) error2(NULL, 0,
     "'%s' default : You can't put character into quoted string variable\n", s);
          return NULL;
          break;

	  Modified by JF 14/04/2000. Try to manage QSTRING_T as CHAR_T, since
	  strings are not allowed (confusion with char *).
	*/

        case CHAR_T :
#ifdef __STDC__
          return (void *)n->val.character;
#else
          return n->val.character;
#endif
          break;
        case UCHAR_T :
          if ((long)*(n->val.real) >= 0 &&
              (long)*(n->val.real) <= UCHAR_MAX)
#ifdef __STDC__
            return (void *)n->val.character;
#else
            return n->val.character;
#endif
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for unsigned character\n", s);
            return NULL;
          }
          break;
        case SHORT_T :
#ifdef __STDC__
          return (void *)n->val.character;
#else
          return n->val.character;
#endif
          break;
        case USHORT_T :
          if ((long)*(n->val.character) >= 0)
#ifdef __STDC__
            return (void *)n->val.character;
#else
            return n->val.character;
#endif
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for short integer\n", s);
            return NULL;
          }
          break;
        case INT_T :
#ifdef __STDC__
          return (void *)n->val.character;
#else
          return n->val.character;
#endif
          break;
        case UINT_T :
          if ((long)*(n->val.character) >= 0)
#ifdef __STDC__
            return (void *)n->val.character;
#else
            return n->val.character;
#endif
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for unsigned integer\n", s);
            return NULL;
          }
          break;
        case LONG_T :
#ifdef __STDC__
          return (void *)n->val.character;
#else
          return n->val.character;
#endif
          break;
        case ULONG_T :
          if ((long)*(n->val.character) >= 0)
#ifdef __STDC__
            return (void *)n->val.character;
#else
            return n->val.character;
#endif
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for unsigned long integer\n", s);
            return NULL;
          }
          break;
/* Correction JF 20/5/94 
        case FLOAT_T :
          if (((double)*(n->val.character) >= -FLT_MAX &&
               (double)*(n->val.character) <= -FLT_MIN) || 
              ((double)*(n->val.character) >=  FLT_MIN &&
               (double)*(n->val.character) <=  FLT_MAX))
*/
        case FLOAT_T :
          if (((double)*(n->val.character) >= -FLT_MAX &&
               (double)*(n->val.character) <= 0.0) || 
              ((double)*(n->val.character) >=  0.0 &&
               (double)*(n->val.character) <=  FLT_MAX))

#ifdef __STDC__
            return (void *)n->val.character;
#else
            return n->val.character;
#endif
          else {

printf("(double)*(n->val.character) = %lg\n",(double)*(n->val.character));

            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for double\n", s);
            return NULL;
          }
          break;
        case DOUBLE_T :
          if (*(n->val.character) <= -DBL_MIN &&
              *(n->val.character) >=  DBL_MIN)
#ifdef __STDC__
            return (void *)n->val.character;
#else
            return n->val.character;
#endif
          else {
            if (msg_flg) error2(NULL, 0,
     "'%s' default : constant is out of range for double\n", s);
            return NULL;
          }
          break;
        default :
          INT_ERROR("in_range");
          break;
      }
      break;
    default :
      INT_ERROR("in_range");
      break;
  }
}


#ifdef __STDC__
static void * set_scalar_value(char *s, Type t, Node *n, Paramvalue *v)
#else
static char  * set_scalar_value(s, t, n, v)
char *s;
short t;
Node *n;
Paramvalue *v;
#endif
{
/* Ajout JF 28/05/96 */
#ifdef __STDC__
  void *ret;
#else
  char *ret;
#endif

  ret = in_range(t, n, TRUE, s);
  switch (t) {
    case QSTRING_T :
      switch(n->name) {
        case QSTRING :
	  /*
	  Modified by JF 14/04/2000. Try to manage QSTRING_T as CHAR_T, since
	  strings are not allowed (confusion with char *).
	  */
        case CHARACTER :
          v->q = (char *)ret;
          break;
	/* case CHARACTER : */
        case INTEGER :
        case REAL:
          v->q =  NULL;
          break;
        default :
          INT_ERROR("set_scalar_value");
          break;
      }
      break;
    case CHAR_T :
      switch(n->name) {
        case QSTRING :
          v->c = (char) NULL;
        case CHARACTER :
          v->c = (char)*((char *)ret);
          break;
        case INTEGER :
          v->c = (char)*((unsigned long *)ret);
          break;
        case REAL:
          v->c = (char)*((double *)ret);
          break;
        default :
          INT_ERROR("set_scalar_value");
          break;
      }
      break;
    case UCHAR_T :
      switch(n->name) {
        case QSTRING :
          v->uc = (unsigned char) NULL;
        case CHARACTER :
          v->uc = (unsigned char)*((char *)ret);
          break;
        case INTEGER :
          v->uc = (unsigned char)*((unsigned long *)ret);
          break;
        case REAL:
          v->uc = (unsigned char)*((double *)ret);
          break;
        default :
          INT_ERROR("set_scalar_value");
          break;
      }
      break;
    case SHORT_T :
      switch(n->name) {
        case QSTRING :
          v->s = (short) NULL;
        case CHARACTER :
          v->s = (short)*((char *)ret);
          break;
        case INTEGER :
          v->s = (short)*((unsigned long *)ret);
          break;
        case REAL:
          v->s = (short)*((double *)ret);
          break;
        default :
          INT_ERROR("set_scalar_value");
          break;
      }
      break;
    case USHORT_T :
      switch(n->name) {
        case QSTRING :
          v->us = (unsigned short) NULL;
        case CHARACTER :
          v->us = (unsigned short)*((char *)ret);
          break;
        case INTEGER :
          v->us = (unsigned short)*((unsigned long *)ret);
          break;
        case REAL:
          v->us = (unsigned short)*((double *)ret);
          break;
        default :
          INT_ERROR("set_scalar_value");
          break;
      }
      break;
    case INT_T :
      switch(n->name) {
        case QSTRING :
          v->i = (int) NULL;
        case CHARACTER :
          v->i = (int)*((char *)ret);
          break;
        case INTEGER :
          v->i = (int)*((unsigned long *)ret);
          break;
        case REAL:
          v->i = (int)*((double *)ret);
          break;
        default :
          INT_ERROR("set_scalar_value");
          break;
      }
      break;
    case UINT_T :
      switch(n->name) {
        case QSTRING :
          v->ui = (unsigned int) NULL;
        case CHARACTER :
          v->ui = (unsigned int)*((char *)ret);
          break;
        case INTEGER :
          v->ui = (unsigned int)*((unsigned long *)ret);
          break;
        case REAL:
          v->ui = (unsigned int)*((double *)ret);
          break;
        default :
          INT_ERROR("set_scalar_value");
          break;
      }
      break;
    case LONG_T :
      switch(n->name) {
        case QSTRING :
          v->l = (long) NULL;
        case CHARACTER :
          v->l = (long)*((char *)ret);
          break;
        case INTEGER :
          v->l = (long)*((unsigned long *)ret);
          break;
        case REAL:
          v->l = (long)*((double *)ret);
          break;
        default :
          INT_ERROR("set_scalar_value");
          break;
      }
      break;
    case ULONG_T :
      switch(n->name) {
        case QSTRING :
          v->ul = (unsigned long) NULL;
        case CHARACTER :
          v->ul = (unsigned long)*((char *)ret);
          break;
        case INTEGER :
          v->ul = (unsigned long)*((unsigned long *)ret);
          break;
        case REAL:
          v->c = (unsigned long)*((double *)ret);
          break;
        default :
          INT_ERROR("set_scalar_value");
          break;
      }
      break;
    case FLOAT_T :
      switch(n->name) {
        case QSTRING :
          v->f = (float) 0.0;
        case CHARACTER :
          v->f = (float)*((char *)ret);
          break;
        case INTEGER :
          v->f = (float)*((unsigned long *)ret);
          break;
        case REAL:
          v->f = (float)*((double *)ret);
          break;
        default :
          INT_ERROR("set_scalar_value");
          break;
      }
      break;
    case DOUBLE_T :
      switch(n->name) {
        case QSTRING :
          v->d = (double) 0.0;
        case CHARACTER :
          v->d = (double)*((char *)ret);
          break;
        case INTEGER :
          v->d = (double)*((unsigned long *)ret);
          break;
        case REAL:
          v->d = (double)*((double *)ret);
          break;
        default :
          INT_ERROR("set_scalar_value");
          break;
      }
      break;
    default :
      INT_ERROR("set_scalar_value");
      break;
  }
  return(ret);    /* Ajout JF 28/05/96 */
}


#ifdef __STDC__
static void set_scalar_default(char *s, Node *n, Paramtype *p)
#else
static set_scalar_default(s, n, p)
char *s;
Node *n;
Paramtype *p;
#endif
{
#ifdef DEBUG
  PRDBG("set_scalar_default(s = %s, n = 0x%x, p = 0x%x\n)", s, (unsigned long)n,
                                                               (unsigned long)p);
#endif
  if ((p->d = MALLOC_PARAMVALUE) != NULL) {
        mw_bzero((void *) p->d, sizeof(Paramvalue));
    if (in_range(p->t, n, FALSE, NULL) != NULL)
      set_scalar_value(s, p->t, n, p->d);
    else {
#ifdef __STDC__
      (void)in_range(p->t, n, TRUE, s);
#else
      in_range(p->t, n, TRUE, s);
#endif
      free(p->d);
      p->d = NULL;
    }
  }
  else
    error2(NULL, 0, "Not enought memory\n");
#ifdef DEBUG
  PRDBG("set_scalar_default : END\n");
#endif
}


#ifdef __STDC__
static void set_interval(char *s, Node *n, Paramtype *p)
#else
static set_interval(s, n, p)
char *s;
Node *n;
Paramtype *p;
#endif
{
  if ((p->i = MALLOC_INTERVAL) != NULL) 
    {
      mw_bzero((void *) p->i, sizeof(struct Interval));
    switch (n->name) {
      case CLOSED_INTERVAL :
        p->i->t = CLOSED;
        break;
      case MAX_EXCLUDED_INTERVAL :
        p->i->t = MAX_EXCLUDED;
        break;
      case MIN_EXCLUDED_INTERVAL :
        p->i->t = MIN_EXCLUDED;
        break;
      case OPEN_INTERVAL :
        p->i->t = OPEN;
        break;
      default :
        INT_ERROR("set_interval");;
        break;
      }
    if (set_scalar_value(s, p->t, n->left, &(p->i->min)) != NULL) 
      {
      if (set_scalar_value(s, p->t, n->right, &(p->i->max)) != NULL) 
	{
	  short is_order_correct;
	  switch (p->t) {
          case QSTRING_T :
	    /*
            error2(NULL, 0,
"'%s' interval : interval can't be used with quoted string variable\n", s);
            free(p->i);
            p->i = NULL;
            return;
            break;
	    Modified by JF 15/04/2000. Try to manage QSTRING_T as CHAR_T, 
	    since strings are not allowed (confusion with char *).
	    */
            is_order_correct = *p->i->max.q >= *p->i->min.q;
	    break;
          case CHAR_T :
            is_order_correct = p->i->max.c >= p->i->min.c;
            break;
          case UCHAR_T :
            is_order_correct = p->i->max.uc >= p->i->min.uc;
            break;
          case SHORT_T :
            is_order_correct = p->i->max.s >= p->i->min.s;
            break;
          case USHORT_T :
            is_order_correct = p->i->max.us >= p->i->min.us;
            break;
          case INT_T :
            is_order_correct = p->i->max.i >= p->i->min.i;
            break;
          case UINT_T :
            is_order_correct = p->i->max.ui >= p->i->min.ui;
            break;
          case LONG_T :
            is_order_correct = p->i->max.l >= p->i->min.l;
            break;
          case ULONG_T :
            is_order_correct = p->i->max.ul >= p->i->min.ul;
            break;
          case FLOAT_T :
            is_order_correct = p->i->max.f >= p->i->min.f;
            break;
          case DOUBLE_T :
            is_order_correct = p->i->max.d >= p->i->min.d;
            break;
          default :
            INT_ERROR("set_interval");
            break;
        }
        if (!is_order_correct) {
          error2(NULL, 0,
      "'%s' interval : max constant is less than min constant\n", s);
          free(p->i);
          p->i = NULL;
        }
      }
      else {
        error2(NULL, 0,
      "'%s' interval : problem with max constant\n", s);
        free(p->i);
        p->i = NULL;
      }
    }
    else {
      error2(NULL, 0,
      "'%s' interval : problem with min constant\n", s);
      free(p->i);
      p->i = NULL;
    }
  }
  else
    error2(NULL, 0, "Not enought memory\n");
}


#ifdef __STDC__
static void set_file_default(char *s, Node *n, Filetype *f)
#else
static set_file_default(s, n, f)
char *s;
Node *n;
Filetype *f;
#endif
{
#ifdef DEBUG
  PRDBG("set_file_default(s = %s, n = 0x%x, f = 0x%x)\n", s, (unsigned long)n,
                                                            (unsigned long)f);
#endif
  switch (n->name) {
    case QSTRING :
#ifdef DEBUG
      PRDBG("set_file_default : f->d = %s\n", f->d);
#endif
      f->d = n->val.qstring;
      break;
    case INTEGER :
    case REAL :
    case CHARACTER :
      error2(NULL, 0, "'%s' default : You can't put numerical constant\n", s);
      break;
    default :
      INT_ERROR("set_file_default");
      break;
  }
#ifdef DEBUG
  PRDBG("set_file_default : END\n");
#endif
}

#ifdef __STDC__
void trusage(Node *mwu, Header **poptionlist, Header **pneededarglist,
             Header **pvararglist, Header **poptarglist, Header **pnotusedarglist)
#else
trusage(mwu, poptionlist, pneededarglist, pvararglist, poptarglist, pnotusedarglist)
Node *mwu;
Header **poptionlist, **pneededarglist, **pvararglist, **poptarglist, **pnotusedarglist;
#endif
{
  *poptionlist     = newlist((void *)NULL);
  *pneededarglist  = newlist((void *)NULL);
  *pvararglist     = newlist((void *)NULL);
  *poptarglist     = newlist((void *)NULL);
  *pnotusedarglist = newlist((void *)NULL);

  for (; mwu->name == ','; mwu = mwu->right)
    trarg((Node *)mwu->left, *poptionlist, *pneededarglist, *pvararglist,
          *poptarglist, *pnotusedarglist);
  trarg((Node *)mwu, *poptionlist, *pneededarglist, *pvararglist, *poptarglist,
        *pnotusedarglist);
}


#ifdef __STDC__
void fillmwarg(void)
#else
fillmwarg()
#endif
{
  extern int errorcnt;
  trusage(mwusage, &optionlist, &neededarglist, &vararglist, &optarglist,
          &notusedarglist);
          
  /* If errors exist we must not generate file */
  if (errorcnt != 0)
    fatal_error("exit\n");
}
