/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifdef DEBUG
#include <stdio.h>
#include "bintree.h"
#include "symbol.h"
#include "mwarg.h"
#include "data_io.h"
#include "io.h"
#define ODBGMWARG_DEC
#include "odbgmwarg.h"

#ifdef __STDC__
static void print_scalar(char *s, char *f, char *b, Paramtype p)
#else
static      print_scalar(s, f, b, p)
char *s, *f, *b;
Paramtype p;
#endif
{
  switch(p.t) {
    case QSTRING_T :
      /*
	INT_ERROR("print_scalar");
	break;
	Modified by JF 14/04/2000. Try to manage QSTRING_T as CHAR_T, since
        strings are not allowed (confusion with char *).
      */
    case CHAR_T :
  PRDBG("%s :   %s.t         %s= CHAR_T\n", s, f, b);
      if (p.d == NULL)
  PRDBG("%s :   %s.d         %s= NULL\n", s, f, b);
      else
  PRDBG("%s :   %s.d->c      %s= '%c'\n", s, f, b, p.d->c);
      if (p.i == NULL)
  PRDBG("%s :   %s.i         %s= NULL\n", s, f, b);
      else {
        switch(p.i->t) {
          case CLOSED :
  PRDBG("%s :   %s.i->t      %s= CLOSED\n", s, f, b);
            break;
          case MAX_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MAX_EXCLUDED\n", s, f, b);
            break;
          case MIN_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MIN_EXCLUDED\n", s, f, b);
            break;
          case OPEN :
  PRDBG("%s :   %s.i->t      %s= OPEN\n", s, f, b);
            break;
          default :
  PRDBG("%s :   %s.i->t      %s= UNKNOWN\n", s, f, b);
            break;
        }
  PRDBG("%s :   %s.i->min.c  %s= '%c'\n", s, f, b, p.i->min.c);
  PRDBG("%s :   %s.i->max.c  %s= '%c'\n", s, f, b, p.i->max.c);
      }
      break;
    case UCHAR_T :
  PRDBG("%s :   %s.t         %s= UCHAR_T\n", s, f, b);
      if (p.d == NULL)
  PRDBG("%s :   %s.d         %s= NULL\n", s, f, b);
      else {
        if (p.d->uc & 0x80 == 0) {
  PRDBG("%s :   %s.d->uc     %s= '%c' (0x%2.2X)\n", s, f, b, p.d->uc,
                                                    p.d->uc);
        }
        else
  PRDBG("%s :   %s.d->uc     %s= 0x%2.2X\n", s, f, b, p.d->uc);
      }
      if (p.i == NULL)
  PRDBG("%s :   %s.i         %s= NULL\n", s, f, b);
      else {
        switch(p.i->t) {
          case CLOSED :
  PRDBG("%s :   %s.i->t      %s= CLOSED\n", s, f, b);
            break;
          case MAX_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MAX_EXCLUDED\n", s, f, b);
            break;
          case MIN_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MIN_EXCLUDED\n", s, f, b);
            break;
          case OPEN :
  PRDBG("%s :   %s.i->t      %s= OPEN\n", s, f, b);
            break;
          default :
  PRDBG("%s :   %s.i->t      %s= UNKNOWN\n", s, f, b);
            break;
        }
        if (p.d->uc & 0x80 == 0)
  PRDBG("%s :   %s.i->min.uc %s= '%c' (0x%2.2X)\n", s, f, b, p.i->min.uc,
                                                  p.i->min.uc);
        else
  PRDBG("%s :   %s.i->min.uc  %s= (0x%2.2X)\n", s, f, b, p.i->min.uc);
        if (p.d->uc & 0x80 == 0)
  PRDBG("%s :   %s.i->max.uc %s= '%c' (0x%2.2X)\n", s, f, b, p.i->max.uc,
                                                  p.i->max.uc);
        else
  PRDBG("%s :   %s.i->max.uc = (0x%2.2X)\n", s, f, b, p.i->max.uc);
     }
      break;
    case SHORT_T :
  PRDBG("%s :   %s.t         %s= SHORT_T\n", s, f, b);
      if (p.d == NULL)
  PRDBG("%s :   %s.d         %s= NULL\n", s, f, b);
      else
  PRDBG("%s :   %s.d->s      %s= %d\n", s, f, b, p.d->s);
      if (p.i == NULL)
  PRDBG("%s :   %s.i         %s= NULL\n", s, f, b);
      else {
        switch(p.i->t) {
          case CLOSED :
  PRDBG("%s :   %s.i->t      %s= CLOSED\n", s, f, b);
            break;
          case MAX_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MAX_EXCLUDED\n", s, f, b);
            break;
          case MIN_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MIN_EXCLUDED\n", s, f, b);
            break;
          case OPEN :
  PRDBG("%s :   %s.i->t      %s= OPEN\n", s, f, b);
            break;
          default :
  PRDBG("%s :   %s.i->t      %s= UNKNOWN\n", s, f, b);
            break;
        }
  PRDBG("%s :   %s.i->min.s  %s= %d\n", s, f, b, p.i->min.s);
  PRDBG("%s :   %s.i->max.s  %s= %d\n", s, f, b, p.i->max.s);
      }
      break;
    case USHORT_T :
  PRDBG("%s :   %s.t         %s= USHORT_T\n", s, f, b);
      if (p.d == NULL)
  PRDBG("%s :   %s.d         %s= NULL\n", s, f, b);
      else
  PRDBG("%s :   %s.d->us     %s= 0x%x (%d)\n", s, f, b, p.d->us,
                                                p.d->us);
      if (p.i == NULL)
  PRDBG("%s :   %s.i         %s= NULL\n", s, f, b);
      else {
        switch(p.i->t) {
          case CLOSED :
  PRDBG("%s :   %s.i->t      %s= CLOSED\n", s, f, b);
            break;
          case MAX_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MAX_EXCLUDED\n", s, f, b);
            break;
          case MIN_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MIN_EXCLUDED\n", s, f, b);
            break;
          case OPEN :
  PRDBG("%s :   %s.i->t      %s= OPEN\n", s, f, b);
            break;
          default :
  PRDBG("%s :   %s.i->t      %s= UNKNOWN\n", s, f, b);
            break;
        }
  PRDBG("%s :   %s.i->min.us %s= %x (%d)\n", s, f, b, p.i->min.us,
                                              p.i->min.us);
  PRDBG("%s :   %s.i->max.us %s= %x (%d)\n", s, f, b, p.i->max.us,
                                              p.i->max.us);
      }
      break;
    case INT_T :
  PRDBG("%s :   %s.t         %s= INT_T\n", s, f, b);
      if (p.d == NULL)
  PRDBG("%s :   %s.d         %s= NULL\n", s, f, b);
      else
  PRDBG("%s :   %s.d->i      %s= %d\n", s, f, b, p.d->i);
      if (p.i == NULL)
  PRDBG("%s :   %s.i         %s= NULL\n", s, f, b);
      else {
        switch(p.i->t) {
          case CLOSED :
  PRDBG("%s :   %s.i->t      %s= CLOSED\n", s, f, b);
            break;
          case MAX_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MAX_EXCLUDED\n", s, f, b);
            break;
          case MIN_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MIN_EXCLUDED\n", s, f, b);
            break;
          case OPEN :
  PRDBG("%s :   %s.i->t      %s= OPEN\n", s, f, b);
            break;
          default :
  PRDBG("%s :   %s.i->t      %s= UNKNOWN\n", s, f, b);
            break;
        }
  PRDBG("%s :   %s.i->min.i  %s= %d\n", s, f, b, p.i->min.i);
  PRDBG("%s :   %s.i->max.i  %s= %d\n", s, f, b, p.i->max.i);
      }
      break;
    case UINT_T :
  PRDBG("%s :   %s.t         %s= UINT_T\n", s, f, b);
      if (p.d == NULL)
  PRDBG("%s :   %s.d         %s= NULL\n", s, f, b);
      else
  PRDBG("%s :   %s.d->ui     %s= 0x%X (%d)\n", s, f, b, p.d->ui,
                                              p.d->ui);
      if (p.i == NULL)
  PRDBG("%s :   %s.i         %s= NULL\n", s, f, b);
      else {
        switch(p.i->t) {
          case CLOSED :
  PRDBG("%s :   %s.i->t      %s= CLOSED\n", s, f, b);
            break;
          case MAX_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MAX_EXCLUDED\n", s, f, b);
            break;
          case MIN_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MIN_EXCLUDED\n", s, f, b);
            break;
          case OPEN :
  PRDBG("%s :   %s.i->t      %s= OPEN\n", s, f, b);
            break;
          default :
  PRDBG("%s :   %s.i->t      %s= UNKNOWN\n", s, f, b);
            break;
        }
  PRDBG("%s :   %s.i->min.ui    %s= %X (%d)\n", s, f, b, p.i->min.ui,
                                               p.i->min.ui);
  PRDBG("%s :   %s.i->max.ui    %s= %X (%d)\n", s, f, b, p.i->max.ui,
                                               p.i->max.ui);
      }
      break;
    case LONG_T :
  PRDBG("%s :   %s.t         %s= LONG_T\n", s, f, b);
      if (p.d == NULL)
  PRDBG("%s :   %s.d         %s= NULL\n", s, f, b);
      else
  PRDBG("%s :   %s.d->l      %s= %ld\n", s, f, b, p.d->l);
      if (p.i == NULL)
  PRDBG("%s :   %s.i         %s= NULL\n", s, f, b);
      else {
        switch(p.i->t) {
          case CLOSED :
  PRDBG("%s :   %s.i->t      %s= CLOSED\n", s, f, b);
            break;
          case MAX_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MAX_EXCLUDED\n", s, f, b);
            break;
          case MIN_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MIN_EXCLUDED\n", s, f, b);
            break;
          case OPEN :
  PRDBG("%s :   %s.i->t      %s= OPEN\n", s, f, b);
            break;
          default :
  PRDBG("%s :   %s.i->t      %s= UNKNOWN\n", s, f, b);
            break;
        }
  PRDBG("%s :   %s.i->min.l  %s= %ld\n", s, f, b, p.i->min.l);
  PRDBG("%s :   %s.i->max.l  %s= %ld\n", s, f, b, p.i->max.l);
      }
      break;
    case ULONG_T :
  PRDBG("%s :   %s.t         %s= ULONG_T\n", s, f, b);
      if (p.d == NULL)
  PRDBG("%s :   %s.d         %s= NULL\n", s, f, b);
      else
  PRDBG("%s :   %s.d->ul     %s= 0x%lX (%ld)\n", s, f, b, p.d->ul,
                                                p.d->ul);
      if (p.i == NULL)
  PRDBG("%s :   %s.i         %s= NULL\n", s, f, b);
      else {
        switch(p.i->t) {
          case CLOSED :
  PRDBG("%s :   %s.i->t      %s= CLOSED\n", s, f, b);
            break;
          case MAX_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MAX_EXCLUDED\n", s, f, b);
            break;
          case MIN_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MIN_EXCLUDED\n", s, f, b);
            break;
          case OPEN :
  PRDBG("%s :   %s.i->t      %s= OPEN\n", s, f, b);
            break;
          default :
  PRDBG("%s :   %s.i->t      %s= UNKNOWN\n", s, f, b);
            break;
        }
  PRDBG("%s :   %s.i->min.ul %s= %lX (%ld)\n", s, f, b, p.i->min.ul,
                                                 p.i->min.ul);
  PRDBG("%s :   %s.i->max.ul %s= %lX (%ld)\n", s, f, b, p.i->max.ul,
                                                 p.i->max.ul);
      }
      break;
    case FLOAT_T :
  PRDBG("%s :   %s.t         %s= FLOAT_T\n", s, f, b);
      if (p.d == NULL)
  PRDBG("%s :   %s.d         %s= NULL\n", s, f, b);
      else
  PRDBG("%s :   %s.d->f      %s= %g\n", s, f, b, (double)p.d->f);
      if (p.i == NULL)
  PRDBG("%s :   %s.i         %s= NULL\n", s, f, b);
      else {
        switch(p.i->t) {
          case CLOSED :
  PRDBG("%s :   %s.i->t      %s= CLOSED\n", s, f, b);
            break;
          case MAX_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MAX_EXCLUDED\n", s, f, b);
            break;
          case MIN_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MIN_EXCLUDED\n", s, f, b);
            break;
          case OPEN :
  PRDBG("%s :   %s.i->t      %s= OPEN\n", s, f, b);
            break;
          default :
  PRDBG("%s :   %s.i->t      %s= UNKNOWN\n", s, f, b);
            break;
        }
  PRDBG("%s :   %s.i->min.f  %s= %g\n", s, f, b, (double)p.i->min.f);
  PRDBG("%s :   %s.i->max.f  %s= %g\n", s, f, b, (double)p.i->max.f);
      }
      break;
    case DOUBLE_T :
  PRDBG("%s :   %s.t         %s= DOUBLE_T\n", s, f, b);
      if (p.d == NULL)
  PRDBG("%s :   %s.d         %s= NULL\n", s, f, b);
      else
  PRDBG("%s :   %s.d->d      %s= %g\n", s, f, b, p.d->d);
      if (p.i == NULL)
  PRDBG("%s :   %s.i         %s= NULL\n", s, f, b);
      else {
        switch(p.i->t) {
          case CLOSED :
  PRDBG("%s :   %s.i->t      %s= CLOSED\n", s, f, b);
            break;
          case MAX_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MAX_EXCLUDED\n", s, f, b);
            break;
          case MIN_EXCLUDED :
  PRDBG("%s :   %s.i->t      %s= MIN_EXCLUDED\n", s, f, b);
            break;
          case OPEN :
  PRDBG("%s :   %s.i->t      %s= OPEN\n", s, f, b);
            break;
          default :
  PRDBG("%s :   %s.i->t      %s= UNKNOWN\n", s, f, b);
            break;
        }
  PRDBG("%s :   %s.i->min.d  %s= %g\n", s, f, b, p.i->min.d);
  PRDBG("%s :   %s.i->max.d  %s= %g\n", s, f, b, p.i->max.d);
      }
      break;
  }
}


