/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {cmnoise};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Put noise on a Cmovie"};
  usage = {
  'g':std->std  
    "additive Gaussian noise with standard deviation std",
  'i':p->p[0.0,100.0]      
    "impulse noise (range 0..255), applied to p percent of the pixels",
   in->in           "input Cmovie",
   out<-cmnoise     "output Cmovie"
  };
*/

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "mw.h"

/* for drand48() */
#ifdef __STDC__
extern void cnoise(Cimage,Cimage,float*,float*,char*);
#else
extern void cnoise();
#endif

Cmovie cmnoise(in,std,p)
Cmovie	in;
float	*std,*p;
{
  Cmovie out;
  Cimage u,new,prev,*next;
  char   *init;
  
  if ((std?1:0) + (p?1:0) != 1) 
    mwerror(FATAL,1,"Please select exactly one of the -g and -i options.");
  
  out = mw_new_cmovie();
  prev = NULL;
  next = &(out->first);
  init = NULL;

  for (u=in->first;u;u=u->next) {
    new = mw_new_cimage();
    cnoise(u,new,std,p,init);
    init = (char *)1;
    new->previous = prev;
    *next = prev = new;
    next = &(new->next);
  }
  *next = NULL;

  return out;
}

