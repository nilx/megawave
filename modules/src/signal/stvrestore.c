/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
name = {stvrestore};
version = {"1.7"};
author = {"Jacques Froment"};
function = {"Restoration of signal using total variation and thresholding in orthonormal bases"};
usage = {
 'r'->relax                  "Relax constraint on approximation space V_J",
 'n':[n=10000]->Niter        "Number of iterations",
 'a':alpha->alpha [0., 1e-4] "Alpha parameter to smooth the total variation",
 'O':O->O    "Use orthonormal wavelet with the given impulse response",
 'I':I->I    "Use wavelet adapted to the interval with the given edge filter",
 'V':V<-V    "Output signal with minimal total variation",
 M->M        "Input thresholding mask (1|-1=coeff.kept, 0=coeff.thresholded)",
 u->u              "Input thresholded signal", 
 out<-stvrestore   "Output restored signal" 
	};
*/
/*----------------------------------------------------------------------
 v1.7 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for owave1(), iowave1(), smse() */

/* Compute the number of recursion (-j) used for the wavelet transform. 
   By the way, check the consistency of the mask M.
*/

int GetWaveletDepth(M)
     
     Fsignal M;
     
{
  int N,n0,n,p,r;
  float v;

  N=M->size;
  for (n=0;n<N;n++)
    {
      v = M->values[n];
      if ((v!=1.)&&(v!=-1.)&&(v!=0.)&&(v!=2.))
	mwerror(FATAL,1,"[GetWaveletDepth] Inconsistent mask M (consistent values are -1,0,1 and 2 only) !\n");	
    }

  for (n0=0;(n0<N)&&(M->values[n0]!=2.0);n0++);
  if (n0>=N) mwerror(FATAL,1,"[GetWaveletDepth] Invalid mask M : 2 not found !\n");
  for (n=n0;(n<N)&&(M->values[n0]==2.0);n++);  
  if (n!=N) mwerror(FATAL,1,"[GetWaveletDepth] Invalid mask M : does not end with 2 !\n");
  if ((N % (2*(n-n0))) != 0)
    mwerror(FATAL,1,"[GetWaveletDepth] Invalid mask M : length is not a power of 2 !\n");    

  p=N/(2*(n-n0));
  for (r=1;(p%2)==0;r=r+1,p/=2);
  mwdebug("Depth of the wavelet transform = %d\n",r);
  return(r);
}

/* Return the function to minimize J(u) and its gradient dJ(u)/du 
   J(u) = exact total variation of u.
*/

double J(u,dJ)
     
     Fsignal u,dJ;
     
{
  int n;
  double r,d,sd,ad;

  r=0.;
  mw_clear_fsignal(dJ,0.);
  for (n=0;n<u->size-1;n++) 
    {
      d = u->values[n+1]-u->values[n];
      if (d>0.) {sd=1.; ad=d;}
      else if (d<0) {sd=-1.; ad=-d;}
      else ad=sd=0.;
      r += ad;
      dJ->values[n] -= sd;
      dJ->values[n+1] += sd;
    }
  return(r);
}

/* Return the function to minimize J(u) and its gradient dJ(u)/du 
   J(u) = smoothed total variation of u.
*/

double J_alpha(u,dJ,alpha)

     Fsignal u,dJ;
     double alpha;
     
{
  int n;
  double r,a,b,c;

  r=0.;
  mw_clear_fsignal(dJ,0.);
  for (n=0;n<u->size-1;n++) 
    {
      a = u->values[n+1]-u->values[n];
      b = sqrt(a*a + alpha);
      c = a/b;
      r += b;;
      dJ->values[n] -= c;
      dJ->values[n+1] += c;
    }
  return(r);
}

/* Compute the projection on the constraint in the wavelet domain */

void Wproj(relax,WdJ,M)
     
char *relax;    /* If != NULL, relax constraint on approximation space V_J.
		   Warning : on versions < 1.4, this option was active (bug) !
		*/
Wtrans1d WdJ;
Fsignal M;

