/*--------------------------- MegaWave2 module  -----------------------------*/
/* mwcommand
 name = {fft1d};
 author = {"Chiaa Babya, Jacques Froment, Lionel Moisan"};
 version = {"1.2"};
 function = {"Compute the Fast Fourier Transform of a complex signal"};
 usage = {     
 'i'->inverse  "Compute the Inverse Transform",
 'I':Xi->Xi    "Input signal (imaginary part)",
 'A':Yr<-Yr    "Output signal (real part)",
 'B':Yi<-Yi    "Output signal (imaginary part)",
  Xr->Xr       "Input signal (real part)"
};
*/
/*----------------------------------------------------------------------
 v1.1: power_of_two test (fixed bug) + optional input/outputs (L.Moisan)
 v1.2: preserve header info for e.g. sound processing (JF) 
----------------------------------------------------------------------*/

#include <stdio.h> 
#include  <math.h>
#include  "mw.h"

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr;

int is_a_power_of_two(n)
int n;
{
  if (n<1) return(0);
  while ((n&1)==0) n=(n>>1);
  return(n==1);
}

void fft1d(Xr,Xi,Yr,Yi,inverse)
Fsignal Xr,Xi,Yr,Yi;
char *inverse;
{
  int size,n,mmax,m,l,j,istep,i,isign;
  double wtemp,wr,wpr,wpi,wi,theta;
  float tempr,tempi;
  Fsignal data;
  
  size = Xr->size;

  if (!is_a_power_of_two(size))
    mwerror(USAGE,1,"Size of input signal must be a power of 2 !\n");

  if (Xi) if (Xi->size!=size)
    mwerror(USAGE,1,"Real and imaginary parts are of different size !\n");
   
  if (!inverse) isign=1; else isign=-1;
  
  data=mw_change_fsignal(NULL,2*size);
  for (i=0;i<size;i++)
    {data->values[2*i]=Xr->values[i];
    data->values[2*i+1]=(Xi?Xi->values[i]:0.0);}
  
  /*--- compute FFT of "data" array ---*/

  n=size << 1;
  j=1;
  for (i=1;i<n;i+=2) {
    if (j>i) {
      SWAP(data->values[j-1],data->values[i-1]);
      SWAP(data->values[j],data->values[i]);
    }
    m=n >> 1;
    while (m >=2 && j>m) {
      j=j-m;
      m >>=1;
    }
    j=j+m;
  }
  mmax=2;
  while (n>mmax) {
    istep=2*mmax;
    theta=2*M_PI/(isign*mmax);
    wtemp=sin(0.5*theta);
    wpr=-2.0*wtemp*wtemp;
    wpi=sin(theta);
    wr=1.0;
    wi=0.0;
    for (m=1;m<mmax;m+=2) {
      for (i=m-1;i<=n-1;i+=istep) {
	j=i+mmax;
	tempr=wr*data->values[j]-wi*data->values[j+1];
	tempi=wr*data->values[j+1]+wi*data->values[j];
	data->values[j]=data->values[i]-tempr;
	data->values[j+1]=data->values[i+1]-tempi;
	data->values[i]=data->values[i]+tempr;
	data->values[i+1]=data->values[i+1]+tempi;
      }
      wr=(wtemp=wr)*wpr-wi*wpi+wr;
      wi=wi*wpr+wtemp*wpi+wi;
    }
    mmax=istep;
  }

  /*--- store result ---*/
  if (Yr) {
    Yr=mw_change_fsignal(Yr,size);
    if (!Yr) mwerror(FATAL,1,"Not enough memory !\n");
    mw_copy_fsignal_header(Xr,Yr);
    if (inverse) 
      for (i=0;i<size;i++)
	Yr->values[i]=data->values[2*i]/(float)size;
    else
      for (i=0;i<size;i++)
	Yr->values[i]=data->values[2*i];
  }
  if (Yi) {
    Yi=mw_change_fsignal(Yi,size);
    if (!Yi) mwerror(FATAL,1,"Not enough memory !\n");
    if (Xi) mw_copy_fsignal_header(Xi,Yi);
    else mw_copy_fsignal_header(Xr,Yi);
    if (inverse) 
      for (i=0;i<size;i++)
	Yi->values[i]=data->values[2*i+1]/(float)size;
    else 
      for (i=0;i<size;i++)
	Yi->values[i]=data->values[2*i+1];
  }
  
  mw_delete_fsignal(data);
}
