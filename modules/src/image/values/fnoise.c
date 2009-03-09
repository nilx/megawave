/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fnoise};
 version = {"1.2"};
 author = {"Lionel Moisan"};
 function = {"Put noise on a Fimage"};
 usage = {
   'g':std->std  
     "additive Gaussian noise with standard deviation std",
   'i':p->p[0.0,100.0]      
     "impulse noise (image range), applied to p percent of the pixels",
   'q':q->q
     "additive uniform noise in [-q/2,q/2]",
   'n'->n_flag         
     "in order NOT to reinitialize the random seed",
    in->u       "input Fimage",
    out<-v      "output Fimage"
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
#include "mw-modules.h"

/*** NB: Calling this module with in=out is possible ***/

void fnoise(Fimage u, Fimage v, float *std, float *p, float *q, char *n_flag)
{
  int i;
  double a,b,z;
  float min,max,c;

  if ((std?1:0) + (p?1:0) + (q?1:0)!= 1) 
    mwerror(FATAL,1,"Please select exactly one of the -g, -i and -q options.");

  /*** Initialize random seed if necessary ***/
  if (!n_flag) srand( (unsigned int) time (NULL) + (unsigned int) getpid() );
  
  /* Allocate memory */
  v = mw_change_fimage(v,u->nrow,u->ncol);
  if (!v) mwerror(FATAL,1,"Not enough memory.");
  
  if (std) 

    /* Gaussian noise */
    for (i=u->ncol*u->nrow;i--;) {
      a = (rand() * 1.)/ RAND_MAX;
      b = (rand() * 1.)/ RAND_MAX;
      z = (double)(*std)*sqrt(-2.0*log(a))*cos(2.0*M_PI*b);
      v->gray[i] = u->gray[i] + (float)z;
    }

  else if (p) {

    /* impulse noise */
    min = max = u->gray[0];
    for (i=u->ncol*u->nrow;i--;) {
      c = u->gray[i];
      if (c<min) min=c;
      if (c>max) max=c;
    }
    for (i=u->ncol*u->nrow;i--;)
      if ((rand() * 1.)/ RAND_MAX * 100.0 < *p)
	   v->gray[i] = (float) (min + (max - min) 
				 * (rand() * 1.)/ RAND_MAX);
    else v->gray[i] = u->gray[i];
    
  } else {

    /* uniform (quantization) noise */
   for (i=u->ncol*u->nrow;i--;)
     v->gray[i] =  u->gray[i] + *q 
	 * (float) ((rand() * 1.)/ RAND_MAX - 0.5);
  }
}



