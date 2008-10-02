/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
name = {w1threshold};
version = {"1.0"};
author = {"Jacques Froment"};
function = {"Thresholding of a 1D-Wavelet Transform"};
usage = {
 'P':P->P  "Percent of coefficients to threshold",
 'T':T->T  "Threshold to use (needed if no -P neither -D)",
 'D':D->D  "Use Donoho threshold to denoise signal with additive gaussian noise of the given standard deviation",
 'M':M<-M  "Output mask in the signal M (1|-1=coeff.kept,0=coeff.thresholded)",
 's'->s    "Soft thresholding (default is hard thresholding)",
 'o'->o    "Output threshold used",
 in->in    "Input wavelet transform", 
 out<-out  "Output thresholded wavelet transform" 
	};
*/

#include <stdio.h>
#include <math.h>
#include  "mw.h"


/* Return the percent of thresholded coefficients for the given T */

float getp_threshold(in,T)

     Wtrans1d in;
     float T;

{
  int i,N,j,n,nj,size;

  N=n=0;
  for (j=1; j<= in->nlevel; j++) 
    {
      nj=0;
      size=in->D[j][0]->size;
      N+=size;
      for(i = 0; i<size; i++) 
	if (fabs(in->D[j][0]->values[i]) <= T) nj++;
      /*printf("\tLevel j=%d : T=%g size=%d Thresholded=%d\n",j,T,size,nj);*/
      n+=nj;
    }
  return((100.0*n)/N);
}

/* Do the threshold */

void do_threshold(in,T,s,M)

     Wtrans1d in;
     float T;
     char *s;
     Fsignal M;

{
  int i,j,offset;

  offset=0;
  if (!s)
    /* Hard thresholding */
    {
      for (j=1; j<= in->nlevel; j++) 
	{
	  for(i=0; i<in->D[j][0]->size; i++) 
	    if (fabs(in->D[j][0]->values[i]) <= T) 
	      {
		in->D[j][0]->values[i]=0.0;
		if (M)  M->values[offset+i]=0.0;
	      }
	    else if (M) M->values[offset+i]=1.0;
	  offset += in->D[j][0]->size;
	}
    }
  else
    /* Soft thresholding */
    {
      for (j=1; j<= in->nlevel; j++) 
	{
	  for(i=0; i<in->D[j][0]->size; i++) 
	    if (fabs(in->D[j][0]->values[i]) <= T) 
	      {
		in->D[j][0]->values[i]=0.0;
		if (M)  M->values[offset+i]=0.0;
	      }
	    else
	      if (in->D[j][0]->values[i] > T)
		{
		  in->D[j][0]->values[i]-=T;
		  if (M) M->values[offset+i]=1.0;
		}
	      else
		{
		  in->D[j][0]->values[i]+=T;
		  if (M) M->values[offset+i]=-1.0;
		}
	  offset += in->D[j][0]->size;
	}
    }
    
}

/* Return threshold bound to get P % coeff. thresholded */

float get_threshold_bound(in,P)

     Wtrans1d in;  /* Input wavelet transform */
     float P;
     
{
  float Tmin,Tmax,T,T0,p;
  
  Tmin=0.0;
  Tmax=100.0;
  T=-100;
  
  do
    {
      T0=T;
      T=Tmin + (Tmax-Tmin)/2.0;
      p=getp_threshold(in,T);
      if (p > P) Tmax=T;
      if (p < P) Tmin=T;
      mwdebug("T=%g \t p=%2.1f %% (%2.1f %%) \t [%g,%g]\n",
	      T,p,P,Tmin,Tmax);
      
    } while ((fabs(T-T0)> 1e-6)&&(fabs(p - P)>= 0.1));
  if (fabs(p - P)>= 0.1)
    mwerror(FATAL,1,"Cannot get threshold bound (obtained=%g, required=%g)!\n",
	    p,P);
  return(T);
}


void w1threshold(P,T,D,s,o,M,in,out)
     
     float *P;     /* percent of coefficients to threshold */
     float *T;
     float *D;
     char *s;
     char *o;
     Fsignal M;
     Wtrans1d in;  /* Input wavelet transform */
     Wtrans1d *out;/* Output wavelet transform */
     
{
  float t;
  
  if (in->nvoice > 1) mwerror(FATAL,1,"Does not work with more than one voice per octave !\n");
  if (in->complex > 0) mwerror(FATAL,1,"Does not work with complex wavelet transform !\n");  
  
  if (!P && !T && !D)
    mwerror(USAGE,1,"One option -P, -T or -D has to be selected !\n");

  if (M) 
    {
      M=mw_change_fsignal(M,in->size);
      if (!M) mwerror(FATAL,1,"Not enough memory !\n");
      /* Non meaningful samples set to 2.0 */
      mw_clear_fsignal(M,2.0);
    }

  if (P) t=get_threshold_bound(in,*P);
  if (D) t=(float) *D*sqrt(2.0*log((double) in->size));
  if (T) t=*T;

  if (o) printf("%g\n",t);      

  do_threshold(in,t,s,M);
  *out=in;
}

