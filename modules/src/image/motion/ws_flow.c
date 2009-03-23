/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand 
 name = {ws_flow}; 
 version = {"1.1"};
 author = {"Florent Ranchin"}; 
 function = {"Weickert and Schnoerr optical flow computation "};
 usage = {
  'p':percent->percent     "if set, stop when |E(n)-E(n-1)|<E(1)*percent/100",
  'n':[n=100]->n           "maximum number of iterations",
  't':[tau=0.166666]->tau  "time-step",
  'l':[lambda=1.]->lambda  "contrast parameter",
  'E':[eps=0.000001]->eps  "epsilon (may be arbitrary small)",
  'A':[alpha=500.]->alpha  "weight of divergence term in pde",
  'R':norm_movie<-norm     "optional output: optical flow norm",
  movie->movie             "Input Fmovie",
  wsU<-wsU                 "Fmovie of OF_{1}(x,y)",
  wsV<-wsV                 "Fmovie of OF_{2}(x,y)"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "mw.h"
#include "mw-modules.h" /* for fop() */

               /* GLOBAL VARIABLES */   
                

/****global variables: not very beautiful, but avoid functions with 50 arguments ;-)******/
static int nx,ny,nz,nzo;
static long adrxy;
static float *a,*Ex,*Ey,*Et;
static float *U,*V,*residue;

static void ALLOCATE(Fmovie out, int nb_image)
{
  Fimage image,image_prev;
  int l;
  
  out = mw_change_fmovie(out);
  if(out==NULL) mwerror(FATAL,1,"Not enough memory\n");
  
  for(l=0;l<nb_image;l++) 
    { if((image = mw_change_fimage(NULL,ny,nx))==NULL)
      {mw_delete_fmovie(out);
      mwerror(FATAL,1,"Not enough memory\n");
      }
    if(l==0)
      out->first=image;
    else
      {image_prev->next=image;
      image->previous =image_prev;
      }  
    image_prev=image;
    }
}

/***derivative of function \psi(s^2)=\epsilon s^2+(1-\epsilon)\lambda^2\sqrt{1+\frac{s^2}{\lamda^2}}
\psi^{\prime}(s^2)=\epsilon+\frac{1-\epsilon}{\sqrt{1+\frac{s^2}{\lamda^2}}}********/
static float psip(float x, float e, float l)
{
  float value;
  value=e+(1-e)/(float)sqrt((double)(1+x*x/(l*l)));
  return value;
}


static void dummies(float *v)
              
     /* creates dummy boundaries by mirroring (Neumann conditions)*/
{
  int i, j,l; /* loop variables */
  
  for(l=0;l<nzo;l++)
    for (i=1; i<=nx-2; i++)
      {
	v[i+nx*ny*l]    = v[i+nx+nx*ny*l];
	v[i+nx*(ny-1)+nx*ny*l] = v[i+nx*(ny-2)+nx*ny*l];
      }
  for(l=0;l<nzo;l++)
    for (j=0; j<=ny-1; j++)
      {
	v[nx*j+nx*ny*l]    = v[1+nx*j+nx*ny*l];
	v[nx-1+nx*j+nx*ny*l] = v[nx-2+nx*j+nx*ny*l];
      }
  
  return;
}

/***derivees compute spatial and temporal derivatives of image*********/
static void derivees(void)
{   
  int i,j,l;
  register float a_or,a_N,a_S,a_E,a_O,a_NE,a_NO,a_SE,a_SO,at_post;
  long adr;
  
  for(l=0;l<nzo;l++)
    {
      for(j=0;j<ny;j++)
        for(i=0;i<nx;i++)
          {
	    adr = i+nx*(j+ny*l);
	    if(j<ny-1)
	      a_N = a[adr+nx];
	    if(j>0)
	      a_S=a[adr-nx];
	    if(i<nx-1)
	      a_E = a[adr+1];
	    if(i>0)
	      a_O=a[adr-1];
	    a_or = a[adr];
	    if((i<nx-1)&&(j<ny-1))
	      a_NE = a[adr+1+nx];
	    if((i>0)&&(j<ny-1))
	      a_NO = a[adr-1+nx];
	    if((i>0)&&(j>0))
	      a_SO = a[adr-1-nx];
	    if((i<nx-1)&&(j>0))
	      a_SE = a[adr+1-nx];
	    at_post = a[adr+ny*nx];
	    
	    
	    if((j==ny-1)||(i==nx-1)||(i==0)||(j==0))
	      {
		Ex[adr] = 0.0;  
		Ey[adr] = 0.0;
		Et[adr] = 0.0;
	      }
	    else
	      {/************schemes for spatial derivatives rotationnally invariant*********/
		Ex[adr] = -0.1464466095*a_NO+0.1464466095*a_NE-0.2071067810*a_O+0.2071067810*a_E-0.1464466095*a_SO+0.1464466095*a_SE;
		Ey[adr] = -0.1464466095*a_SE+0.1464466095*a_NE-0.2071067810*a_S+0.2071067810*a_N-0.1464466095*a_SO+0.1464466095*a_NO;
		Et[adr] = at_post-a_or;
	      }
	    
	    
	  }
    }
}     

static void schema_ws(float *threshold, float eps, float tau, float alpha, float lambda, int niter)
{
  int i,j,l,p;
  long adr;
  register float grad_2u,grad_2v;
  float *diffus,*tampU,*tampV;
  register float wN,wS,wE,wO,wNz,wSz;
  register float maxresidue,maxresidue0;
  /****one pixel has six neighbours in 3D (North, South, Est, West [Ouest in french], before [South in z] and after [North in z]) and four in 2D (North, South, Est, West)********/
  long taille;
  
  taille=(long)(nx*ny*nzo);
  
  diffus=(float *)malloc(taille*sizeof(float));
  tampU=(float *)malloc(taille*sizeof(float));
  tampV=(float *)malloc(taille*sizeof(float));
  
  if((diffus==NULL)||(tampU==NULL)||(tampV==NULL))
    mwerror(FATAL,1,"Allocation error in ws_flow");
  for(l=0;l<nzo;l++)
    for (j=0;j<ny;j++) 
      for (i=0;i<nx;i++)
	{
	  adr=i+nx*(j+ny*l);
	  U[adr]=0.0;
	  V[adr]=0.0;
	}
  p=0;
  while(p<niter )
    {
      if(!(p%50))
	printf("iteration %d/%d\n",p+1,niter);
      /*********calculus of the diffusion tensor****************/
      for(l=0;l<nzo;l++)
	for (j=1;j<ny-1;j++) 
	  for (i=1;i<nx-1;i++)
	    {
	      adr=i+nx*(j+ny*l);
	      
	      if((l>0)&&(l<(nzo-1)))
		grad_2u=(float)pow((double)((U[adr+1]-U[adr-1])/2),2.0)+(float)pow((double)((U[adr+nx]-U[adr-nx])/2),2.0)+(float)pow((double)((U[adr+nx*ny]-U[adr-nx*ny])/2),2.0);
	      if(l==0)
		grad_2u=(float)pow((double)((U[adr+1]-U[adr-1])/2),2.0)+(float)pow((double)((U[adr+nx]-U[adr-nx])/2),2.0)+(float)pow((double)(U[adr+nx*ny]-U[adr]),2.0);
	      if(l==(nzo-1))
		grad_2u=(float)pow((double)((U[adr+1]-U[adr-1])/2),2.0)+(float)pow((double)((U[adr+nx]-U[adr-nx])/2),2.0)+(float)pow((double)(U[adr]-U[adr-nx*ny]),2.0);
	      
	      if((l>0)&&(l<(nzo-1)))
		grad_2v=(float)pow((double)((V[adr+1]-V[adr-1])/2),2.0)+(float)pow((double)((V[adr+nx]-V[adr-nx])/2),2.0)+(float)pow((double)((V[adr+nx*ny]-V[adr-nx*ny])/2),2.0);
	      if(l==0)
		grad_2v=(float)pow((double)((V[adr+1]-V[adr-1])/2),2.0)+(float)pow((double)((V[adr+nx]-V[adr-nx])/2),2.0)+(float)pow((double)(V[adr+nx*ny]-V[adr]),2.0);
	      if(l==(nzo-1))
		grad_2v=(float)pow((double)((V[adr+1]-V[adr-1])/2),2.0)+(float)pow((double)((V[adr+nx]-V[adr-nx])/2),2.0)+(float)pow((double)(V[adr]-V[adr-nx*ny]),2.0);
	      
	      diffus[adr]=psip((float)sqrt((double)(grad_2u+grad_2v)),eps,lambda);
	      tampU[adr]=U[adr];
	      tampV[adr]=V[adr];
	    }
      dummies(diffus);
      dummies(tampU);
      dummies(tampV);
      /**********algorithm**************/
      
      for(l=0;l<nzo;l++){
	for (j=1;j<ny-1;j++) 
	  for (i=1;i<nx-1;i++)
	    {
	      adr=i+nx*(j+ny*l);
	      if(j==ny-2)
		wN=0.0;
	      else
		wN=(diffus[adr+nx]+diffus[adr])/2;
	      if(j==1)
		wS=0.0;
	      else
		wS=(diffus[adr-nx]+diffus[adr])/2;
	      if(i==nx-2)
		wE=0.0;
	      else
		wE=(diffus[adr+1]+diffus[adr])/2;
	      if(i==1)
		wO=0.0;
	      else
		wO=(diffus[adr-1]+diffus[adr])/2;
	      
	      if(l<(nzo-1))
		wNz=(diffus[adr+nx*ny]+diffus[adr])/2; 
	      else
		wNz=0.0;
	      if(l>0)
		wSz=(diffus[adr-nx*ny]+diffus[adr])/2;
	      else
		wSz=0.0;
	      
	      /**************** semi-implicit scheme (diffusion term discretized at order n)***********/
	      /**************** (u^{n+1}-u^{n})/delta=tau*sum_{x\in 6-neighbourhood} W(u^{n})(x)u^{n}(x)
				                      -tau/alpha*E_{x}(E_{x}u^{n+1}+E_{y}v^{n}+E_{t}) ***********/
	      if((l>0)&&(l<nzo-1))
		{  
		  
		  U[adr]=(tampU[adr]+tau*(wN*tampU[adr+nx]+wS*tampU[adr-nx]+wE*tampU[adr+1]+wO*tampU[adr-1]+wNz*tampU[adr+nx*ny]+wSz*tampU[adr-nx*ny]-(wN+wS+wE+wO+wNz+wSz)*tampU[adr])-tau/alpha*Ex[adr]*(Ey[adr]*tampV[adr]+Et[adr]))/(1+tau/alpha*Ex[adr]*Ex[adr]);
		  V[adr]=(tampV[adr]+tau*(wN*tampV[adr+nx]+wS*tampV[adr-nx]+wE*tampV[adr+1]+wO*tampV[adr-1]+wNz*tampV[adr+nx*ny]+wSz*tampV[adr-nx*ny]-(wN+wS+wE+wO+wNz+wSz)*tampV[adr])-tau/alpha*Ey[adr]*(Ex[adr]*tampU[adr]+Et[adr]))/(1+tau/alpha*Ey[adr]*Ey[adr]);
		  
		}	 
	      if(l==0)
		{	  	    
		  U[adr]=(tampU[adr]+tau*(wN*tampU[adr+nx]+wS*tampU[adr-nx]+wE*tampU[adr+1]+wO*tampU[adr-1]+wNz*tampU[adr+nx*ny]-(wN+wS+wE+wO+wNz)*tampU[adr])-tau/alpha*Ex[adr]*(Ey[adr]*tampV[adr]+Et[adr]))/(1+tau/alpha*Ex[adr]*Ex[adr]);
		  V[adr]=(tampV[adr]+tau*(wN*tampV[adr+nx]+wS*tampV[adr-nx]+wE*tampV[adr+1]+wO*tampV[adr-1]+wNz*tampV[adr+nx*ny]-(wN+wS+wE+wO+wNz)*tampV[adr])-tau/alpha*Ey[adr]*(Ex[adr]*tampU[adr]+Et[adr]))/(1+tau/alpha*Ey[adr]*Ey[adr]);
		  
		}  
	      if(l==(nzo-1)) 
		{	     
		  U[adr]=(tampU[adr]+tau*(wN*tampU[adr+nx]+wS*tampU[adr-nx]+wE*tampU[adr+1]+wO*tampU[adr-1]+wSz*tampU[adr-nx*ny]-(wN+wS+wE+wO+wSz)*tampU[adr])-tau/alpha*Ex[adr]*(Ey[adr]*tampV[adr]+Et[adr]))/(1+tau/alpha*Ex[adr]*Ex[adr]);
		  V[adr]=(tampV[adr]+tau*(wN*tampV[adr+nx]+wS*tampV[adr-nx]+wE*tampV[adr+1]+wO*tampV[adr-1]+wSz*tampV[adr-nx*ny]-(wN+wS+wE+wO+wSz)*tampV[adr])-tau/alpha*Ey[adr]*(Ex[adr]*tampU[adr]+Et[adr]))/(1+tau/alpha*Ey[adr]*Ey[adr]);
		  
		}
	      /*if(p)*/
	      residue[l]+=((U[adr]-tampU[adr])*(U[adr]-tampU[adr])+(V[adr]-tampV[adr])*(V[adr]-tampV[adr]));
	    }
	/*if(p){*/
	residue[l]=(float)sqrt((double)residue[l])/(nx*ny);
	if(l==0)
	  maxresidue=residue[l];
	else
	  maxresidue = (maxresidue > residue[l] ) ? maxresidue : residue[l];
	/*}*/
	if(p==0)
	  maxresidue0=maxresidue;
	dummies(U);
	dummies(V);
      }
      
      if(threshold!=NULL && maxresidue<((*threshold)*maxresidue0)) 
	break;
      
      p++;
    }
  printf("%d iterations\t residue (||OF^{k+1}-OF{k}|| l^2 norm):%f\n",p,maxresidue);
  
  free(tampU);
  free(tampV);
  free(diffus);
  
}		  


void ws_flow(float *percent, int *n, float *tau, float *lambda, float *eps, float *alpha, Fmovie norm, Fmovie movie, Fmovie wsU, Fmovie wsV)
{
  int l;
  long adr;
  char normflag=1;
  Fimage u,uU,uV,ud,nd=NULL;
  
  
  nz=1;
  u = movie->first;
  while((u->next)!=NULL)
    {
      nz++;
      u=u->next;
    }
  nx=u->ncol;
  ny=u->nrow;
  adrxy=(long)(nx*ny);
  nzo=nz-1;
  mwdebug("Size of images, size of movie\n nx=%d\t ny=%d\t nz=%d\t\n",nx,ny,nz);
  Ex=(float *)malloc(adrxy*nzo*sizeof(float));
  Ey=(float *)malloc(adrxy*nzo*sizeof(float));
  Et=(float *)malloc(adrxy*nzo*sizeof(float));
  a=(float *)malloc(adrxy*nz*sizeof(float));
  
  U=(float *)malloc(adrxy*nzo*sizeof(float));
  V=(float *)malloc(adrxy*nzo*sizeof(float));
  residue=(float *)malloc(nzo*sizeof(float));
  
  ALLOCATE(wsU,nzo);
  ALLOCATE(wsV,nzo);
  if(norm)
    ALLOCATE(norm,nzo);
  ud=movie->first;
  for(l=0;l<nz;l++,ud=ud->next)
    for(adr=0;adr<adrxy;adr++)
      a[adr+nx*ny*l]=(float) ud->gray[adr];
  
  derivees();
  
  schema_ws(percent,*eps,*tau,*alpha,*lambda,*n);
  
  uU = wsU->first;
  uV = wsV->first;
  if(norm)
    nd=norm->first;
  for(l=0;l<nzo;l++)
    {
      for(adr=0;adr<adrxy;adr++)
	{
	  uU->gray[adr] = U[adr+nx*ny*l];
	  uV->gray[adr] = V[adr+nx*ny*l];
	}
      if(norm){
	fop(uV,nd,uU,NULL,NULL,NULL,NULL,NULL,NULL,&normflag,NULL,NULL,NULL,NULL,NULL);
	nd=nd->next;
      }      
      uU=uU->next;
      uV=uV->next;
    }
  
  free(Ex);
  free(Ey);
  free(Et);
  free(U);
  free(V);
  free(residue);
  free(a);

}
  
  

     
