/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {fnoise};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Put noise on a Fimage"};
  usage = {
  'g':std->std  
    "additive Gaussian noise with standard deviation std",
  'i':p->p[0.0,100.0]      
    "impulse noise (image range), applied to p percent of the pixels",
  'n'->n_flag         
    "in order NOT to reinitialize the random seed",
   in->u       "input Fimage",
   out<-v      "output Fimage"
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


void fnoise(u,v,std,p,n_flag)
Fimage	u,v;
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
  v = mw_change_fimage(v,u->nrow,u->ncol);
  if (!v) mwerror(FATAL,1,"Not enough memory.");
  
  if (std) 

    /* Gaussian noise */
    for (i=u->ncol*u->nrow;i--;) {
      a = drand48();
      b = drand48();
      z = (double)(*std)*sqrt(-2.0*log(a))*cos(2.0*M_PI*b);
      v->gray[i] = u->gray[i] + (float)z;
    }

  else {

    /* impulse noise */
    min = max = u->gray[0];
    for (i=u->ncol*u->nrow;i--;) {
      c = u->gray[i];
      if (c<min) min=c;
      if (c>max) max=c;
    }
    for (i=u->ncol*u->nrow;i--;)
      if (drand48()*100.0<*p) v->gray[i] = (float)(min+(max-min)*drand48());
    else v->gray[i] = u->gray[i];
    
  }
}



