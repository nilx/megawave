/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {fsharpen};
author = {"Jacques Froment"};
version = {"1.1"};
function = {"Sharpening using linear filtering"};
usage = {
   'p':[p=75]->p [0,99]  "percent of sharpening, default is 75%",
   A->A                  "Input Fimage",
   B<-B                  "Output sharpened Fimage"
};
*/

/*----------------------------------------------------------------------
 v1.1 : fixed bug on non-square image (JF)
----------------------------------------------------------------------*/


#include <stdio.h>
#include  "mw.h"

Fimage fsharpen(A,B,p)
     
     Fimage A,B;
     float *p;
     
{
  register float *a,*b;
  int x,y;
  float u,v,d,sum;
  
  if ((A->nrow<3)||(A->ncol<3))
    mwerror(FATAL,1,"Image too small !\n");
  B = mw_change_fimage(B, A->nrow, A->ncol);
  if (B == NULL) mwerror(FATAL,1,"Not enough memory.\n");
  
  u=*p/100.0;
  v=9.0-u;
  d=9.0*(1.0-u);

  a=A->gray; b=B->gray;
  for (y=0; y<A->nrow; y++)
    for (x=0; x<A->ncol;x++)
      {
	if ((x==0)||(x==A->ncol-1)||(y==0)||(y==A->nrow-1)) 
	  *b=*a;
	else
	  {
	    sum = *(a-1) + *(a+1)+ *(a-A->ncol) +  *(a+A->ncol) +
	      *(a-1-A->ncol) + *(a-1+A->ncol) +
	      *(a+1-A->ncol) + *(a+1+A->ncol);
	    *b = (v * *a - u*sum)/d;
	  }
	a++; b++;
      }
  
  return(B);
}

