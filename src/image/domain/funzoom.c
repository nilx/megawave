/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {funzoom};
   version = {"2.3"};
   author = {"Lionel Moisan"};
   function = {"Image reduction by projection on a B-spline space"};
   usage = {  
'z':[z=2.0]->z     "unzoom factor (default 2.0)",
'x':tx->tx         "to first translate (x) the original image",
'y':ty->ty         "to first transalte (y) the original image",
'o':[o=0]->o       "spline space order, 0..5, default 0",
in->in             "input Fimage",
out<-out           "output Fimage"
};
*/
/*----------------------------------------------------------------------
 v2.3: fixed image border bug (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

extern void finvspline();
extern Fimage fdirspline();


/* NB : calling this module with out=in is possible */

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
double ipow(x,n)
     double x;
     int n;
{
  double res;

  for (res=1.;n;n>>=1) {
    if (n&1) res*=x;
    x*=x;
  }
  return(res);
}

/* binomial coefficient */
double binom(k,n)
     int k,n;
{
  int i;
  double v;

  for (i=0,v=1.;i<k;i++) v*=(double)(n-i)/(double)(k-i);
  return v;
}


/*------------ SMALL piecewise polynomial functions LIBRARY ------------*/


/* piecewise polynomial function (0 outside its domain) */
#define PPMAXKNOTS 50
#define PPMAXDEG   50

typedef struct ppfunction {
  int n;                /* number of knot points */
  float x[PPMAXKNOTS];  /* knot points (in increasing order) */
  int k;                /* number of coefficients for each polynomial */
  double a[PPMAXKNOTS][PPMAXDEG];  
                        /* a[i][j] X^j contributes between x[i] and x[i+1] */
} *Ppfunction;

/* print algebraic description of Ppfunction */
void ppprint(f)
     Ppfunction f;
{
  int i,j;

  printf("%d knots\n",f->n);
  for (i=0;i<f->n-1;i++) {
    printf("interval [%f,%f]\n",f->x[i],f->x[i+1]);
    printf("%f ",f->a[i][0]);
    for (j=1;j<f->k;j++) 
      printf("+ %f X^%d",f->a[i][j],j);
    printf("\n\n");
  }
}

/* find the interval [x(i),x(i+1)] corresponding to a given point */
/* convention : i=-1 if the interval is unbounded */
int ppfind(f,x)
     Ppfunction f;
     double x;
{
  int i;

  if (f->n==0 || x<f->x[0]) return(-1);
  for (i=1;i<f->n && x>=f->x[i];i++);
  if (i==f->n) return(-1); else return(i-1);
}

/* eval a Ppfunction at a given point x (Horner algorithm) */
double ppeval(f,x)
     Ppfunction f;
     double x;
{
  int i,j;
  double v;

  i = ppfind(f,x);
  if (i==-1) return(0.);
  for (j=f->k,v=0.;j--;) 
    v = v*x + f->a[i][j];
  return(v);
}
    
/* add integral between x[i] and t-w (sign=-1) 
             or between x[i] and t+w (sign=1) */
void ppaddint(f,g,i,sign,w)
     Ppfunction f,g;
     int i,sign;
     double w;
{
  double x,v;
  int n,j,i0,k;
  
  i0 = (sign==-1?i-1:i);
  if (i0<0 || i0>=f->n-1) return;

  x = (double)(f->x[i]);
  n = g->n-2;
  
  for (j=0;j<f->k;j++) {
    v = (double)sign*(double)(f->a[i0][j])/(double)(j+1);
    for (k=0;k<=j+1;k++) 
       g->a[n][k] += v*binom(k,j+1)*ipow(w*(double)sign,j+1-k);
    g->a[n][0] -= v*ipow(x,j+1);
  }
}

/* add integral between x[i1] and x[i2] */
void ppaddcte(f,g,i1,i2)
     Ppfunction f,g;
     int i1,i2;
{
  double x,y;
  int n,i,j;
  
  if (i1<0 || i1>=i2 || i2>f->n-1) return;

  n = g->n-2;
  
  for (j=0;j<f->k;j++) {
    y = ipow( (double)(f->x[i1]),j+1 );
    for (i=i1;i<i2;i++) {
      x = y;
      y = ipow( (double)(f->x[i+1]),j+1 );
      g->a[n][0] += (double)(f->a[i][j])*(y-x)/(double)(j+1);
    }
  }
}


/* convolution of a piecewise polynomial function */
/* by a symmetric pulse of width 2*w and height 1 */
void ppconvol(f,g,w)
     Ppfunction f,g;
     double w;
{
  float x1,x2;
  int i1,i2;

  /*  memset((void *)g->a,0,600*sizeof(double));*/
  for (i1=0;i1<PPMAXKNOTS;i1++)
    for (i2=0;i2<PPMAXDEG;i2++)
      g->a[i1][i2]=0.;
    
  g->n = 0; g->k = f->k+1;
  
  /* i1 is the left index of the interval containing x-w */
  /* i2 is the left index of the interval containing x+w */
  i1 = i2 = -1; /* we start with x = - infinity */

  while (i1<f->n-1) {
    /* determine next knot point */
    x1 = f->x[i1+1]+w;
    if (i2<f->n-1) x2=f->x[i2+1]-w; else x2=x1+1.;

    if (x1<=x2) {
      /* right bound encountered on the left */
      g->x[g->n++]=x1;
      if (g->n>1) {
	ppaddint(f,g,i1+1,-1,w);
	ppaddcte(f,g,i1+1,i2);
	ppaddint(f,g,i2,1,w);
      }
      i1++;
      if (x2==x1) i2++;
    } else {
      /* right bound encountered on the right */
      g->x[g->n++]=x2;
      if (g->n>1) {
	ppaddint(f,g,i1+1,-1,w);
	ppaddcte(f,g,i1+1,i2);
	ppaddint(f,g,i2,1,w);
      }
      i2++;
    }
  }
}

