/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fcrop};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"image croping (with zoom) using interpolation"};
   usage = {  
'x':sx->sx         "force x-size of output image",
'y':sy->sy         "force y-size of output image",
'z':z->z           "zoom factor (default 1.0)",
'b':[bg=0.0]->bg   "background grey value, default: 0.0",
'o':[o=3]->o       "order: 0,1=linear,-3=cubic,3,5..11=spline, default 3",
'p':p->p           "Keys' parameter (when o=-3), in [-1,0], default -0.5",
in->in             "input Fimage",
out<-out           "output Fimage",
X1->X1             "upleft corner",
Y1->Y1             "upleft corner",
X2->X2             "downright corner",
Y2->Y2             "downright corner"
};
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

extern void finvspline();


/* NB : calling this module with out=in is possible */

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

Fimage fcrop(in,out,sx,sy,z,bg,o,p,X1,Y1,X2,Y2)
     Fimage in,out;
     float  *sx,*sy,*z,*p,*bg,X1,Y1,X2,Y2;
     int    *o;
{
  int    nsx,nsy,n1,n2,nx,ny,x,y,xi,yi,adr,d;
  float  zx,zy,res,xp,yp,u,c[12],ak[13];
  Fimage ref,tmp,coeffs;

  /* CHECK ORDER */
  if (*o!=0 && *o!=1 && *o!=-3 && 
      *o!=3 && *o!=5 && *o!=7 && *o!=9 && *o!=11)
    mwerror(FATAL,1,"unrecognized interpolation order.\n");

  /* COMPUTE OUTPUT DIMENSIONS AND ZOOM FACTORS */
  if (!sx && !sy && !z) zx=zy=1.;
  if (sx) zx = *sx/(X2-X1);
  if (sy) zy = *sy/(Y2-Y1);
  if (sx && !sy) zy = zx;
  if (!sx && sy) zx = zy;
  if (z) zx = zy = *z;
  nsx = (int)rint((double)zx*(double)(X2-X1));
  nsy = (int)rint((double)zy*(double)(Y2-Y1));
  mwdebug("Output size is %d x %d\n",nsx,nsy);
  if (nsx<0 || nsy<0) mwerror(FATAL,1,"illegal window specification.\n");

  nx = in->ncol; ny = in->nrow;
  
  if (*o>=3) {
    coeffs = mw_new_fimage();
    finvspline(in,*o,coeffs);
    ref = coeffs;
    if (*o>3) init_splinen(ak,*o);
  } else {
    coeffs = NULL;
    ref = in;
  }

  tmp = mw_change_fimage(NULL,ny,nsx);

  /********** FIRST LOOP (x) **********/
  
  for (x=0;x<nsx;x++) {

    xp = X1+( (float)x + 0.5 )/zx;

    if (*o==0) { /* zero order interpolation (pixel replication) */

      xi = (int)floor((double)xp); 
      if (xi<0 || xi>=nx)
	for (y=0;y<ny;y++) tmp->gray[y*nsx+x] = *bg; 
      else for (y=0;y<ny;y++) tmp->gray[y*nsx+x] = ref->gray[y*nx+xi];
      
    } else { /* higher order interpolations */

      if (xp<0. || xp>(float)nx) 
	for (y=0;y<ny;y++) tmp->gray[y*nsx+x]=*bg; 

      else {

	xp -= 0.5;
	xi = (int)floor((double)xp); 
	u = xp-(float)xi;
	switch (*o) 
	  {
	  case 1: /* first order interpolation (bilinear) */
	    n2 = 1; c[0]=u; c[1]=1.-u; break;
	    
	  case -3: /* third order interpolation (bicubic Keys' function) */
	    n2 = 2; keys(c,u,(p?*p:-0.5)); break;
	    
	  case 3: /* spline of order 3 */
	    n2 = 2; spline3(c,u); break;
	    
	  default: /* spline of order >3 */
	    n2 = (1+*o)/2; splinen(c,u,ak,*o); break;
	  }
	
	n1 = 1-n2;
	/* this test saves computation time */
	if (xi+n1>=0 && xi+n2<nx) {
	  for (y=0;y<ny;y++) {
	    for (d=n1,res=0.;d<=n2;d++) 
	      res += c[n2-d]*ref->gray[y*nx+xi+d];
	    tmp->gray[y*nsx+x] = res;
	  }
	} else 
	  for (y=0;y<ny;y++) {
	    for (d=n1,res=0.;d<=n2;d++) 
	      res += c[n2-d]*v(ref,xi+d,y,*bg);
	    tmp->gray[y*nsx+x] = res;
	  }
      }
    }
  }
  
  ref = tmp;
  out = mw_change_fimage(out,nsy,nsx);
  if (!out) mwerror(FATAL,1,"not enough memory\n");

  /********** SECOND LOOP (y) **********/
  
  for (y=0;y<nsy;y++) {

    yp = Y1+( (float)y + 0.5 )/zy;

    if (*o==0) { /* zero order interpolation (pixel replication) */

      yi = (int)floor((double)yp); 
      if (yi<0 || yi>=ny)
	for (x=0;x<nsx;x++) out->gray[y*nsx+x] = *bg; 
      else for (x=0;x<nsx;x++) out->gray[y*nsx+x] = ref->gray[yi*nsx+x];
      
    } else { /* higher order interpolations */

      if (yp<0. || yp>(float)ny) 
	for (x=0;x<nsx;x++) out->gray[y*nsx+x]=*bg; 

      else {

	yp -= 0.5;
	yi = (int)floor((double)yp); 
	u = yp-(float)yi;
	switch (*o) 
	  {
	  case 1: /* first order interpolation (bilinear) */
	    n2 = 1; c[0]=u; c[1]=1.-u; break;
	    
	  case -3: /* third order interpolation (bicubic Keys' function) */
	    n2 = 2; keys(c,u,(p?*p:-0.5)); break;
	    
	  case 3: /* spline of order 3 */
	    n2 = 2; spline3(c,u); break;
	    
	  default: /* spline of order >3 */
	    n2 = (1+*o)/2; splinen(c,u,ak,*o); break;
	  }
	
	n1 = 1-n2;
	/* this test saves computation time */
	if (yi+n1>=0 && yi+n2<ny) {
	  for (x=0;x<nsx;x++) {
	    for (d=n1,res=0.;d<=n2;d++) 
	      res += c[n2-d]*ref->gray[(yi+d)*nsx+x];
	    out->gray[y*nsx+x] = res;
	  }
	} else 
	  for (x=0;x<nsx;x++) {
	    for (d=n1,res=0.;d<=n2;d++) 
	      res += c[n2-d]*v(ref,x,yi+d,*bg);
	    out->gray[y*nsx+x] = res;
	  }
      }
    }
  }
  mw_delete_fimage(tmp);
  if (coeffs) mw_delete_fimage(coeffs);

  return(out);
}

