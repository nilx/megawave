/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {fhisto};
  version = {"1.3"};
  author = {"Lionel Moisan"};
  function = {"Compute the histogram of a Fimage"};
  usage = {
    'l':l->l    "left bound of sampling interval",
    'r':r->r    "right bound of sampling interval",
    'n':n->n    "number of cells (if no option specified: 100)",
    's':s->s    "size of each cell (alternate option)",
    't':t->t    "truncate values outside interval",
    input->in   "input Fimage",
    output<-out "output Fsignal"
          };
*/
/*----------------------------------------------------------------------
 v1.2: more possible combinations of options, new -t option (L.Moisan)
 v1.3: default num = (max-min)/size (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

#define DEFAULT_NUMBER_OF_CELLS 100

Fsignal fhisto(in,out,l,r,n,s,t)
     Fimage   in;
     Fsignal  out;
     float    *l,*r;
     int      *n;
     float    *s;
     char     *t;
{
  float min,max,size,v;
  int num,i,cell;

  /* compute min and max */
  min = max = in->gray[0];
  for (i = in->nrow*in->ncol;i--;) {
    v=in->gray[i];
    if (v<min) min=v;
    if (v>max) max=v;
  }

  /* default */
  if (l) min = *l;
  if (r) max = *r;
  if (n) num = *n; else {
    if (s) {
      size = *s;
      num = (int)(0.5+(max-min)/size);
      if (num<=0) num=1;
    } else num = DEFAULT_NUMBER_OF_CELLS;
  }
  if (!s) size = (max-min)/(float)num;

  switch ((l?1:0) + (r?1:0) + (n?1:0) + (s?1:0)) {

  case 0: 
    break;

  case 1:
    if (s) {
      min = size * (float)floor((double)(min/size));
      num = 1+(int)floor((double)((max-min)/size));
    } 
    break;
    
  case 2: 
    if (n && s) 
      mwerror(USAGE,1,"You cannot use only -n and -s options together\n");
    break;
      
  case 3: 
    if (!l) min = max-(float)num*size;
    if (!r) max = min+(float)num*size;
    if (!n) {
      size = (max-min)/(float)(num);
      if (size!=*s) 
	mwerror(WARNING,0,"cell size changed to match interval bounds\n");
    }
    break;
    
  default: 
    mwerror(USAGE,1,"You cannot use the 4 options -l -r -n and -s together\n");
  }

  /* prepare output */
  out = mw_change_fsignal(out,num);
  if (!out) mwerror(FATAL,1,"Not enough memory\n");
  mw_clear_fsignal(out,0.0);
  out->shift = min;
  out->scale = size;

  /* compute histogram */
  for (i = in->nrow*in->ncol;i--;) {
    cell = (in->gray[i]-min)/size;
    if (t) {
      if (cell<0) cell=0;
      if (cell>=num) cell=num-1;
    }
    if (cell>=0 && cell<num) out->values[cell] += 1.;
  }

  return(out);
}

