/*-------------------------------------------*/
/*                unix_bsd.c                 */
/*                 Vers 1.1                  */
/* Definition of some Unix BSD functions not */
/* (yet) implemented on Unix System V        */
/* J.Froment 1994-98                         */
/* Tested on HP-UX system version A.09.01    */
/*-------------------------------------------*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#if defined(HPUX) || defined (sun4_5) || defined(IRIX) || defined(Linux)

#define emul_unix_bsd

#include <math.h>
#include <poll.h>
#include "mw.h"

/* Round x */

double my_rint(x)
double x;
{
  return(floor(x+0.5));
}

/* aint */ 

double aint(x)
double x;
{
  if (x > 0.0) return(floor(x));
  else
  return(floor(x+0.5));
}

/* Fonction nint() presente a partir de la release Sun 4.0  (math.h)*/

int nint(x)
double x;
{return((int)my_rint(x));}

/* Fonction log2() presente a partir de la release Sun 4.0  (math.h)*/

double log2(x)
double x;
{return((double)log(x)/log(2.0));}

/* Fonction exp2() presente a partir de la release Sun 4.0  (math.h)*/

double exp2(x)
double x;
{return((double)pow(2.0,x));}

/* Racine cubique... Buguee sur HP A.09.01 */
double my_cbrt(x)
double x;
{
return((double) pow(x,.33333333333333333333));
} 

/* Function usleep(): suspend execution for interval in microseconds */

void usleep(x)
unsigned x;
{
  struct pollfd dummy;
  poll(&dummy, 0, (int) x);
}

#endif                  



