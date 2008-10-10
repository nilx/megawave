/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {tvdenoise};
version = {"2.1"};
author = {"Lionel Moisan"};
function = {"Image denoising by TV minimization (Rudin-Osher)"};
usage = {
 'W':[W=10.]->W    "weight on regularization term",
 's':[s=1.]->s     "initial (and maximal) time step",
 'E':[eps=1.]->eps "epsilon in sqrt(epsilon+|Du|^2)",
 'p':[p=0.5]->p    "positive coefficient for the gradient scheme",
 'e':[e=0.02]->e   "stop when |u(n)-u(n-1)|<e (L2 error)",
 'n':n->n          "or perform a fixed number n of iterations",
 'r':ref->ref      "to specify a reference image different from in",
 'v'->v            "verbose : print energy and L2 errors at each iteration",
 'c'->c            "cancel auto step reduction",
 in->in            "input Fimage",
 out<-out          "output Fimage"
        };
*/
/*----------------------------------------------------------------------
 v1.1: fixed consistency bug in mytvgrad() and step reduction loop (LM)
 v1.2: added -p option to allow more general gradient schemes (LM)
 v2.0: changed -w option to -W, ie weight on REGULARIZATION term (LM)
 v2.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for fnorm() */

#define STEP_FACTOR 0.8
#define STEP_LIMIT 1e-20


/* compute weight * total variation and add its gradient to grad */
double mytvgrad(u,grad,eps,p,weight)
     Fimage u,grad;
     double eps,p,weight;
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

/* Compute energy F(u) = int |u-u_0|^2 and its gradient */
double fidelity_term_grad(u,u0,grad)
     Fimage u,u0,grad;
{
  int adr;
  double e,d;

  for (e=0.,adr=u0->ncol*u0->nrow;adr--;) {
    d = (double)u->gray[adr] - (double)u0->gray[adr];
    grad->gray[adr] = 2.*(float)d;
    e += d*d;
  }

  return(e);
}


/*----------------------- MAIN MODULE ---------------*/

Fimage tvdenoise(in,out,s,c,v,e,n,W,ref,eps,p)
     Fimage in,out,ref;
     int *n;
     double *s,*e,*W,*eps,*p;
     char *v,*c;
{
  Fimage dE,aux,tmp,cur,prev;
  double energy,old_energy,step;
  float two,norm1,norm2;
  int i,j,border,cont,prevok,nx,ny;

  /* initialization */
  nx = in->ncol; 
  ny = in->nrow;
  border = 0;
  two = 2.;

  dE = mw_change_fimage(NULL,ny,nx);
  out = mw_change_fimage(out,ny,nx);
  aux = mw_change_fimage(NULL,ny,nx);
  if (!dE || !out || !aux) 
    mwerror(FATAL,1,"inttv: not enough memory\n");

  mw_copy_fimage(in,out);
  mw_copy_fimage(in,aux);
  if (!ref) ref = in;
  prev = aux; cur = out; 
  i = prevok = 0; step = *s;

  /* compute initial error and its gradient */
  energy = fidelity_term_grad(cur,ref,dE);
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
    energy = fidelity_term_grad(cur,ref,dE);
    if (*W!=0.) energy += mytvgrad(cur,dE,*eps,*p,*W);
    energy /= (double)(nx*ny);
    
    if (energy>old_energy) {
      tmp=prev; prev=cur; cur=tmp;
      energy = fidelity_term_grad(cur,ref,dE);
      if (*W!=0.) energy += mytvgrad(cur,dE,*eps,*p,*W);
      energy /= (double)(nx*ny);
      if (c) {
	printf("Stop after %d iterations (energy increases).\n",i);
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
	norm1 = fnorm(cur,ref,&two,0,0,&border,0,0);
	printf("n=%3d: E = %f, dt = %f, |u-u0| = %f",i,energy,step,norm1);
	if (prevok) printf(", |du| = %f\n",norm2); else printf("\n");
      }
      prevok = 1;
    }

  } while (cont);
  /***** END OF MAIN LOOP *****/

  if (cur!=out) mw_copy_fimage(cur,out);
  mw_delete_fimage(aux);

  return(out);
}










