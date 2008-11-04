/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {resthline};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Restore infrared-type images by suppressing horizontal line effects"};
  usage = {
            input -> u      "input image",
            output <- v     "output image"
          };
*/

#include <stdlib.h>
#include <stdio.h>
#include "mw.h"

#define ABS(x) ((x)>0?(x):(-(x)))


/***** Minimize the total variation between two arrays ****/

static int minimize_tv(a,b,n)
unsigned char *a,*b;
int n;
{
    int d,dmin,i,tv,tvmin,diff;

    tvmin = 256*n; dmin = 0;
    for (d=-80; d<=80; d++) {
	tv = 0;
	for (i=0;i<n;i++) {
	    diff = d+b[i]-a[i];
	    tv+=ABS(diff);
	}
	if (tv<tvmin) {
	    dmin = d;
	    tvmin=tv;
	}
    }
    mwdebug("tvmin=%d, dmin=%d\n",tvmin,dmin);

    return dmin;
}

/*** Main function : Suppress horizontal line artefacts by minimizing ***
 ***      the total variation under horizontal offsets correction     ***/

void resthline(u,v)
Cimage u,v;
{
  int x,y,adr,dx,dy,cofs,tofs,*ofs,new;

  /* Allocate memory */
  dx=u->ncol; dy=u->nrow;
  v = mw_change_cimage(v,dy,dx);
  ofs = (int *)malloc(dy*sizeof(int));
  if (!v || !ofs) mwerror(FATAL,1,"Not enough memory.");

  /* Compute offsets between line pairs */
  cofs = 0;
  tofs = 0;
  for (y=0;y<dy-1;y++) {
      adr = y*dx;
      ofs[y] = minimize_tv(u->gray+adr,u->gray+adr+dx,dx);
      cofs += ofs[y];
      tofs += cofs;
  }
  cofs = -tofs/dy; /* correction for mean adjustment */
  mwdebug("total offset = %d\n",cofs);

  /* Process image */
  for (y=0;y<dy;y++) {
      adr = y*dx;
      for (x=0;x<dx;x++) {
	  new = u->gray[adr+x]+cofs;
	  v->gray[adr+x] = (new<0?0:new>255?255:new);
      }
      cofs += ofs[y];
  }
}

