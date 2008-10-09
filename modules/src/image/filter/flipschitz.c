/*--------------------------- MegaWave2 module  -----------------------------*/
/* mwcommand
 name = {flipschitz};
 author = {"Lionel Moisan"};
 version = {"1.1"};
 function = {"Lipschitz sup (or inf) enveloppe of an image"};
 usage = {
  'i'->i          "compute inf enveloppe (instead of sup)",
  's':s->s        "if set, the shape s is taken as structuring element",
  'r':[r=1.0]->r  "otherwise, a disc of radius r is used",
  'n':[n=1]->n    "number of iterations",
  in->in          "input Fimage",
  lip->lip        "lipschitz constant (0 means dilation/erosion)",
  out<-out        "output Fimage"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"

/* NOTE: calling this module with in=out is possible */


extern Curve disc();


/*------------- one step -------------*/

void step(u,v,s,delta,i_flag)
     Fimage u,v;
     Curve  s;
     float *delta;
     char *i_flag;
{
  int x,y,nx,ny,k,xx,yy;
  float new,curr;
  Point_curve p;

  nx = u->ncol; ny = u->nrow;

  if (i_flag) { /* inf enveloppe */

    for (x=0;x<nx;x++)
      for (y=0;y<ny;y++) {
	new = u->gray[y*nx+x];
	for (p=s->first,k=0;p;p=p->next,k++) {
	  xx = x+p->x;
	  yy = y+p->y;
	  if (xx>=0 && xx<nx && yy>=0 && yy<ny) {
	    curr = u->gray[yy*nx+xx]+delta[k];
	    if (curr<new) new=curr; 
	  }
	}
	v->gray[y*nx+x] = new;
      }
    
  } else { /* sup enveloppe */
    
    for (x=0;x<nx;x++)
      for (y=0;y<ny;y++) {
	new = u->gray[y*nx+x];
	for (p=s->first,k=0;p;p=p->next,k++) {
	  xx = x+p->x;
	  yy = y+p->y;
	  if (xx>=0 && xx<nx && yy>=0 && yy<ny) {
	    curr = u->gray[yy*nx+xx]-delta[k];
	    if (curr>new) new=curr; 
	  }
	}
	v->gray[y*nx+x] = new;
      }

  }
}
 
/*---------------------- Main function : lipschitz --------------------*/

Fimage flipschitz(in,lip,out,r,s,n,i)
     Fimage in,out;
     float lip,*r;
     char *i;
     int *n;
     Curve s;
{
  Fimage w,*src,*dst,*new;
  Curve  shape;
  int    niter,k;
  float  *delta;
  Point_curve p;
  
  /* check options and allocate memory */
  
  if (*n<=0) mwerror(FATAL,1,"The -n option parameter must be positive.");
  else niter=*n;
  
  if (*r<=0) mwerror(FATAL,1,"Radius parameter must be positive.");
  if (s) shape = s; else shape = disc(*r,NULL); 
  
  for (p=shape->first,k=0;p;p=p->next,k++);
  delta = (float *)malloc(k*sizeof(float));
  for (p=shape->first,k=0;p;p=p->next,k++) 
    delta[k] = lip * sqrt((double) p->y * (double) p->y
			  + (double)p->x * (double) p->x);

  out = mw_change_fimage(out,in->nrow,in->ncol);
  if (!out) mwerror(FATAL,1,"Not enough memory.");
  if (niter>1) {
    w = mw_change_fimage(NULL,in->nrow,in->ncol);
    if (!w) mwerror(FATAL,1,"Not enough memory.");
  } else w=NULL;
  
  src=&in; 
  if (niter&1) {  /* if niter is odd */
    dst=&out;
    new=&w;
  } else {        /* if niter is even */
    dst=&w;
    new=&out;
  }
  while (niter--) {
    step(*src,*dst,shape,delta,i);
    src=dst; dst=new; new=src;
  }
  
  free(delta);
  if (!s) mw_delete_curve(shape);
  if (w) mw_delete_fimage(w);
  
  return(out);
}