{
  register int i;
  register float *pW;
  register float *pM;
  int j,offset;

  pM=M->values; 
  /* Hard or soft thresholding */
  for (j=1; j<= WdJ->nlevel; j++) 
    {
      pW= WdJ->D[j][0]->values;
      for(i=0; i<WdJ->D[j][0]->size; i++, pW++, pM++) 
	if ((*pM==1.)||(*pM==-1.)) *pW=0.0;
    }
  if (!relax) /* Constraint on approximation space V_J with J = - nlevel */
    {
      pW=WdJ->A[WdJ->nlevel][0]->values;
      for(i=0; i<WdJ->A[WdJ->nlevel][0]->size; i++, pW++) 
	*pW=0.0;
    }
}

/* Compute the projection on the constraint P(dJ) */

void Proj(relax,dJ,M,O,I,r)

char *relax;    /* If != NULL, relax constraint on approximation space V_J.
		   Warning : on versions < 1.4, this option was active (bug) !
		*/
Fsignal dJ,M,O;
Fimage I;
int *r;

{
  int Edge;
  int Precond=0;
  int FilterNorm=2;
  Wtrans1d WdJ=NULL;
  Fsignal dJ0=NULL;
  int dbg;
  double	SNR;		/* Signal to noise ratio / `Sig1` */
  double	PSNR;		/* Peak signal to noise ratio / `Sig1` */
  double	MSE;		/* Mean square error between Sig1 and Sig2 */
  double	MRD;

  int x0,y0,sx,sy;
  
  if (I) Edge=3; else Edge=1;

  if (mwdbg!=0)
    {
      dJ0 = mw_change_fsignal(dJ0,dJ->size);
      if (!dJ0) mwerror(FATAL,1,"Not enough memory !\n");  
      mw_copy_fsignal(dJ, dJ0); 
    }

  WdJ=mw_new_wtrans1d();
  if (!WdJ) mwerror(FATAL,1,"Not enough memory !\n");     

  owave1(r,NULL, &Edge, &Precond, NULL, &FilterNorm, dJ, WdJ, O, I);
  Wproj(relax,WdJ,M);
  iowave1(r,NULL, &Edge, &Precond, NULL, &FilterNorm, WdJ, dJ, O, I);

  if (mwdbg!=0)
    {
      /* Check that without Wproj the error is negligible */
      dbg=mwdbg;
      mwdbg=0;
      smse(dJ0,dJ,NULL,&SNR, &PSNR, &MSE, &MRD);
      mwdbg=dbg;
      mwdebug("\t dJ-P(dJ) : snr=%g psnr=%g mse=%g mrd=%g\n",SNR,PSNR,MSE,MRD);
    }

  mw_delete_wtrans1d(WdJ);
  if (mwdbg != 0) mw_delete_fsignal(dJ0);
}

/* Return the f(t) = J(u-tP(dJ(u))) (Not needed : to draw f(t) only)
   Exact total variation case.
*/

double f(t,u,dJ)
     
     double t;
     Fsignal u;
     Fsignal dJ;  
     
{
  int n;
  double r=0.0;

  for (n=0;n<u->size-1;n++)
    r += fabs((double) u->values[n+1]-u->values[n]
	      -t*(dJ->values[n+1]-dJ->values[n]));
  return(r);
}

/* Return the f(t) = J(u-tP(dJ(u))) (Not needed : to draw f(t) only)
   Smoothed total variation case.
*/

double f_alpha(t,u,dJ,alpha)
     
     double t;
     Fsignal u;
     Fsignal dJ;  
     double alpha;
     
{
  int n;
  double r,dn;

  r=0.;
  for (n=0;n<u->size-1;n++)
    {
      dn = ((double) u->values[n+1] - u->values[n]) 
	- t * ((double) dJ->values[n+1]-dJ->values[n]);
      r += sqrt (dn*dn + alpha);
    }
  return(r);
}

/* Output the values of f(t)
*/

void output_f(u,dJ,alpha)
     
     Fsignal u;
     Fsignal dJ;  
     double *alpha;
     
{
  double t;

  if (!alpha)
    for (t=0;t<=0.1;t+=1e-5) fprintf(stderr,"%f %g\n",t,f(t,u,dJ));
  else
    for (t=0;t<=0.1;t+=1e-5) fprintf(stderr,"%f %g\n",t,f_alpha(t,u,dJ,*alpha));    
}

/* Return the derivative of f(t) = J(u-tP(dJ(u))) 
   Exact total variation case.
*/

double df(t,u,dJ)

     double t;
     Fsignal u;
     Fsignal dJ;  
     
