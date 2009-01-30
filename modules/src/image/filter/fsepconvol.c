/*--------------------------- MegaWave2 module  -----------------------------*/
/* mwcommand
 name = {fsepconvol};
 author = {"Lionel Moisan"};
 version = {"1.2"};
 function = {"Convolution with a separable 2D-kernel"};
 usage = {
   'c':width->width           
    "unit mass constant kernel of specified width",
   'g':std->std[0.0,1.0e4]    
    "Gaussian kernel, standart deviation std (precision 1e-3)",
   'b':[b=1]->b  "boundary condition: 0=zero, 1=symmetry, 2=torus",
   in->in        "input Fimage",
   out<-out      "output (convolved Fimage)",
 { xker->xker    "use a Fsignal (and its shift) as kernel along x axis",
   yker->yker    "use a Fsignal (and its shift) as kernel along y axis" }
};
*/
/*----------------------------------------------------------------------
 v1.2: new sgauss(), -b option, acceleration, return output (L.Moisan)
----------------------------------------------------------------------*/

/* NOTE: calling this module with in=out is possible */

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for sconst(), sgauss() */

Fimage fsepconvol(Fimage in, Fimage out, Fsignal xker, Fsignal yker, int *width, float *std, int *b)
{
  Fimage tmp;
  int nx,nx2,ny,ny2,x,y,org,i,s;
  float value,sum,precision;
  int delete_xker;

  /* check options */
  if ((xker?1:0) + (width?1:0) + (std?1:0) != 1)
    mwerror(USAGE,1,"Please specify exactly one kernel");

  if (!xker) {
    xker = mw_new_fsignal();
    delete_xker = 1;
  } else delete_xker = 0;

  nx = in->ncol; nx2 = 2*nx;
  ny = in->nrow; ny2 = 2*ny;
  tmp = mw_change_fimage(NULL,ny,nx);
  out = mw_change_fimage(out,ny,nx);
  
  /*** COMPUTE KERNEL IF NEEDED ***/

  if (width) {
    /* constant kernel */
    value = 1.0/(float)(*width);
    sconst(width,&value,xker);
    xker->shift = -0.5*(float)(*width);
    yker = xker;
  }
  if (std) {
    /* Gaussian kernel */
    precision = 3.;
    sgauss(*std,xker,NULL,&precision);
    yker = xker;
  }

  /* convolution along x axis */
  org = -(int)xker->shift;
  for (y=ny;y--;) for (x=nx;x--;) {
    for (sum=0.0,i=xker->size;i--;) {
      s = x-i+org;
      switch(*b) {
      case 0: 
	if (s>=0 && s<nx) sum += xker->values[i]*in->gray[y*nx+s];
	break;
      case 1: 
	while (s<0) s+=nx2;
	while (s>=nx2) s-=nx2;
	if (s>=nx) s = nx2-1-s;
	sum += xker->values[i]*in->gray[y*nx+s];
	break;
      case 2: 
	while (s<0) s+=nx;
	while (s>=nx) s-=nx;
	sum += xker->values[i]*in->gray[y*nx+s];
	break;
      }
    }
    tmp->gray[y*nx+x] = sum;
  }
  
  /* convolution along y axis */
  org = -(int)yker->shift;
  for (y=ny;y--;) for (x=nx;x--;) {
    for (sum=0.0,i=yker->size;i--;) {
      s = y-i+org;
      switch(*b) {
      case 0: 
	if (s>=0 && s<ny) sum += yker->values[i]*tmp->gray[s*nx+x];
	break;
      case 1: 
	while (s<0) s+=ny2;
	while (s>=ny2) s-=ny2;
	if (s>=ny) s = ny2-1-s;
	sum += yker->values[i]*tmp->gray[s*nx+x];
	break;
      case 2: 
	while (s<0) s+=ny;
	while (s>=ny) s-=ny;
	sum += yker->values[i]*tmp->gray[s*nx+x];
	break;
      }
    }
    out->gray[y*nx+x] = sum;
  }

  /* free memory */
  mw_delete_fimage(tmp);
  if (delete_xker) 
    mw_delete_fsignal(xker);

  return(out);
}
