/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {fdirspline};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"2D direct B-spline transform"};
  usage = {
     in->in               "input (Fimage)",
     n->n                 "spline order",
     out<-out             "output (Fimage of coefficients)"
          };
*/

#include <stdlib.h>
#include <math.h>
#include "mw.h"

/* extract image value (symmetrize outside image domain) */
float v(in,x,y)
     Fimage in;
     int x,y;
{
  if (x<0) x=-x;
  if (x>=in->ncol) x=2*(in->ncol-1)-x;
  if (y<0) y=-y;
  if (y>=in->nrow) y=2*(in->nrow-1)-y;
  return(in->gray[y*in->ncol+x]);
}

/* fast integral power function */
float ipow(x,n)
     float x;
     int n;
{
  float res;

  for (res=1.;n;n>>=1) {
    if (n&1) res*=x;
    x*=x;
  }
  return(res);
}



/*------------------------------ MAIN MODULE ------------------------------*/

Fimage fdirspline(in,n,out)
     Fimage in,out;
     int n;
{
  Fimage tmp;
  int nx,ny,n1,n2,d,i,k,x,y;
  float a,m,*c,xn,res;
     
  if (n<0) mwerror(FATAL,1,"fdirspline: spline order cannot be negative\n");

  nx = in->ncol;
  ny = in->nrow;
  out = mw_change_fimage(out,ny,nx);
  tmp = mw_change_fimage(NULL,ny,nx);

  if (n<=1) {
    mw_copy_fimage(in,out);
    return(out);
  }

  /* compute spline coefficients c[0..n2] */
  n2 = n/2;
  c = calloc(n2+1,sizeof(float));
  for (a=1.,k=2;k<=n;k++) a/=(float)k;
  for (k=0;k<=n+1;k++) {
    for (i=0;i<=n2;i++) {
      xn = 0.5*(float)(n+1) + (float)(i-k);
      if (xn>0.) 
	c[i] += a*ipow(xn,n);
    }
    a *= -(float)(n+1-k)/(float)(k+1);
  }

  /* FIRST LOOP (x) */
  for (x=0;x<nx;x++) 
    /* this test saves computation time */
    if (x-n2>=0 && x+n2<nx) 
      for (y=0;y<ny;y++) {
	res = c[0]*in->gray[y*nx+x];
	for (d=1;d<=n2;d++) 
	  res += c[d]*(in->gray[y*nx+x+d]+in->gray[y*nx+x-d]);
	tmp->gray[y*nx+x] = res;
      }
    else 
      for (y=0;y<ny;y++) {
	res = c[0]*in->gray[y*nx+x];
	for (d=1;d<=n2;d++) 
	  res += c[d]*(v(in,x+d,y)+v(in,x-d,y));
	tmp->gray[y*nx+x] = res;
      }

  /* SECOND LOOP (y) */
  n1 = 1-n2;
  for (y=0;y<ny;y++) 
    /* this test saves computation time */
    if (y-n2>=0 && y+n2<ny) 
      for (x=0;x<nx;x++) {
	res = c[0]*tmp->gray[y*nx+x];
	for (d=1;d<=n2;d++) 
	  res += c[d]*(tmp->gray[(y+d)*nx+x]+tmp->gray[(y-d)*nx+x]);
	out->gray[y*nx+x] = res;
      }
    else 
      for (x=0;x<nx;x++) {
	res = c[0]*tmp->gray[y*nx+x];
	for (d=1;d<=n2;d++) 
	  res += c[d]*(v(tmp,x,y+d)+v(tmp,x,y+d));
	out->gray[y*nx+x] = res;
      }

  mw_delete_fimage(tmp);
  free(c);

  return(out);
}
