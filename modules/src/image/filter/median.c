/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {median};
  version = {"2.5"};
  author = {"Lionel Moisan"};
  function = {"apply discrete median to a Cimage"};
  usage = {
     's':s->s         "if set, the shape s is taken as structuring element",
     'r':[r=1.0]->r   "otherwise, a disc of radius r is used",
     'n':[n=1]->n     "number of iterations",
     in->u            "input Cimage",
     out<-v           "output Cimage"
          };
*/
/*----------------------------------------------------------------------
 v2.5 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include "mw.h"

extern Curve disc();


/*--------------- Median of a cimage : one iteration  -------------*/

static int comp(i,j)
     int *i,*j;
{ 
  return ((int)*((unsigned char *)i)-(int)*((unsigned char *)j));
}

int median1(u,v,s)
     Cimage u,v;
     Curve  s;
{
  int           ir,changed;
  register int  i,x,y,nx,ny,xx,yy;
  unsigned char curr[10000];
  Point_curve   p;
  long          adr;
  
  nx = u->ncol; 
  ny = u->nrow;
  changed = 0;
  for (x=0;x<nx;x++)
    for (y=0;y<ny;y++) {
      i=0;
      for (p=s->first;p;p=p->next) {
	xx = x+p->x;
	yy = y+p->y;
	if (xx>=0 && xx<nx && yy>=0 && yy<ny) 
	  curr[i++] = u->gray[yy*nx+xx];		  
      }
      qsort((char *)curr,i,sizeof(char),comp);
      adr = y*nx+x;
      v->gray[adr] = curr[i/2];
      if (v->gray[adr]!=u->gray[adr]) changed++;
    }
  return (changed);
}

/*---------------------- Main function : median --------------------*/

Cimage median(u, v, r, s, n)
     Cimage u,v;
     float  *r;
     Curve  s;
     int    *n;
{
  Cimage w,*src;
  int    i,changed;
  Curve  shape;
  
  /* check options and allocate memory */
  if (*n<=0) mwerror(FATAL,1,"The -n option parameter must be positive.");
  
  if (*r<=0) mwerror(FATAL,1,"Radius parameter must be positive.");
  if (s) shape = s; else shape = disc(*r,NULL); 
  
  v = mw_change_cimage(v,u->nrow,u->ncol);
  if (!v) mwerror(FATAL,1,"Not enough memory.");
  if (*n!=1) {
    w = mw_change_cimage(NULL,u->nrow,u->ncol);
    if (!w) mwerror(FATAL,1,"Not enough memory.");
  } else w=NULL;
  
  src =&u; 
  changed = TRUE;
  for (i=0;i<*n && changed;i++) { 
    if (src!=&u) mw_copy_cimage(v,*src);
    changed = median1(*src,v,shape);
    mwdebug("iteration %d : %d grey level(s) modified\n",i+1,changed);
    src = &w;
  }
  
  /* print number of iteration if median blocked */
  if (!changed) 
    printf("median blocked after %d effective iteration(s)\n",i-1);
  
  /* free memory */
  if (!s) mw_delete_curve(shape);
  if (w) mw_delete_cimage(w);
  
  return v;
}
