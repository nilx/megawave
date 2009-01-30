/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {cmcollect};
  version = {"1.1"};
  author = {"Lionel Moisan"};
  function = {"Collect images of a Cmovie into a single Cimage"};
  usage = {
    'n':n->n       "number of images per line (default auto 4/3)",
    'i':[i=1]->i   "inside margin in pixels",    
    'o':[o=1]->o   "outside margin in pixels",
    'c':[c=0]->c   "margin grey level",
    'x':x->x       "image width (default from first image)",
    'y':y->y       "image height (default from first image)",
    in->in         "input Cmovie",
    out<-out       "output Cimage"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

Cimage cmcollect(Cmovie in, Cimage out, int *n, int *i, int *o, int *c, int *x, int *y)
{
  Cimage u;
  int nx,ny,nl,nr,ox,oy,ix,iy,tx,ty,dx,dy,il,k;

  if (!in || !in->first) mwerror(FATAL,1,"Empty input.");
  u = in->first;
  nx = (x?*x:u->ncol);
  ny = (y?*y:u->nrow);
  for (k=0;u;u=u->next,k++);
  if (n) nl=*n; else {
    nl = 0;
    do {
      nl++;
      nr = (k+nl-1)/nl;
    } while (nl<k && 3*nl*nx<4*nr*ny);
  }
  nr = (k+nl-1)/nl;
  ox = *o*2+*i*(nl-1)+nx*nl;
  oy = *o*2+*i*(nr-1)+ny*nr;
  mwdebug("%d images put in a %dx%d frame of %dx%d pixels\n",k,nl,nr,ox,oy);

  out = mw_change_cimage(out,oy,ox);
  mw_clear_cimage(out,*c);

  for (u=in->first,il=0,dx=dy=*o;u;u=u->next) {
    for (ix=0;ix<u->ncol;ix++) {
      tx = ix+(nx-u->ncol)/2;
      if (tx>=0 && tx<nx) 
	for (iy=0;iy<u->nrow;iy++) {
	  ty = iy+(ny-u->nrow)/2;
	  if (ty>=0 && ty<ny) 
	    out->gray[ox*(dy+ty)+dx+tx] = u->gray[iy*u->ncol+ix];
	}
    }
    il++;
    if (il==nl) {
      il = 0;
      dx = *o;
      dy += *i+ny;
    } else 
      dx += *i+nx;
  }

  return(out);
}
