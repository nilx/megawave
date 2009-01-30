/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {tvdeblur};
version = {"1.1"};
author = {"Lionel Moisan"};
function = {"Image deblurring by TV minimization"};
usage = {
 'W':[W=1.]->W     "weight on regularization term",
 's':[s=1.]->s     "initial (and maximal) time step",
 'E':[eps=1.]->eps "epsilon in sqrt(epsilon+|Du|^2)",
 'p':[p=0.5]->p    "positive coefficient for the gradient scheme",
 'e':[e=0.02]->e   "stop when |u(n)-u(n-1)|<e (L2 error)",
 'n':n->n          "or perform a fixed number n of iterations",
 'r':ref->ref      "to specify a reference image different from in",
 'v'->v            "verbose : print energy and L2 errors at each iteration",
 'c'->c            "cancel auto step reduction",
 in->in            "input Fimage",
 ker->ker          "convolution kernel (Fimage)",
 out<-out          "output Fimage"
        };
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for fnorm() */

#define STEP_FACTOR 0.8
#define STEP_LIMIT 1e-20

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))


/* global variable */
Fimage aux2;


/* compute weight * total variation and add its gradient to grad */
static double mytvgrad(Fimage u, Fimage grad, double eps, double p, double weight)
{
  int x,y,nx,ny,adr;
  double E,a,b,c,d,e,f,z,norm,q;

  q = (1.-p)/2.; p = p/2;
  nx = u->ncol; 
  ny = u->nrow;
  E = 0.;

  for (x=0;x<nx-1;x++)
    for(y=0;y<ny-1;y++) {
      adr = y*nx+x;
      a = (double)u->gray[adr     ];
      b = (double)u->gray[adr   +1];
      c = (double)u->gray[adr+nx+1];
      d = (double)u->gray[adr+nx  ];
      e=b-d; f=c-a; z=d; d-=c; c-=b; b-=a; a-=z; 
      norm = sqrt( eps + p*(a*a+b*b+c*c+d*d) + q*(e*e+f*f) );
      if (norm!=0.) {
	E += weight * norm;
	norm /= weight;
	grad->gray[adr     ] += (float)((p*(a-b)-q*f)/norm);
	grad->gray[adr   +1] += (float)((p*(b-c)+q*e)/norm);
	grad->gray[adr+nx+1] += (float)((p*(c-d)+q*f)/norm);
	grad->gray[adr+nx  ] += (float)((p*(d-a)-q*e)/norm);
      }
    }

  return(E);
}

/* Convolution v=k*u, restricted domain for v */
static void conv1(Fimage u, Fimage k, Fimage v)
{
  int kx,kx2,ky2,nx,ny,x,y,dx,dy;
  double s;

  kx = k->ncol;
  kx2 = k->ncol/2;
  ky2 = k->nrow/2;
  nx = u->ncol;
  ny = u->nrow;

  mw_clear_fimage(v,0.);

  for (y=ky2;y<ny-ky2;y++)
    for (x=kx2;x<nx-kx2;x++) {
      s = 0.;
      for (dx=-kx2;dx<=kx2;dx++) 
	for (dy=-ky2;dy<=ky2;dy++) 
	  s += (double)( u->gray[(y-dy)*nx+x-dx]*k->gray[(dy+ky2)*kx+dx+kx2] );
      v->gray[y*nx+x] = (float)s;
    }
}


/* Convolution v=k'*u, restricted domain for u (not v !) */
static void conv2(Fimage u, Fimage k, Fimage v)
{
  int kx,kx2,ky2,nx,ny,x,y,dx,dy,dxmax,dymax;
  double s;

  kx = k->ncol;
  kx2 = k->ncol/2;
  ky2 = k->nrow/2;
  nx = u->ncol;
  ny = u->nrow;

  for (y=0;y<ny;y++)
    for (x=0;x<nx;x++) {
      s = 0.;
      dxmax = MIN(kx2,nx-1-kx2-x);
      for (dx=MAX(-kx2,kx2-x);dx<=dxmax;dx++) {
	dymax = MIN(ky2,ny-1-ky2-y);
	for (dy=MAX(-ky2,ky2-y);dy<=dymax;dy++) 
	  s += (double)( u->gray[(y+dy)*nx+x+dx]
			 * k->gray[(dy+ky2)*kx+dx+kx2] );
	v->gray[y*nx+x] = (float)s;
      }
    }
}


