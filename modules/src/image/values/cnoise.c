/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {cnoise};
 version = {"1.2"};
 author = {"Lionel Moisan"};
 function = {"Put noise on a Cimage"};
 usage = {
   'g':std->std  
     "additive Gaussian noise with standard deviation std",
   'i':p->p[0.0,100.0]      
     "impulse noise (range 0..255), applied to p percent of the pixels",
   'q':q->q
     "additive uniform noise in [-q/2,q/2]",
   'n'->n_flag         
     "in order NOT to reinitialize the random seed",
    in->u       "input Cimage",
    out<-v      "output Cimage"
};
*/
/*----------------------------------------------------------------------
 v1.1: added -q option (LM)
 v1.2: added processus number term in the random seed initialization (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h> /* FIXME: unix-centric */
#include "mw.h"

/*** NB: Calling this module with in=out is possible ***/


unsigned char truncation(x)
float x;
{
  return (x<0.0?0:(x>255.0?255:(unsigned char)x));
}

void cnoise(u,v,std,p,q,n_flag)
     Cimage	u,v;
     float	*std,*p,*q;
     char       *n_flag;
{
  int i;
  double a,b,z;
  float min,max,c;

  if ((std?1:0) + (p?1:0) + (q?1:0)!= 1) 
    mwerror(FATAL,1,"Please select exactly one of the -g, -i and -q options.");

  /*** Initialize random seed if necessary ***/
  if (!n_flag) srand( (unsigned int) time (NULL) + (unsigned int) getpid() );
  
  /* Allocate memory */
  v = mw_change_cimage(v,u->nrow,u->ncol);
  if (!v) mwerror(FATAL,1,"Not enough memory.");
  
  if (std) 

    /* Gaussian noise */
    for (i=u->ncol*u->nrow;i--;) {
      a = (double) rand() / RAND_MAX;
      b = (double) rand() / RAND_MAX;
      z = (double)(*std)*sqrt(-2.0*log(a))*cos(2.0*M_PI*b);
      v->gray[i] = truncation( u->gray[i] + (float)z );
    }

  else if (p) {

    /* impulse noise */
    for (i=u->ncol*u->nrow;i--;)
      if ((double) rand() / RAND_MAX * 100.0 < *p) 
	v->gray[i] = truncation( (float)(255.999 
					 * (double) rand() / RAND_MAX));
    else v->gray[i] = u->gray[i];
    
  } else {

    /* uniform (quantization) noise */
   for (i=u->ncol*u->nrow;i--;)
     v->gray[i] = truncation( u->gray[i] + *q 
			      * (float) ((double) rand() / RAND_MAX - 0.5));
  }
}