/* build spline of order o : W1^o */
void ppspline(f,o)
     Ppfunction f;
     int o;
{
  struct ppfunction g;
  Ppfunction f1,f2,f3;
  int i;

  /* init */
  f->n=2; f->x[0]=-0.5; f->x[1]=0.5; f->k=1; f->a[0][0]=1.;
  f1=f; f2=&g;

  for (i=0;i<o;i++) {
    ppconvol(f1,f2,0.5);
    f3=f1; f1=f2; f2=f3;
  }

  if (f!=f1) *f=g;
}

/* build unzoom kernel of order o : (W1*Wz)^o */
void ppkernel(f,o,z)
     Ppfunction f;
     int o;
     float z;
{
  struct ppfunction g;
  Ppfunction f1,f2,f3;
  int i;

  /* init */
  f->n=2; f->x[0]=-0.5; f->x[1]=0.5; f->k=1; f->a[0][0]=1.;
  for (i=1;i<=o+1;i++) f->a[0][0]/=z;

  f1=f; f2=&g;

  for (i=0;i<o;i++) {
    ppconvol(f1,f2,0.5);
    f3=f1; f1=f2; f2=f3;
  }
  for (i=0;i<=o;i++) {
    ppconvol(f1,f2,0.5*z);
    f3=f1; f1=f2; f2=f3;
  }
  
  if (f!=f1) *f=g;
}


/*------------------------ MAIN MODULE ---------------------------------*/

Fimage funzoom(in,out,z,o,tx,ty)
     Fimage in,out;
     float *z,*tx,*ty;
     int *o;
{
  int i,d,n,x,y,nx,ny,nsx,nsy;
  float *c,p,u,res,delta;
  Fimage tmp,in0,out0;
  struct ppfunction xi;

  if (*z<1.) 
    mwerror(FATAL,1,"illegal unzoom factor (%f) - should be at least 1.\n",*z);

  if (*o<0 || *o>5) 
    mwerror(USAGE,0,"spline space order should be in [0..5]\n");

  nx = in->ncol; ny = in->nrow;
  nsx = (int)rint((double)nx/(double)(*z));
  nsy = (int)rint((double)ny/(double)(*z));
  out = mw_change_fimage(out,nsy,nsx);
  
  tmp = mw_change_fimage(NULL,ny,nsx);
  
  n = (int)ceil(0.5*(double)(*z+1.)*(float)(*o+1));
  c = (float *)malloc((2*n+1)*sizeof(float));


  /********** PRE-FILTERING **********/

  if (*o>1) {
    in0 = mw_change_fimage(NULL,ny,nx);
    finvspline(in,*o,in0); 
  } else in0 = in;


  /********** CONVOLUTION AND SAMPLING **********/

  /* compute convolution kernel xi = (W1*Wz)^{n+1} */
  ppkernel(&xi,*o,*z);

  if (*o==0) out0 = out;
  else out0 = mw_change_fimage(NULL,nsy,nsx);

  /* FIRST LOOP (x) */

  delta = (tx?*tx:0.);
  for (x=0;x<nsx;x++) {
    p = *z*((float)x+0.5)-0.5-delta;
    i = (int)floor((double)p); 
    u = p-(float)i;
    for (d=-n;d<=n;d++) 
      c[n+d]=(float)ppeval(&xi,u+(float)d);
    for (y=0;y<ny;y++) {
      /* this test saves computation time */
      if (i-n>=0 && i+n<nx)
	for (d=-n,res=0.;d<=n;d++)
	  res += c[n+d]*in0->gray[y*nx+i-d];
      else 
	for (d=-n,res=0.;d<n;d++)
	  res += c[n+d]*v(in0,i-d,y);
      tmp->gray[y*nsx+x] = res;
    }
  }
  
  /* SECOND LOOP (y) */

  delta = (ty?*ty:0.);
  for (y=0;y<nsy;y++) {
    p = *z*((float)y+0.5)-0.5-delta;
    i = (int)floor((double)p); 
    u = p-(float)i;
    for (d=-n;d<=n;d++) 
      c[n+d]=(float)ppeval(&xi,u+(float)d);
    for (x=0;x<nsx;x++) {
      /* this test saves computation time */
      if (i-n>=0 && i+n<ny)
	for (d=-n,res=0.;d<=n;d++)
	  res += c[n+d]*tmp->gray[(i-d)*nsx+x];
      else 
	for (d=-n,res=0.;d<=n;d++)
	  res += c[n+d]*v(tmp,x,i-d);
      out0->gray[y*nsx+x] = res;
    }
  }

  /********** POST-FILTERING **********/
  if (*o>=1) {
    if (*o>1) {
      finvspline(out0,*o*2+1,out0);
      fdirspline(out0,*o,out);
      mw_delete_fimage(in0);
    } else finvspline(out0,*o*2+1,out);
    mw_delete_fimage(out0);
  }
  mw_delete_fimage(tmp);

  return(out);
}


