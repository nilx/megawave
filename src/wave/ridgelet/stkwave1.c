/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
  name = {stkwave1};
  version = {"1.0"};
  author = {"Claire Jonchery, Amandine Robin"};
  function ={"One-dimensional wavelet transform using Starck's algorithm (band-limited scaling function)"};
  usage = {
  np->np "resolution np",
  in->in "input in Fourier domain (the size of signal must be a power of 2)",
  out<-out "result in Fourier domain, from left to right : details and approximation"
};
*/
/*----------------------------------------------------------------------
 v1.0: initial revision before publication (J.Froment)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "mw.h"

#define EPS 1e-15

extern void   fft1d();

/* compute the scaling function */

void phi(x,out)
double x,*out;
{
  if (fabs(x)>EPS) *out=3./8.*pow(((sin(M_PI*x/4.))/( M_PI*x/4.)),4.);
  else *out=3./8.; 
}


/* compute the fft of phi (scaling function)*/

void phichapo(N,p,outr,outi) 
     int N;
     int p;
     Fsignal outr,outi;
     /* p est l'inverse du pas d'échantillonnage de phi, p divise N,
	outr est la partie réelle de la TF de phi, outi sa partie imaginaire*/
{
 Fsignal phid;
 int i;
 double val,l; 
 
 outr=mw_change_fsignal(outr,N);
 if (!outr) mwerror(FATAL,1,"not enough memory !\n");

 outi=mw_change_fsignal(outi,N);
 if (!outi) mwerror(FATAL,1,"not enough memory !\n");

 phid=mw_change_fsignal(NULL,N); 
 if (!phid) mwerror(FATAL,1,"not enough memory! \n");

 /* on translate les valeurs de phi pour x négatif à la suite des valeurs de phi pour x positif */
 /* phid est le signal shifté*/
 for (i=0; i<(N/2);i++)
  { 
    l=i*1./p;
    phi(l,&val);
    phid->values[i] =val;
    }
 for(i=-(N/2);i<0;i++)
 { 
   l=i*1./p;
     phi(l,&val) ;
     phid->values[i+N]=val;
    }

 fft1d(phid,NULL,outr,outi);
 mw_delete_fsignal(phid);
}


/* compute the low-pass filter in Fourier domain */

void hchapo(N,out)
     int N; /*nombre de valeurs échantillonnées, N est une puissance de 2*/
     Fsignal out;/*TF du filtre h*/
{
  Fsignal phichap1,phichap2,im1,im2;
  int i;

  out=mw_change_fsignal(out,N);
  if (!out) mwerror(FATAL,1,"not enough memory !\n");

  phichap1=mw_change_fsignal(NULL,N);
  if (!phichap1) mwerror(FATAL,1,"not enough memory !\n");

  phichap2=mw_change_fsignal(NULL,(2*N));
  if (!phichap2) mwerror(FATAL,1,"not enough memory !\n");

  im1=mw_change_fsignal(NULL,N);
  if (!im1) mwerror(FATAL,1,"not enough memory !\n");

   im2=mw_change_fsignal(NULL,(2*N));
   if (!im2) mwerror(FATAL,1,"not enough memory !\n");

   phichapo(N,1,phichap1,im1);
   phichapo((2*N),2,phichap2,im2);
    
    /* on a négligé la partie imaginaire de phichapo*/

   for (i=0; i<(N/4) ;i++)
    out->values[i]=(phichap2->values[2*i])/(phichap1->values[i]);
    

   for (i=(N/4); i<(3*N/4);i++)
     out->values[i]=0;

   for (i=(3*N/4); i<N ;i++)
     out->values[i]=(phichap2->values[2*i])/(phichap1->values[i]);

   for (i=0; i<N;i++)
      out->values[i]=fabs(out->values[i]);

   mw_delete_fsignal(phichap1);
   mw_delete_fsignal(phichap2);
   mw_delete_fsignal(im1);
   mw_delete_fsignal(im2);
}


