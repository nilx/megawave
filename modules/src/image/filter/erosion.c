/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {erosion};
  version = {"2.6"};
  author = {"Lionel Moisan"};
  function = {"erosion/dilation of a Cimage"};
  usage = {
  'i'->i          "if set, a dilation is applied instead of an erosion",
  's':s->s        "if set, the shape s is taken as structuring element",
  'r':[r=1.0]->r  "otherwise, a disc of radius r is used",
  'n':[n=1]->n    "number of iterations",
  in->u           "input Cimage",
  out<-v          "output Cimage"
          };
*/
/*----------------------------------------------------------------------
 v2.6 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h" /* for disc() */

#define ABS(x) ((x)>0?(x):(-(x)))
#define EROSION 0
#define DILATION 1

/*------------- one erosion/dilation of a cimage by a shape s -------------*/

static void erodilat1(Cimage u, Cimage v, Curve s, short int action)
{
  register int x,y,nx,ny,xx,yy;
  register unsigned char new,curr;
  Point_curve p;
  
  nx=u->ncol; 
  ny=u->nrow;
  for (x=0;x<nx;x++)
    for (y=0;y<ny;y++) {
      new = u->gray[y*nx+x];
      for (p=s->first;p;p=p->next) {
	xx = x+p->x;
	yy = y+p->y;
	if (xx>=0 && xx<nx && yy>=0 && yy<ny) {
	  curr = u->gray[yy*nx+xx];
	  if (action==EROSION) 
	    {  if (curr<new) new=curr; } /*** erosion  -> inf ***/
	  else if (curr>new) new=curr;   /*** dilation -> sup ***/
	  
	}
      }
      v->gray[y*nx+x]=new;
    }
}

/*---------------------- Main function : erosion --------------------*/

Cimage erosion(Cimage u, Cimage v, float *r, Curve s, int *n, char *i)
{
  Cimage w,*src,*dst,*new;
  Curve  shape;
  short  action;
  int    niter;
  
  /* check options and allocate memory */
  
  if (*n<=0) mwerror(FATAL,1,"The -n option parameter must be positive.");
  else niter=*n;
  
  if (*r<=0) mwerror(FATAL,1,"Radius parameter must be positive.");
  if (s) shape = s; else shape = disc(*r,NULL); 
  
  v = mw_change_cimage(v,u->nrow,u->ncol);
  if (!v) mwerror(FATAL,1,"Not enough memory.");
  if (niter>1) {
    w = mw_change_cimage(NULL,u->nrow,u->ncol);
    if (!w) mwerror(FATAL,1,"Not enough memory.");
  } else w=NULL;
  
  action = (i?DILATION:EROSION);
  
  src=&u; 
  if (niter&1) {  /* if niter is odd */
    dst=&v;
    new=&w;
  } else {        /* if niter is even */
    dst=&w;
    new=&v;
  }
  while (niter--) {
    erodilat1(*src,*dst,shape,action);
    src=dst; dst=new; new=src;
  }
  
  if (!s) mw_delete_curve(shape);
  if (w) mw_delete_cimage(w);
  
  return v;
}