{
  int n;
  double r=0.0;
  double s,du,dw;

  for (n=0;n<u->size-1;n++)
    {
      dw=dJ->values[n+1]-dJ->values[n];
      du=u->values[n+1]-u->values[n];
      s = du - t * dw;
      if (s > 0) r-=dw;
      else if (s < 0) r+=dw;
    }
  return(r);
}

/* Return the derivative of f(t) = J(u-tP(dJ(u))) 
   Smoothed total variation case.
*/

double df_alpha(t,u,dJ,alpha)

     double t;
     Fsignal u;
     Fsignal dJ;  
     double alpha;
     
{
  int n;
  double r,dw,dn;

  r=0.;
  for (n=0;n<u->size-1;n++)
    {
      dw = dJ->values[n+1]-dJ->values[n];
      dn = ((double) u->values[n+1] - u->values[n]) - t * dw;
      r -= (dw * dn) / sqrt( dn*dn + alpha);
    }
  return(r);
}


/* Generate comments in output signal u
*/

void Make_Comments(u,tv,Niter,alpha,O,I)

     Fsignal u;
     double tv;
     int Niter;
     double *alpha;
     Fsignal O;    
     Fimage I;     
     
{
  char A[BUFSIZ];

  if (!alpha)
    sprintf(u->cmt,"TV=%g ",tv);
  else
    sprintf(u->cmt,"TV=%g (alpha=%g) ",tv,*alpha);

  sprintf(A,"N=%d ",Niter);
  strcat(u->cmt,A);
  
  if (O)
    {
      strcat(u->cmt,O->name);
      if (I) sprintf(A," on interval");
      else sprintf(A," periodized");
      strcat(u->cmt,A);
    }

}


/* ----- Main function ----- */

Fsignal stvrestore(relax,Niter,alpha,O,I,V,M,u)

char *relax;    /* If != NULL, relax constraint on approximation space V_J.
		   Warning : on versions < 1.4, this option was active (bug) !
		*/
int *Niter;     /* Number of iterations */
double *alpha;  /* Alpha parameter to smooth the TV */
Fsignal O;      /* Impulse response of orthonormal wavelet */
Fimage I;       /* Edge filter */
Fsignal V;
Fsignal M;      /* Input thresholding mask */
Fsignal u;      /* Input thresholded signal */

{
  int n,r,i,minNiter;
  double tv,tv0,mintv,t;
  double t0=5e-2;/* Initial fixed step */
  Fsignal dJ=NULL;

  if (!O) mwerror(FATAL,1,"At this time option -O is needed !\n");

  /* Compute number of recursion (-j) used for the wavelet transform */
  r=GetWaveletDepth(M);

  dJ = mw_change_fsignal(dJ,u->size);
  if (!dJ) mwerror(FATAL,1,"Not enough memory !\n");

  if (V)
    {
      V = mw_change_fsignal(V,u->size);
      if (!V) mwerror(FATAL,1,"Not enough memory !\n");
      mw_copy_fsignal(u, V); 
      minNiter=1;
    }

  mintv=tv0=1e20;
  for (n=1;n<=*Niter;n++)
    {
      mwdebug("Iteration %d/%d\n",n,*Niter);

      /* Compute J(u) and dJ */
      if (!alpha) tv = J(u,dJ) / (double) u->size;
      else tv = J_alpha(u,dJ,*alpha) / (double) u->size;
      
      /* Compute the projection on the constraint P(dJ) */
      Proj(relax,dJ,M,O,I,&r);

      /* Compute the step t */
      t= ((double) t0/n);

      /* Compute u_{k+1} = u_k - t P(dJ) */
      for (i=0;i<u->size;i++) u->values[i] -= t*dJ->values[i];

      if ((mwdbg==1)||(n==1)||(n==*Niter)||((n%1000)==0))
	{
	  if (!alpha) 
	    printf("\t [%d/%d] total variation J(u)=%g  step t=%g\n",
		   n,*Niter,tv,t);
	  else
	    printf("\t [%d/%d] smoothed total variation J(u)=%g  step t=%g\n",
		   n,*Niter,tv,t);
	}

      tv0=tv;  
      if (V && (mintv > tv))
	{
	  mintv=tv;
	  minNiter = n;
	  mw_copy_fsignal(u, V); 
	}
    }
  
  Make_Comments(u,tv,*Niter,alpha,O,I);
  if (V) Make_Comments(V,mintv,minNiter,alpha,O,I);

  return(u);
}











