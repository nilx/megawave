/*----------------------------MegaWave2 module----------------------------*/
/* mwcommand
name = {harris};
version = {"1.0"};
author = {"Frederic Cao"};
function = {"Harris corner detector"};
usage = {
   'k':[k=0.04]->k
      "weight of trace, default 0.04 (suggested by Harris)",
   'g':[g=1.]->g
      "standard deviation of gaussian filtering, default 1.0",
   's':[s=3]->size
      "size of neighborhood for maxima research",
   't':[t=4.]->t
      "log10(threshold) for cornerness, default 4.0",
   in->in
      "Input image",
   out<-harris
      "Corner list (Flist)"
      };
*/

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "mw.h"

extern void fderiv();
extern Fimage fsepconvol();


#define _(A,i,j) ((A)->gray[(i)*(A)->ncol+(j)])

Flist harris(in,k,g,size,t)
     Fimage in;
     float *k,*g;
     double *t;
     int *size;
{
  Fimage gradx,grady,ux2,uy2,uxuy,corner;
  Fimage Sux2,Suy2,Suxuy;
  int period=2,b=2,nsize=8,neigh;
  int ncol,nrow,i,j,m,n;
  Flist out;
  float *p,max,fzero = 0.;
  float cmax,cmin;
  
  nrow = in->nrow; ncol = in->ncol;

  /* Compute image gradient */
  gradx = mw_new_fimage();
  grady = mw_new_fimage();
  ux2 = mw_change_fimage(NULL,nrow,ncol);
  uy2 = mw_change_fimage(NULL,nrow,ncol);
  uxuy = mw_change_fimage(NULL,nrow,ncol);
  if(!gradx || !grady || !ux2 || !uy2 || !uxuy)
    mwerror(FATAL,1,"Not enough memory.\n");
  fderiv(in,NULL,NULL,NULL,NULL,gradx,grady,NULL,NULL,&fzero,&nsize);
  for(i=0;i<nrow;i++)
    for(j=0;j<ncol;j++){
      _(ux2,i,j) = _(gradx,i,j)*_(gradx,i,j);
      _(uy2,i,j) = _(grady,i,j)*_(grady,i,j);
      _(uxuy,i,j) = _(gradx,i,j)*_(grady,i,j);
    }

  /* smooth M where M = Du (Du ^t) */
  Sux2 = mw_new_fimage();
  Suy2 = mw_new_fimage();
  Suxuy = mw_new_fimage();
  if(!Sux2 || !Suy2 || !Suxuy)
    mwerror(FATAL,1,"Not enough memory.\n");
  if(*g>0.){
    Sux2 = fsepconvol(ux2,Sux2,NULL,NULL,NULL,g,&b);
    Suy2 = fsepconvol(uy2,Suy2,NULL,NULL,NULL,g,&b);
    Suxuy = fsepconvol(uxuy,Suxuy,NULL,NULL,NULL,g,&b);
  } else {
    Sux2 = ux2;
    Suy2 = uy2;
    Suxuy = uxuy;
  }

  /* compute cornerness function = det(M)-k (Tr(M))^2 */
  corner = mw_change_fimage(NULL,nrow,ncol);
  if(!corner) mwerror(FATAL,1,"Not enough memory.\n");
  for(i=0;i<nrow;i++)
    for(j=0;j<ncol;j++){
      _(corner,i,j) = ((_(Sux2,i,j)*_(Suy2,i,j))
		       -_(Suxuy,i,j)*_(Suxuy,i,j)
		       -(*k)*((_(Sux2,i,j)+_(Suy2,i,j))
			      *(_(Sux2,i,j)+_(Suy2,i,j))));
    }



  /*compute and store maxima of cornerness */
  out = mw_change_flist(NULL,nrow*ncol,0,2);
  p = out->values;
  neigh = *size/2;
  for(i=0;i<nrow;i++)
    for(j=0;j<ncol;j++){
      max = -1e37;
      for(m=-neigh;m<=neigh;m++)
	for(n=-neigh;n<=neigh;n++){
	  if((m!=0 || n!=0) && i+m>=0 && i+m<nrow && j+n>=0 && j+n<=ncol){
	    if(_(corner,i+m,j+n)>max)
	      max = _(corner,i+m,j+n);
	  }
	}
      if(_(corner,i,j)>(float) pow(10.,*t) && _(corner,i,j)>max){
	/* we have here a strict local maximum */
	*(p++) = j+0.5;
	*(p++) = i+0.5;
	out->size++;
      }
    }
  return(out);
}
