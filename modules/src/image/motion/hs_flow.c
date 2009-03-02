/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {hs_flow};
 version = {"1.2"};
 author = {"Olivia Sanchez"};
 function = {"Horn and Schunck iterative scheme to compute optical flow"};
 usage = {
   'n':[niter=200]->niter   "number of iterations",
   'a':[alpha=10.]->alpha   "weight on smoothing term",  
   in->in                   "input Fmovie",
   xflow<-xflow             "output Fmovie: x coordinate of optical flow",
   yflow<-yflow             "output Fmovie: y coordinate of optical flow"
};
*/

/*----------------------------------------------------------------------
 v1.1: removed unused angle() function (JF)
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "mw.h"
#include "mw-modules.h"

#define CARRE(X)((X)*(X))
#define ABS(X)((X)<0?-(X):(X))


               /* GLOBAL VARIABLES */   

float *a;               /* original movie  */
float *Ex,*Ey,*Et;      /* gray level derivatives*/
float **U,**V;

int   nx,ny,nz,nzo;     /* movie input dimensions */ 
long  adrxyz,adrxyz2,adrxy;

 

static void ALLOCATE(Fmovie out, int nb_image)
{
  Fimage image,image_prev;
  int l;
  
  out = mw_change_fmovie(out);
  if(out==NULL) mwerror(FATAL,1,"Not enough memory\n");
  
  for(l=0;l<nb_image;l++) {
    if ((image = mw_change_fimage(NULL,ny,nx))==NULL) {
      mw_delete_fmovie(out);
      mwerror(FATAL,1,"Not enough memory\n");
    }
    if(l==0)
      out->first=image;
    else {
      image_prev->next=image;
      image->previous =image_prev;
    }  
    image_prev=image;
  }
}



/* computes gray level derivatives E(x,y,t) 
   (following the scheme proposed by Horn and Schunck)*/

static void CALC_EDERIV(void)
{   
  int i,j,l;
  register float a010,a000,a110,a100,a011,a001,a111,a101;
  long adr;
  
  for(l=0;l<nz-1;l++) {

    for(j=0;j<ny;j++)
      for(i=0;i<nx;i++) {

	adr = i+nx*(j+ny*l);
	
	a010 = a[adr+nx];
	a000 = a[adr];
	a110 = a[adr+1+nx];
	a100 = a[adr+1];
	a011 = a[adr+nx+ny*nx];
	a001 = a[adr +ny*nx];
	a111 = a[adr+1+nx+ny*nx];
	a101 = a[adr+1+ny*nx];
	
	if ((j==ny-1) || (i==nx-1)) {
	  Ex[adr] = 0;  
	  Ey[adr] = 0;
	  Et[adr] = 0;
	} else {
	  Ex[adr] = 0.25*(a111-a011+a101-a001+a110-a010+a100-a000);
	  Ey[adr] = 0.25*(a111-a101+a011-a001+a110-a100+a010-a000);
	  Et[adr] = 0.25*(a111-a110+a011-a010+a101-a100+a001-a000);
	}
      }
  } 
}     


/* rentree du schema iteratif pour calculer le flot optique */