/* Compute energy F(u) = int (K*u-u_0)^2 and its gradient */
static double fidelity_term_grad(Fimage u, Fimage k, Fimage u0, Fimage grad)
{
  int adr;
  double e,d;

  conv1(u,k,aux2);

  for (e=0.,adr=u0->ncol*u0->nrow;adr--;) {
    d = (double)aux2->gray[adr] - (double)u0->gray[adr];
    aux2->gray[adr] = 2.*(float)d;
    e += d*d;
  }

  conv2(aux2,k,grad);

  return(e);
}


/*----------------------- MAIN MODULE ---------------*/

Fimage tvdeblur(Fimage in, Fimage out, Fimage ker, double *s, char *c, char *v, double *e, int *n, double *W, Fimage ref, double *eps, double *p)
{
  Fimage dE,aux,tmp,cur,prev;
  double energy,old_energy,step;
  float two,norm2;
  int i,j,border,cont,prevok,nx,ny;

  /* initialization */
  nx = in->ncol; 
  ny = in->nrow;
  border = 0;
  two = 2.;

  dE = mw_change_fimage(NULL,ny,nx);
  out = mw_change_fimage(out,ny,nx);
  aux = mw_change_fimage(NULL,ny,nx);
  aux2 = mw_change_fimage(NULL,ny,nx);
  if (!dE || !out || !aux || !aux2) 
    mwerror(FATAL,1,"tvdeblur: not enough memory\n");

  /* init: zero order zoom or initial guess */
  mw_copy_fimage(in,out);
  mw_copy_fimage(in,aux);
  if (!ref) ref = in;
  prev = aux; cur = out; 
  i = prevok = 0; step = *s;

  /* compute initial error and its gradient */
  energy = fidelity_term_grad(cur,ker,ref,dE);
  if (*W!=0.) energy += mytvgrad(cur,dE,*eps,*p,*W);
  energy /= (double)(nx*ny);
  if (v) printf("init:  E = %f\n",energy);

  /***** MAIN LOOP *****/ 
  do {

    /* update image */
    for (j=nx*ny;j--;) 
      prev->gray[j] = cur->gray[j] - (float)step * dE->gray[j];
    
    /* flip pointers and increment */
    tmp=prev; prev=cur; cur=tmp;
    i++;
    
    /* compute error and its gradient for the next iteration */
    old_energy = energy;
    energy = fidelity_term_grad(cur,ker,ref,dE);
    if (*W!=0.) energy += mytvgrad(cur,dE,*eps,*p,*W);
    energy /= (double)(nx*ny);
    
    if (energy>old_energy) {
      tmp=prev; prev=cur; cur=tmp;
      energy = fidelity_term_grad(cur,ker,ref,dE);
      if (*W!=0.) energy += mytvgrad(cur,dE,*eps,*p,*W);
      energy /= (double)(nx*ny);
      if (c) {
	printf("Stop after %d iterations (energy increases)\n",i);
	cont = 0;
      } else {
	step *= STEP_FACTOR;
	prevok = 0;
	i--;
	cont = (step>*s*STEP_LIMIT);
	if (v) {
	  if (cont) printf("reduction to step=%f\n",step);
	  else printf("Step reduction limit reached, stop.\n");
	}
      }
    } else {
      if (prevok && (v || !n)) 
	norm2 = fnorm(cur,prev,&two,0,0,&border,0,0);
      if (n) cont=(i!=*n); else cont=(!prevok || norm2>=*e);
      if (v) {
	printf("n=%3d: E = %f, dt = %f",i,energy,step);
	if (prevok) printf(", |du| = %f\n",norm2); else printf("\n");
      }
      prevok = 1;
    }

  } while (cont);
  /***** END OF MAIN LOOP *****/

  if (cur!=out) mw_copy_fimage(cur,out);
  mw_delete_fimage(aux2);
  mw_delete_fimage(aux);

  return(out);
}










