/*----------------------------MegaWave2 module----------------------------*/
/* mwcommand
name = {tvdenoise};
version = {"1.0"};
author = {"Lionel Moisan"};
function = {"Image denoising by TV minimization (Rudin-Osher)"};
usage = {
 'w':[w=0.1]->w    "weight on fidelity term (default 0.1)",
 's':[s=1.]->s     "initial (and maximal) time step, default 1.",
 'E':[eps=1.]->eps "epsilon in sqrt(epsilon+|Du|^2), default 1.",
 'e':[e=0.1]->e    "stop when |u(n)-u(n-1)|<e (L2 error, default 0.1)",
 'n':n->n          "or perform a fixed number of iterations (default: 5)",
 'r':ref->ref      "to specify a reference image different from in",
 'v'->v            "verbose : print energy and L2 errors at each iteration",
 'c'->c            "cancel auto step reduction",
 in->in            "input Fimage",
 out<-out          "output Fimage"
        };
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mw.h"

extern float fnorm();

#define COEFF .70710678118654752440
#define COEFF2P2 3.41421356237309504880

double mytvgrad(u,grad,eps)
Fimage u,grad;
double eps;
{
  int x,y,nx,ny,adr;
  double E,a,b,c,d,e,f,z,norm;

  nx = u->ncol; 
  ny = u->nrow;
  E = 0.;
  if (grad) {
    mw_change_fimage(grad,ny,nx);
    mw_clear_fimage(grad,0.);
  }

  for (x=0;x<nx-1;x++)
    for(y=0;y<ny-1;y++) {
      adr = y*nx+x;
      a = (double)u->gray[adr     ];
      b = (double)u->gray[adr   +1];
      c = (double)u->gray[adr+nx+1];
      d = (double)u->gray[adr+nx  ];
      z=d; d-=c; c-=b; b-=a; a-=z; e=COEFF*(b-d); f=COEFF*(c-a);
      norm = sqrt( eps + (a*a+b*b+c*c+d*d + e*e+f*f)/COEFF2P2 );
      E += norm;
      norm *= COEFF2P2;
      if (grad) {
	if (norm==0.) norm=1.;
	grad->gray[adr     ] += (a-b+COEFF*(-e-f))/norm;
	grad->gray[adr   +1] += (b-c+COEFF*( e-f))/norm;
	grad->gray[adr+nx+1] += (c-d+COEFF*( e+f))/norm;
	grad->gray[adr+nx  ] += (d-a+COEFF*(-e+f))/norm;
      }
    }

  return(E);
}


/* Compute energy F(u) = weight * int |u-u_0|^2 and its gradient */
double fidelity_term_grad(u,u0,grad,weight)
     Fimage u,u0,grad;
     double weight;
{
  int x,y,nx,ny,adr;
  double e,d;

  nx = u->ncol; 
  ny = u->nrow;
  e = 0.;

  for (x=0;x<nx;x++)
    for(y=0;y<ny;y++) {
      adr = y*nx+x;
      d = (double)u->gray[adr] - (double)u0->gray[adr];
      if (grad) grad->gray[adr] += (float)(weight*2.*d);
      e += weight*d*d;
    }

  return(e);
}


/*----------------------- MAIN MODULE ---------------*/

Fimage tvdenoise(in,out,s,c,v,e,n,w,ref,eps)
Fimage in,out,ref;
int *n;
double *s,*e,*w,*eps;
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
  energy = mytvgrad(cur,dE,*eps);
  if (*w!=0.) energy += fidelity_term_grad(cur,ref,dE,*w);
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
    energy = mytvgrad(cur,dE,*eps);
    if (*w!=0.) energy += fidelity_term_grad(cur,ref,dE,*w);
    energy /= (double)(nx*ny);
    
    if (energy>old_energy) {
      tmp=prev; prev=cur; cur=tmp;
      energy = mytvgrad(cur,dE,*eps);
      if (*w!=0.) energy += fidelity_term_grad(cur,ref,dE,*w);
      energy /= (double)(nx*ny);
      if (c) {
	printf("Stop after %d iterations (energy increases)\n",i);
	cont = 0;
      } else {
	step *= 0.8;
	if (v) printf("reduction to step=%f\n",step);
	prevok = 0;
	i--;
	cont = 1;
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










