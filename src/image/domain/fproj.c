/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {fproj};
   version = {"2.1"};
   author = {"Lionel Moisan"};
   function = {"affine or projective mapping using interpolation"};
   usage = {  
     'x':[sx=512]->sx   "x-size of output image",
     'y':[sy=512]->sy   "y-size of output image",
     'b':[bg=0.0]->bg   "background grey value",
     'o':[o=3]->o       "order: 0,1=linear,-3=cubic,3,5..11=spline",
     'p':[p=-.5]->p     "Keys' parameter (when o=-3), in [-1,0]",
     'i'->i             "compute inverse transform",
     in->in             "input Fimage",
     out<-out           "output Fimage",
     X1->X1             "upleft corner",
     Y1->Y1             "upleft corner",
     X2->X2             "upright corner",
     Y2->Y2             "upright corner",
     X3->X3             "downleft corner",
     Y3->Y3             "downleft corner",
  {
     x4->x4             "downright corner (for projective transform)",
     y4->y4             "downright corner (for projective transform)"
  }
};
*/
/*----------------------------------------------------------------------
 v2.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

extern void finvspline();


/* NB : calling this module with out=in is not possible */

/* extract image value (even outside image domain) */
float v(in,x,y,bg)
     Fimage in;
     int x,y;
     float bg;
{
  if (x<0 || x>=in->ncol || y<0 || y>=in->nrow)
    return(bg); else return(in->gray[y*in->ncol+x]);
}


/* c[] = values of interpolation function at ...,t-2,t-1,t,t+1,... */

/* coefficients for cubic interpolant (Keys' function) */
void keys(c,t,a)
     float *c,t,a;
{
  float t2,at;

  t2 = t*t;
  at = a*t;
  c[0] = a*t2*(1.0-t);
  c[1] = (2.0*a+3.0 - (a+2.0)*t)*t2 - at;
  c[2] = ((a+2.0)*t - a-3.0)*t2 + 1.0;
  c[3] = a*(t-2.0)*t2 + at;
}

/* coefficients for cubic spline */
void spline3(c,t)
     float *c,t;
{
  float tmp;

  tmp = 1.-t;
  c[0] = 0.1666666666*t*t*t;
  c[1] = 0.6666666666-0.5*tmp*tmp*(1.+t);
  c[2] = 0.6666666666-0.5*t*t*(2.-t);
  c[3] = 0.1666666666*tmp*tmp*tmp;
}

