/*--------------------------- MegaWave2 module  -----------------------------*/
/* mwcommand
 name = {fsepconvol};
 author = {"Lionel Moisan"};
 version = {"1.0"};
 function = {"Convolution with a separable 2D-kernel)"};
 usage = {
   'c':width->width           
    "unit mass constant kernel of specified width",
   'g':std->std[0.0,1.0e4]    
    "unit mass Gaussian kernel with standart deviation std (half-width 4*std)",
   in->in        "input Fimage",
   out<-out      "output (convolved Fimage)",
 { xker->xker    "use a Fsignal (and its shift) as kernel along x axis",
   yker->yker    "use a Fsignal (and its shift) as kernel along y axis" }
};
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

#ifdef __STDC__
extern void sconst(int*,float*,Fsignal);
extern void sgauss(float*,int*,Fsignal);
#else
extern void sconst();
extern void sgauss();
#endif


void fsepconvol(in,out,xker,yker,width,std)
Fimage in,out;
Fsignal xker,yker;
int *width;
float *std;
{
  Fimage tmp;
  int nx,ny,x,y,org,i,iter;
  float value,sum;
  int delete_xker;

  /* check options */
  if ((xker?1:0) + (width?1:0) + (std?1:0) != 1)
    mwerror(USAGE,1,"Please specify exactly one kernel");

  if (!xker) {
    xker = mw_new_fsignal();
    delete_xker = 1;
  } else delete_xker = 0;

  nx = in->ncol;
  ny = in->nrow;
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
    sgauss(std,NULL,xker);
    yker = xker;
  }

  /* convolution along x axis */
  org = -(int)xker->shift;
  for (y=ny;y--;) for (x=nx;x--;) {
    for (sum=0.0,i=xker->size;i--;) 
      sum += xker->values[i]*in->gray[y*nx+(x+nx*100-i+org)%nx];
    tmp->gray[y*nx+x] = sum;
  }
  
  /* convolution along y axis */
  org = -(int)yker->shift;
  for (y=ny;y--;) for (x=nx;x--;) {
    for (sum=0.0,i=yker->size;i--;) 
      sum += yker->values[i]*tmp->gray[((y+ny*100-i+org)%ny)*nx+x];
    out->gray[y*nx+x] = sum;
  }

  /* free memory */
  mw_delete_fimage(tmp);
  if (delete_xker) 
    mw_delete_fsignal(xker);
}