static void SCHEMA_ITER(int niter, float *alpha)
{
  int i,j,k,l;
  long adr; 
  float f,g,d,o;
  float **Ua,**Va;          /* coordonnees des vecteurs moyennes */ 
  register float um0,u01,u10,u0m,umm,um1, u11,u1m;
  register float vm0,v01,v10,v0m,vmm,vm1,v11,v1m;
  
  mwdebug("niter = %d\n",niter);
  mwdebug("nz = %d\n",nz);
  
  mwdebug("alpha = %f\n",*alpha);
  
  Ua = (float **)malloc(niter*sizeof(float*));
  Va = (float **)malloc(niter*sizeof(float*));
  
  for(k=0;k<2;k++) {
    Ua[k] = (float *)malloc(adrxyz2*sizeof(float));  
    Va[k] = (float *)malloc(adrxyz2*sizeof(float));
    
  }
  
  for(l=0;l<nz-1;l++)
    for (j=0;j<ny;j++) 
      for (i=0;i<nx;i++) {
	
	adr = i+nx*(j+ny*l);
	
	f=CARRE(Ex[adr]);
	g=CARRE(Ey[adr]);
	
	if((f+g)!=0) {
	  U[0][adr] = (-Et[adr]*Ex[adr])/(f+g); 
	  V[0][adr] = (-Et[adr]*Ey[adr])/(f+g);  
	} else {
	  U[0][adr] = 0;  
	  V[0][adr] = 0;
	}
	
      }
  
  /* determination des coordonnees de u et v */ 
  
  for(l=0;l<nz-1;l++) {

    printf("image %d/%d\n",l+1,nz-1);
    
    k=0;
    while(k<=niter) {
      
      for (j=0;j<ny;j++)  
	for (i=0;i<nx;i++) {
	  adr = i+nx*(j+ny*l);
	  if((j==0) || (j==ny-1)) {
	    Ua[0][adr] = 0;  
	    Va[0][adr] = 0;
	  } else {
	    um0 = U[0][adr-1];
	    u01 = U[0][adr+nx];
	    u10 = U[0][adr+1];
	    u0m = U[0][adr-nx];
	    umm = U[0][adr-1-nx];
	    um1 = U[0][adr-1+nx];
	    u11 = U[0][adr+1+nx];
	    u1m = U[0][adr+1-nx];
	    
	    vm0 = V[0][adr-1];
	    v01 = V[0][adr+nx];
	    v10 = V[0][adr+1];
	    v0m = V[0][adr-nx];
	    vmm = V[0][adr-1-nx];
	    vm1 = V[0][adr-1+nx];
	    v11 = V[0][adr+1+nx];
	    v1m = V[0][adr+1-nx];
	    
	    Ua[0][adr] = 0.16666*(um0+u01+u10+u0m)+0.08333*(umm+um1+u11+u1m);
	    Va[0][adr] = 0.16666*(vm0+v01+v10+v0m)+0.08333*(vmm+vm1+v11+v1m);
	  }  
	}  
      
      
      for (j=0;j<ny;j++) 
	for (i=0;i<nx;i++) {
	  adr = i+nx*(j+ny*l);
	  
	  f=CARRE(Ex[adr]);
	  g=CARRE(Ey[adr]);
	  
	  o = Ex[adr]*Ua[0][adr] + Ey[adr]*Va[0][adr] + Et[adr];  
	  d = 1/(CARRE(*alpha)+f+g);  
	  
	  U[1][adr] = Ua[0][adr] - (Ex[adr])*o*d;   
	  V[1][adr] = Va[0][adr] - (Ey[adr])*o*d;
	  
	}
      
      /*retour condition*/
      for (j=0;j<ny;j++) 
	for (i=0;i<nx;i++) {
	  adr = i+nx*(j+ny*l);
	  
	  U[0][adr] = U[1][adr]; 
	  V[0][adr] = V[1][adr]; 
	}
      k++;
      
    }
  }
  
  free(Ex);
  free(Ey);
  free(Et);
  free(Ua);
  free(Va);
}



/*------------------------------ MAIN MODULE ------------------------------*/

void hs_flow(int *niter, float *alpha, Fmovie in, Fmovie xflow, Fmovie yflow)
{
  Fimage u,ud,ux,uy;
  int  k,l;
  long  adr;
  
  if((in->first==NULL)||(in->first->next==NULL))
    mwerror(FATAL,1,"\nInput movie is too short\n");
  
  /* determine the number of frames in initial movie */
  u = in->first;
  nz = 1;
  while((u->next)!=NULL) {
    nz=nz+1;
    u = u->next;
  }
  
  nx=u->ncol; ny=u->nrow;
  nzo=nz-1;
  
  ALLOCATE(xflow,nzo);
  ALLOCATE(yflow,nzo);
  
  /* Allocate arrays */
  adrxy = (long)nx*(long)ny;
  adrxyz = adrxy*(long)nz;
  adrxyz2 = adrxy*(long)(nz-1);
  
  a  = (float *)malloc(adrxyz*sizeof(float));
  Ex = (float *)malloc(adrxyz2*sizeof(float));
  Ey = (float *)malloc(adrxyz2*sizeof(float));
  Et = (float *)malloc(adrxyz2*sizeof(float));

  U  = (float **)malloc(2*sizeof(float*));
  V  = (float **)malloc(2*sizeof(float*));

  for(k=0;k<2;k++) {
    U[k] = (float *)malloc(adrxyz2*sizeof(float));
    V[k] = (float *)malloc(adrxyz2*sizeof(float));
  }
  
  if ((!a)||(!Ex)||(!Ey)||(!Et))
    mwerror(FATAL,1,"not enough memory.\n");
  
  /* Copy input movie into a[] */
  ud = in->first; 
  for(l=0;l<nz;l++,ud=ud->next) {
    for (adr=0;adr<adrxy;adr++)
      a[adrxy*l+adr] = (float) ud->gray[adr];
  }
  
  /* MAIN LOOP*/
  
  CALC_EDERIV();
  
  SCHEMA_ITER(*niter,alpha);
  
  ux = xflow->first;
  uy = yflow->first;
  for (l=0;l<nzo;l++) {
    for (adr=0;adr<adrxy;adr++) {
      ux->gray[adr] = U[1][adr+nx*ny*l];
      uy->gray[adr] = V[1][adr+nx*ny*l];
    }
    ux = ux->next;
    uy = uy->next;
  }
  
  free(U);
  free(V);
}










