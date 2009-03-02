/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {sreadasc};
  version = {"1.1"};
  author = {"Lionel Moisan"};
  function = {"Read a signal in ascii format"};
  usage = {    
      's':s->s  "specify scale field",
      't':t->t  "specify shift field",
      'g':g->g  "specify gain field",
      out<-out  "output Fsignal",
    { n->n      "size of signal" }

    };
*/
/*----------------------------------------------------------------------
 v1.1: added -s, -t and -g options (LM)
-----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"
 
Fsignal sreadasc(Fsignal out, int *n, float *s, float *t, float *g)
{
  int i,size;
  float v;

  if (!n) { /* read them from standart input */
    if (scanf("%d",&size) != 1) 
      mwerror(FATAL,1,"Premature end of file. Cannot read size.\n");
  } else size = *n;

  out = mw_change_fsignal(out,size);
  if (s) out->scale = *s;
  if (t) out->shift = *t;
  if (g) out->gain = *g;
  mw_clear_fsignal(out,0.);
  if (!out) mwerror(FATAL,1,"Not enough memory\n");

  for (i=0;i<size;i++) {
    if (scanf("%f",&v) != 1) 
      mwerror(WARNING,1,"Premature end of file. Will pad data with 0.\n");
      out->values[i] = v;
    }

  return(out);
}
