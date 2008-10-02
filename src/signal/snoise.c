/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {snoise};
  version = {"1.1"};
  author = {"Lionel Moisan"};
  function = {"Put noise on a Fsignal"};
  usage = {
  'g':std->std      "additive Gaussian noise with standard deviation std",

  't':t->t[0.0,100.0]     
    "transmission noise : t percent of signal values are lost (set to 0)",

  'n'->n_flag       "in order NOT to reinitialize the random seed",

   in->in           "input Fsignal",

   out<-out         "output Fsignal"
  };
*/
/*----------------------------------------------------------------------
 v1.1: preserve header info for e.g. sound processing (JF) 
----------------------------------------------------------------------*/

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

Fsignal snoise(in,out,std,t,n_flag)
     Fsignal in,out;
     float   *std,*t;
     char    *n_flag;
{
  int    i;
  double a,b,z;

  if ((std?1:0) + (t?1:0) != 1) 
    mwerror(USAGE,0,"Please select exactly one of the -g and -t options.");

  /*** Initialize random seed if necessary ***/
  if (!n_flag) srand48( (long int) time (NULL) );
  
  /* Allocate memory */
  out = mw_change_fsignal(out,in->size);
  if (!out) mwerror(FATAL,1,"Not enough memory.");
  mw_copy_fsignal_header(in,out);

  if (std) 

    /* Gaussian noise */
    for (i=in->size;i--;) {
      a = drand48();
      b = drand48();
      z = (double)(*std)*sqrt(-2.0*log(a))*cos(2.0*M_PI*b);
      out->values[i] = in->values[i] + (float)z;
    }

  else {

    /* transmission noise */
    for (i=in->size;i--;)
      if (drand48()*100.0<*t) out->values[i] = 0.;
      else out->values[i] = in->values[i];
    
  }

  return(out);
}



