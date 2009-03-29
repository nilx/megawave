/*--------------------------- MegaWave2 module  -----------------------------*/
/* mwcommand
 name = {fct1d};
 author = {"Chiaa Babya, Jacques Froment"};
 version = {"1.2"};
 function = {"Computes the Fast Cosine Transform of a signal"};
 usage = {     
 'i'->inverse "Compute the Inverse Transform",
 X->X "Input signal",
 Y<-Y "Output signal"
};
*/
/*----------------------------------------------------------------------
 v1.1: fixed bug : power_of_two test (L.Moisan)
 v1.2: preserve header info for e.g. sound processing (JF) 
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for fft1d() */

static int is_a_power_of_two(int n)
{
  if (n<1) return(0);
  while ((n&1)==0) n=(n>>1);
  return(n==1);
}

static void realft(Fsignal data, int n, char *inverse)
{
  int i,i1,i2,i3,i4,n2p3;
  float c1=0.5,c2,h1r,h1i,h2r,h2i;
  double wr,wi,wpr,wpi,wtemp,theta;
  Fsignal data1=NULL;
  Fsignal data2=NULL;
  
  data1=mw_change_fsignal(data1,n);
  data2=mw_change_fsignal(data2,n);
  for (i=0;i<n;i++)
    {data1->values[i]=data->values[2*i];
    data2->values[i]=data->values[2*i+1];}
  theta=M_PI/(double) n;
  if (!inverse) {
    c2=-0.5;
    fft1d(data1,data2,data1,data2,NULL);
    for (i=0;i<n;i++)
      {data->values[2*i]=data1->values[i];
      data->values[2*i+1]=data2->values[i];}
  } else {
    c2=0.5;
    theta=-theta;
  }
  wtemp=sin(0.5*theta);
  wpr=-2.0*wtemp*wtemp;
  wpi=sin(theta);
  wr=1.0+wpr;
  wi=wpi;
  n2p3=2*n+3;
  for (i=2;i<=n/2;i++)
    {i4=1+(i3=n2p3-(i2=1+(i1=i+i-1)));
    h1r=c1*(data->values[i1-1]+data->values[i3-1]);
    h1i=c1*(data->values[i2-1]-data->values[i4-1]);
    h2r=-c2*(data->values[i2-1]+data->values[i4-1]);
    h2i=c2*(data->values[i1-1]-data->values[i3-1]);
    data->values[i1-1]=h1r+wr*h2r-wi*h2i;
    data->values[i2-1]=h1i+wr*h2i+wi*h2r;
    data->values[i3-1]=h1r-wr*h2r+wi*h2i;
    data->values[i4-1]=-h1i+wr*h2i+wi*h2r;
    wr=(wtemp=wr)*wpr-wi*wpi+wr;
    wi=wi*wpr+wtemp*wpi+wi;
    }
  if (!inverse) {
    data->values[0]=(h1r=data->values[0])+data->values[1];
    data->values[1]=h1r-data->values[1];
  }
  else{
    data->values[0]=c1*((h1r=data->values[0])+data->values[1]);
    data->values[1]=c1*(h1r-data->values[1]);
    for (i=0;i<n;i++)
      {data1->values[i]=data->values[2*i];
      data2->values[i]=data->values[2*i+1];}
    fft1d(data1,data2,data1,data2,NULL);
    for (i=0;i<n;i++)
      {data->values[2*i]=data1->values[i];
      data->values[2*i+1]=data2->values[i];}
  }
  
  mw_delete_fsignal(data2);
  mw_delete_fsignal(data1);
}


void fct1d(Fsignal X, Fsignal Y, char *inverse)
{
  int j,m,n2,n;
  float enf0,even,odd,sum,sume,sumo,y1,y2;
  double theta,wi=0.0,wr=1.0,wpi,wpr,wtemp;
  
  n = X->size;
  
  if (!is_a_power_of_two(n))
    mwerror(USAGE,1,"Size of input signal must be a power of 2 !\n");
  
  Y=mw_change_fsignal(Y,n);
  if (Y == NULL) mwerror(FATAL,1,"Not enough memory !\n");
  mw_copy_fsignal(X,Y);
  mw_copy_fsignal_header(X,Y);
  
  theta=M_PI/(double) n;
  wtemp=sin(0.5*theta);
  wpr=-2.0*wtemp*wtemp;
  wpi=sin(theta);
  sum=Y->values[0];
  m=n >> 1;
  n2=n+2;
  for (j=2;j<=m;j++)
    {wr=(wtemp=wr)*wpr-wi*wpi+wr;
    wi=wi*wpr+wtemp*wpi+wi;
    y1=0.5*(Y->values[j-1]+Y->values[n2-j-1]);
    y2=(Y->values[j-1]-Y->values[n2-j-1]);
    Y->values[j-1]=y1 -wi*y2;
    Y->values[n2-j-1]=y1+wi*y2;
    sum=sum + wr*y2;}
  realft(Y,m,NULL);
  Y->values[1]=sum;
  for (j=3;j<=n-1;j+=2) 
    {sum=sum + Y->values[j];
    Y->values[j]=sum;}
  
  if (inverse) {
    even=Y->values[0];
    odd=Y->values[1];
    for (j=2;j<=n-2;j+=2)
      {even=even + Y->values[j];
      odd=odd + Y->values[j+1];
      }
    enf0=2.0*(even-odd);
    sumo=Y->values[0]-enf0;
    sume=(2.0*odd/n)-sumo;
    Y->values[0]=0.5*enf0;
    Y->values[1]=Y->values[1]-sume;
    for (j=2;j<=n-2;j+=2)
      {Y->values[j]=Y->values[j]-sumo;
      Y->values[j+1]=Y->values[j+1]-sume;
      }}
  
  if (inverse)
    for (j=0;j<Y->size;j++)
      Y->values[j] = 2.0 * Y->values[j] / Y->size;
}