/* pre-computation for spline of order >3 */
void init_splinen(a,n)
     float *a;
     int n;
{
  int k;

  a[0] = 1.;
  for (k=2;k<=n;k++) a[0]/=(float)k;
  for (k=1;k<=n+1;k++)
    a[k] = - a[k-1] *(float)(n+2-k)/(float)k;
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

/* coefficients for spline of order >3 */
void splinen(c,t,a,n)
     float *c,t,*a;
     int n;
{
  int i,k;
  float xn;
  
  memset((void *)c,0,(n+1)*sizeof(float));
  for (k=0;k<=n+1;k++) { 
    xn = ipow(t+(float)k,n);
    for (i=k;i<=n;i++) 
      c[i] += a[i-k]*xn;
  }
}


/*------------------------ MAIN MODULE ---------------------------------*/

void fproj(in,out,sx,sy,bg,o,p,i,X1,Y1,X2,Y2,X3,Y3,x4,y4)
     Fimage in,out;
     int    *sx,*sy,*o;
     char   *i;
     float  *bg,*p,X1,Y1,X2,Y2,X3,Y3,*x4,*y4;
{
  int    n1,n2,nx,ny,x,y,xi,yi,adr,dx,dy;
  float  res,xx,yy,xp,yp,ux,uy,a,b,d,fx,fy,x12,x13,y12,y13;
  float  cx[12],cy[12],ak[13];
  Fimage ref,coeffs;

  /* CHECK ORDER */
  if (*o!=0 && *o!=1 && *o!=-3 && 
      *o!=3 && *o!=5 && *o!=7 && *o!=9 && *o!=11)
    mwerror(FATAL,1,"unrecognized interpolation order.\n");

  /* ALLOCATE NEW IMAGE */
  nx = in->ncol; ny = in->nrow;
  out = mw_change_fimage(out,*sy,*sx);
  if (!out) mwerror(FATAL,1,"not enough memory\n");
  
  if (*o>=3) {
    coeffs = mw_new_fimage();
    finvspline(in,*o,coeffs);
    ref = coeffs;
    if (*o>3) init_splinen(ak,*o);
  } else {
    coeffs = NULL;
    ref = in;
  }

  /* COMPUTE NEW BASIS */
  if (i) {
    x12 = (X2-X1)/(float)nx;
    y12 = (Y2-Y1)/(float)nx;
    x13 = (X3-X1)/(float)ny;
    y13 = (Y3-Y1)/(float)ny;
  } else {
    x12 = (X2-X1)/(float)(*sx);
    y12 = (Y2-Y1)/(float)(*sx);
    x13 = (X3-X1)/(float)(*sy);
    y13 = (Y3-Y1)/(float)(*sy);
  }
  if (y4) {
    xx=((*x4-X1)*(Y3-Y1)-(*y4-Y1)*(X3-X1))/((X2-X1)*(Y3-Y1)-(Y2-Y1)*(X3-X1));
    yy=((*x4-X1)*(Y2-Y1)-(*y4-Y1)*(X2-X1))/((X3-X1)*(Y2-Y1)-(Y3-Y1)*(X2-X1));
    a = (yy-1.0)/(1.0-xx-yy);
    b = (xx-1.0)/(1.0-xx-yy);
  } else a=b=0.0;

  /********** MAIN LOOP **********/

  for (x=0;x<*sx;x++) 
    for (y=0;y<*sy;y++) {
      
      /* COMPUTE LOCATION IN INPUT IMAGE */
      if (i) {
	xx = 0.5+(((float)x-X1)*y13-((float)y-Y1)*x13)/(x12*y13-y12*x13);
	yy = 0.5-(((float)x-X1)*y12-((float)y-Y1)*x12)/(x12*y13-y12*x13);
	d = 1.0-(a/(a+1.0))*xx/(float)nx-(b/(b+1.0))*yy/(float)ny;
	xp = xx/((a+1.0)*d);
	yp = yy/((b+1.0)*d);
      } else {
	fx = (float)x + 0.5;
	fy = (float)y + 0.5;
	d = a*fx/(float)(*sx)+b*fy/(float)(*sy)+1.0;
	xx = (a+1.0)*fx/d;
	yy = (b+1.0)*fy/d;
	xp = X1 + xx*x12 + yy*x13;
	yp = Y1 + xx*y12 + yy*y13;
      }

      /* INTERPOLATION */
      
      if (*o==0) { 
	
	/* zero order interpolation (pixel replication) */
	xi = (int)floor((double)xp); 
	yi = (int)floor((double)yp);
	if (xi<0 || xi>=in->ncol || yi<0 || yi>=in->nrow)
	  res = *bg; else res = in->gray[yi*in->ncol+xi];
	
      } else { 
	
	/* higher order interpolations */
	if (xp<0. || xp>(float)nx || yp<0. || yp>(float)ny) res=*bg; 
	else {
	  xp -= 0.5; yp -= 0.5;
	  xi = (int)floor((double)xp); 
	  yi = (int)floor((double)yp);
	  ux = xp-(float)xi;
	  uy = yp-(float)yi;
	  switch (*o) 
	    {
	    case 1: /* first order interpolation (bilinear) */
	      n2 = 1;
	      cx[0]=ux;	cx[1]=1.-ux;
	      cy[0]=uy; cy[1]=1.-uy;
	      break;
	      
	    case -3: /* third order interpolation (bicubic Keys' function) */
	      n2 = 2;
	      keys(cx,ux,*p);
	      keys(cy,uy,*p);
	      break;

	    case 3: /* spline of order 3 */
	      n2 = 2;
	      spline3(cx,ux);
	      spline3(cy,uy);
	      break;

	    default: /* spline of order >3 */
	      n2 = (1+*o)/2;
	      splinen(cx,ux,ak,*o);
	      splinen(cy,uy,ak,*o);
	      break;
	    }
	  
	  res = 0.; n1 = 1-n2;
	  /* this test saves computation time */
	  if (xi+n1>=0 && xi+n2<nx && yi+n1>=0 && yi+n2<ny) {
	    adr = yi*nx+xi; 
	    for (dy=n1;dy<=n2;dy++) 
	      for (dx=n1;dx<=n2;dx++) 
		res += cy[n2-dy]*cx[n2-dx]*ref->gray[adr+nx*dy+dx];
	  } else 
	    for (dy=n1;dy<=n2;dy++)
	      for (dx=n1;dx<=n2;dx++) 
		res += cy[n2-dy]*cx[n2-dx]*v(ref,xi+dx,yi+dy,*bg);
	}
      }
      out->gray[y*(*sx)+x] = res;
    }
  if (coeffs) mw_delete_fimage(coeffs);
}