/* compute the high-pass filter gchapo=1-hchapo*/
void gchapo(N,out)
     int N;
     Fsignal out;
{
  Fsignal h;
  int i;
 
  out=mw_change_fsignal(out,N);
  if (!out) mwerror(FATAL,1,"not enough memory !\n");

  h=mw_change_fsignal(NULL,N);
  if (!h) mwerror(FATAL,1,"not enough memory !\n");

  hchapo(N,h);
  for (i=0; i<N;i++)
    out->values[i]=1-(h->values[i]);

  mw_delete_fsignal(h);
}

/* compute the details of the signal*/

void detail(in,out)
     Fsignal in,out;
{
 Fsignal g;
 int i,N;

 N=(in->size);
 out=mw_change_fsignal(out,N);
 if (!out) mwerror(FATAL,1,"not enough memory !\n");

 g=mw_change_fsignal(NULL,N);
 if (!g) mwerror(FATAL,1,"not enough memory !\n");

 gchapo(N,g);

 for (i=0;i<N;i++)
   out->values[i]=(in->values[i])*(g->values[i]);
 mw_delete_fsignal(g);
}


/* compute the approximation of the signal*/
void approx(in,out)
     Fsignal in,out;
{
  Fsignal h;
  int i,N;

  N=(in->size);

  out=mw_change_fsignal(out,N);
  if (!out) mwerror(FATAL,1,"not enough memory !\n");

  h=mw_change_fsignal(NULL,N);
  if (!h) mwerror(FATAL,1,"not enough memory !\n");

  hchapo(N,h);

  for (i=0; i<N;i++)
    out->values[i]=(in->values[i])*(h->values[i]);

  mw_delete_fsignal(h);
}

/* decomposition in wavelets of a signal (using the algorithm of Starck) */

void stkwave1(np,in,out)
     Fsignal in,out;
     int np;
     /*si np=1,on obtient un vecteur de taille 2N, avec dans la première partie les détails, dans la seconde, l'approximation, l'un et l'autre non décimés*/

{
  Fsignal d,a,temp=NULL;
  int i,j,k,N,taille;

  N=(in->size);
  out=mw_change_fsignal(out,(2*N));
  if (!out) mwerror(FATAL,1,"not enough memory !\n");

  d=mw_change_fsignal(NULL,N);
  if (!d) mwerror(FATAL,1,"not enough memory !\n");

  a=mw_change_fsignal(NULL,N);
  if (!a) mwerror(FATAL,1,"not enough memory !\n");
  

  detail(in,d);
  for (i=0;i<N;i++)
    out->values[i]=(d->values[i]);

  approx(in,a);

  for (j=1;j<np;j++)
    {
      taille= (int) (N/((int) pow(2.0,(double)j)));
      temp=mw_change_fsignal(temp,taille);
     /*  printf("taille= %d \n",temp->size); */
      if (temp == NULL) mwerror(FATAL,1,"not enough memory !\n");
            
      for (i=0;i<(N/pow(2.0,(double)(j+1)));i++)
	temp->values[i]=(a->values[i]);

      for (i=N/(pow(2.0,(double) (j+1)));i<N/(pow(2.0,(double)j));i++)
	{
	  k=i+N/(pow(2.0,(double)j));
          temp->values[i]=(a->values[k]);
	}
      
      detail(temp,d);
      approx(temp,a);
   
      for (i=2*N-(N/pow(2.0,(double)(j-1)));i<(2*N-(N/pow(2.0,(double)j)));i++)
	{
	  k=i-2*N+(N/pow(2.0,(double)(j-1)));
	  out->values[i]=(d->values[k]);
	}
    }
  for (i=2*N-(N/pow(2.0,(double)(np-1)));i<=(2*N-1);i++)
    {
      k=i-2*N+(N/pow(2.0,(double)(np-1)));
      out->values[i]=(a->values[k]);
    }
			   
  mw_delete_fsignal(d);
  mw_delete_fsignal(a);
  if (np>1) mw_delete_fsignal(temp);
}  


