/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {fhisto};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Compute the histogram of a Fimage"};
  usage = {
    'l':l->l    "left bound of sampling interval",
    'r':r->r    "right bound of sampling interval",
    'n':n->n    "number of cells (if no option specified: 100)",
    's':s->s    "size of each cell (alternate option)",
    input->in   "input Fimage",
    output<-out "output Fsignal"
          };
*/
/*-- MegaWave2 - Copyright (C) 1994 Jacques Froment. All Rights Reserved. --*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

#define DEFAULT_NUMBER_OF_CELLS 100

void fhisto(in,out,l,r,n,s)
Fimage in;
Fsignal out;
float *l,*r;
int *n;
float *s;
{
  float left,right,size,v;
  int num,i,cell;

  /* check options and compute interval*/

  num = DEFAULT_NUMBER_OF_CELLS;

  switch ((l?1:0) + (r?1:0) + (n?1:0) + (s?1:0)) {
  case 1:
    if (!n)
      mwerror(USAGE,1,"Use no option, -n alone, both -r and -l, or 3 options");
    num = *n;
    
  case 0: 
    left = right = in->gray[0];
    for (i = in->nrow*in->ncol;i--;) {
      v=in->gray[i];
      if (v<left) left=v;
      if (v>right) right=v;
    }
    size = (right-left)/(float)(num);
    break;
    
  case 2: 
    if (!l || !r) 
      mwerror(USAGE,1,"Use no option, -n alone, both -r and -l, or 3 options");
    left = *l;
    size = (*r-left)/(float)(num);
    break;
      
  case 3: 
    if (!l) left = *r-(float)(*n)*(*s); else left=*l;
    if (!s) size = (*r-(*l))/(float)(*n); else size=*s;
    if (!n) {
      num = (int)ceil((double)((*r-(*l))/(*s)));
      size = (*r-(*l))/(float)(num);
      if (size!=*s) 
	mwerror(WARNING,0,"cell size changed to match interval bounds");
    } else num=*n;
    break;
    
  default: 
    mwerror(USAGE,1,"Use no option, -n alone, both -r and -l, or 3 options");
  }

  /* prepare output */
  out = mw_change_fsignal(out,num);
  if (!out) mwerror(FATAL,1,"Not enough memory");
  mw_clear_fsignal(out,0.0);
  out->shift = left;
  out->scale = size;

  /* compute histogram */
  for (i = in->nrow*in->ncol;i--;) {
    cell = (in->gray[i]-left)/size;
    if (cell<0) cell=0;
    if (cell>=num) cell=num-1;
    out->values[cell] ++;
  }
}

