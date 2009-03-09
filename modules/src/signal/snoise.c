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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "mw.h"
#include "mw-modules.h"

/*** NB: Calling this module with in=out is possible ***/

Fsignal snoise(Fsignal in, Fsignal out, float *std, float *t, char *n_flag)
{
  int    i;
  double a,b,z;

  if ((std?1:0) + (t?1:0) != 1) 
    mwerror(USAGE,0,"Please select exactly one of the -g and -t options.");

  /*** Initialize random seed if necessary ***/
  if (!n_flag) srand( (unsigned int) time (NULL) );
  
  /* Allocate memory */
  out = mw_change_fsignal(out,in->size);
  if (!out) mwerror(FATAL,1,"Not enough memory.");
  mw_copy_fsignal_header(in,out);

  if (std) 

    /* Gaussian noise */
    for (i=in->size;i--;) {
      a = (rand() * 1.)/ RAND_MAX;
      b = (rand() * 1.)/ RAND_MAX;
      z = (double)(*std)*sqrt(-2.0*log(a))*cos(2.0*M_PI*b);
      out->values[i] = in->values[i] + (float)z;
    }

  else {

    /* transmission noise */
    for (i=in->size;i--;)
      if ((rand() * 1.)/ RAND_MAX * 100.0 < *t) out->values[i] = 0.;
      else out->values[i] = in->values[i];
    
  }

  return(out);
}