#ifdef __STDC__
void print_mwarg(char *s, Mwarg *e)
#else
print_mwarg(s, e)
char *s;
Mwarg *e;
#endif
{
  PRDBG("%s : e = {\n", s);
  PRDBG("%s :   name                = %s\n", s, e->name);
  PRDBG("%s :   type                = (0x%lx) %N\n", s, (unsigned long)e->type, e->type);
  PRDBG("%s :   access              = (0x%lx) %N\n", s, (unsigned long)e->access,
                                               e->access);
  PRDBG("%s :   texname             = %s\n", s, e->texname);
  PRDBG("%s :   lineno              = %d\n", s, e->lineno);
  PRDBG("%s :   filein              = %s\n", s, e->filein);
  PRDBG("%s :   desc                = %s\n", s, e->desc);
  if (e->iodesc != NULL) {
    switch(e->iodesc->t) {
      case QSTRING_T :
  PRDBG("%s :   iodesc->t           = QSTRING_T\n", s);
        break;
      case CHAR_T :
  PRDBG("%s :   iodesc->t           = CHAR_T\n", s);
        break;
      case UCHAR_T :
  PRDBG("%s :   iodesc->t           = UCHAR_T\n", s);
        break;
      case SHORT_T :
  PRDBG("%s :   iodesc->t           = SHORT_T\n", s);
        break;
      case USHORT_T :
  PRDBG("%s :   iodesc->t           = USHORT_T\n", s);
        break;
      case INT_T :
  PRDBG("%s :   iodesc->t           = INT_T\n", s);
        break;
      case UINT_T :
  PRDBG("%s :   iodesc->t           = UINT_T\n", s);
        break;
      case LONG_T :
  PRDBG("%s :   iodesc->t           = LONG_T\n", s);
        break;
      case ULONG_T :
  PRDBG("%s :   iodesc->t           = ULONG_T\n", s);
        break;
      case FLOAT_T :
  PRDBG("%s :   iodesc->t           = FLOAT_T\n", s);
        break;
      case DOUBLE_T :
  PRDBG("%s :   iodesc->t           = DOUBLE_T\n", s);
        break;
      case MW2_T :
  PRDBG("%s :   iodesc->t           = MW2_T\n", s);
        break;
      case NONE_T :
  PRDBG("%s :   iodesc->t           = NONE_T\n", s);
        break;
      default :
  PRDBG("%s :   iodesc->t           = UNKNOWN\n", s);
        break;
    }
  PRDBG("%s :   iodesc->type        = %s\n", s, e->iodesc->type);
    switch(e->iodesc->rw) {
      case READ :
  PRDBG("%s :   iodesc->rw          = READ\n", s);
        break;
      case WRITE :
  PRDBG("%s :   iodesc->rw          = WRITE\n", s);
        break;
      default :
  PRDBG("%s :   iodesc->rw          = UNKNOWN\n", s);
        break;
    }
  PRDBG("%s :   iodesc->function    = %s\n", s, e->iodesc->function);
  PRDBG("%s :   iodesc->include     = %s\n", s, e->iodesc->include);
  }
  else
  PRDBG("%s :   iodesc              = NULL\n", s);

  switch (e->t) {
    case OPTION :
  PRDBG("%s :   t                   = OPTION\n", s);
  PRDBG("%s :   d.o.o               = '%c'\n", s, e->d.o.o);
  PRDBG("%s :   d.o.d.rw            = %s\n", s,
            (e->d.o.d.rw == READ) ? "READ" :
            (e->d.o.d.rw == WRITE) ? "WRITE" : "UNKNOWN");
      switch(e->d.o.d.t) {
        case FILEARG :
  PRDBG("%s :   d.o.d.t             = FILEARG\n", s);
  PRDBG("%s :   d.o.d.v.f.d         = %s\n", s, (e->d.o.d.v.f.d!=NULL)?e->d.o.d.v.f.d:"NULL");
          break;
        case SCALARARG :
  PRDBG("%s :   d.o.d.t             = SCALARARG\n", s);
          print_scalar(s, "d.o.d.v.p", "", e->d.o.d.v.p);
          break;
        case FLAGARG :
  PRDBG("%s :   d.o.d.t             = FLAGARG\n", s);
          break;
        default :
  PRDBG("%s :   d.o.d.t             = UNKNOWN\n", s);
          break;
      }
      break;
    case NEEDEDARG :
    case VARARG :
    case OPTIONARG :
    case NOTUSEDARG :
      switch(e->t){
        case NEEDEDARG :
  PRDBG("%s :   t                   = NEEDEDARG\n", s);
          break;
        case VARARG :
  PRDBG("%s :   t                   = VARARG\n", s);
          break;
        case OPTIONARG :
  PRDBG("%s :   t                   = OPTIONARG\n", s);
          break;
        case NOTUSEDARG :
  PRDBG("%s :   t                   = NOTUSEDARG\n", s);
          break;
      }
  PRDBG("%s :   d.a.rw            = %s\n", s,
            (e->d.a.rw == READ) ? "READ" :
            (e->d.a.rw == WRITE) ? "WRITE" : "UNKNOWN");
      switch(e->d.a.t) {
        case FILEARG :
  PRDBG("%s :   d.a.t               = FILEARG\n", s);
  PRDBG("%s :   d.a.v.f.d           = %s\n", s, (e->d.a.v.f.d!=NULL)?e->d.a.v.f.d:"NULL");
          break;
        case SCALARARG :
  PRDBG("%s :   d.a.t               = SCALARARG\n", s);
          print_scalar(s, "d.a.v.p", "  ", e->d.a.v.p);
          break;
        case FLAGARG :
  PRDBG("%s :   d.a.t               = FLAGARG\n", s);
          break;
        default :
  PRDBG("%s :   d.a.t               = UNKNOWN\n", s);
          break;
      }
      break;
    default :
  PRDBG("%s :   t                   = UNKNOWN\n", s);
      break;
  }
  PRDBG("%s : }\n\n", s);
}
#endif
