/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {cnoise};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Put noise on a Cimage"};
  usage = {
  'g':std->std  
    "additive Gaussian noise with standard deviation std",
  'i':p->p[0.0,100.0]      
    "impulse noise (range 0..255), applied to p percent of the pixels",
  'n'->n_flag         
    "in order NOT to reinitialize the random seed",
   in->u       "input Cimage",
   out<-v      "output Cimage"
  };
*/

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "mw.h"

/* for drand48() */
#ifdef __STDC__
#include <stdlib.h>
#else
extern double drand48();
#endif


/*** NB: Calling this module with in=out is possible ***/


unsigned char truncate(x)
float x;
{
  return (x<0.0?0:(x>255.0?255:(unsigned char)x));
}

void cnoise(u,v,std,p,n_flag)
Cimage	u,v;
float	*std,*p;
char    *n_flag;
{
  int i;
  double a,b,z;
  float min,max,c;

  if ((std?1:0) + (p?1:0) != 1) 
    mwerror(FATAL,1,"Please select exactly one of the -g and -i options.");

  /*** Initialize random seed if necessary ***/
  if (!n_flag) srand48( (long int) time (NULL) );
  
  /* Allocate memory */
  v = mw_change_cimage(v,u->nrow,u->ncol);
  if (!v) mwerror(FATAL,1,"Not enough memory.");
  
  if (std) 

    /* Gaussian noise */
    for (i=u->ncol*u->nrow;i--;) {
      a = drand48();
      b = drand48();
      z = (double)(*std)*sqrt(-2.0*log(a))*cos(2.0*M_PI*b);
      v->gray[i] = truncate( u->gray[i] + (float)z );
    }

  else {

    /* impulse noise */
    for (i=u->ncol*u->nrow;i--;)
      if (drand48()*100.0<*p) 
	v->gray[i] = truncate( (float)(255.999*drand48()) );
    else v->gray[i] = u->gray[i];
    
  }
}



