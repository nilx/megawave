/*----------------------Megawave2 Module-----------------------------------*/
/*mwcommand
  name = {forder};
  version = {"2.1"};
  author = {"Jacques Froment, Yann Guyonvarc'h"};
  function = {"Order filtering : do Erosion/Dilation/Median in a 3x3 window"};
  usage = { 
  'e':[e=5]->e[1,9]  "order of the pixel in a ascending list which sorts the values of the pixels in the window (1..9, default 5)",
  'n':[n=1]->N[1,10000] "number of the filter's iterations (default 1)",
  in->in "Input fimage",
  out<-out "Output fimage"
          };
*/
/*----------------------------------------------------------------------
 v2.0: fixed free() bug  (L.Moisan)
 v2.1: fixed nrow/ncol inversion bug  (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
 
#define MAXSIZE 9
#define IN(i,j) (ptrIN[(j)*ncIN + (i)])


static  int compare(k,l)
         
float *k, *l;
         
{
  if (*k > *l) return(1);
  if (*k < *l) return(-1);
  return(0);
}

void forder(e,N,in,out)
Fimage in,out;
int *e,*N;        
         
{
 float *a;
 int n;
 register int i,j,ncIN;
 register float *ptrIN,*ptrOUT;

 a = ( float *) malloc ( MAXSIZE * sizeof(float)); 
 if (!a) mwerror(FATAL,1,"not enough memory\n");
 (*e)--;

 out=mw_change_fimage(out,in->nrow,in->ncol);
 if (!out) mwerror(FATAL,1,"not enough memory\n"); 
 ncIN = in->ncol;

 for (n=1;n<=*N;n++)
   {
     if (n > 1) mw_copy_fimage(out,in);

     ptrIN = in->gray;
     ptrOUT = out->gray;

     for(j=0;j<(in->nrow);j++)
       for(i=0;i<(in->ncol);i++)
	 {          
	   if (j==0 && i==0)   
	     {
	       a[0] =   IN(i+1,j+1);
	       a[1] =   IN(i,j+1);
	       a[2] =   IN(i+1,j+1);
	       a[3] =   IN(i+1,j);
	       a[4] =   IN(i,j);
	       a[5] =   IN(i+1,j);
	       a[6] =   IN(i+1,j+1);
	       a[7] =   IN(i,j+1);
	       a[8] =   IN(i+1,j+1);
	     }
	   else  if (j==0 && i==((ncIN)-1))    
	     {
	       a[0] =   IN(i-1,j+1);
	       a[1] =   IN(i,j+1);
	       a[2] =   IN(i-1,j+1);
	       a[3] =   IN(i-1,j);
	       a[4] =   IN(i,j);
	       a[5] =   IN(i-1,j);
	       a[6] =   IN(i-1,j+1);
	       a[7] =   IN(i,j+1);
	       a[8] =   IN(i-1,j+1);
	     }
	   else   if (j==0 && i!=0 && i!=((ncIN)-1))    
	     {
	       a[0] =   IN(i-1,j+1);
	       a[1] =   IN(i,j+1);
	       a[2] =   IN(i+1,j+1);
	       a[3] =   IN(i-1,j);
	       a[4] =   IN(i,j);
	       a[5] =   IN(i+1,j);
	       a[6] =   IN(i-1,j+1);
	       a[7] =   IN(i,j+1);
	       a[8] =   IN(i+1,j+1);
	     }
	   else  if (j==((in->nrow)-1) && i==0)    
	     {
	       a[0] =   IN(i+1,j-1);
	       a[1] =   IN(i,j-1);
	       a[2] =   IN(i+1,j-1);
	       a[3] =   IN(i+1,j);
	       a[4] =   IN(i,j);
	       a[5] =   IN(i+1,j);
	       a[6] =   IN(i+1,j-1);
	       a[7] =   IN(i,j-1);
	       a[8] =   IN(i+1,j-1);
	     }
	   else if (j==((in->nrow)-1) && i==((ncIN)-1))    
	     {
	       a[0] =   IN(i-1,j-1);
	       a[1] =   IN(i,j-1);
	       a[2] =   IN(i-1,j-1);
	       a[3] =   IN(i-1,j);
	       a[4] =   IN(i,j);
	       a[5] =   IN(i-1,j);
	       a[6] =   IN(i-1,j-1);
	       a[7] =   IN(i,j-1);
	       a[8] =   IN(i-1,j-1);
	     }
	   else if (j==((in->nrow)-1) && i!=0 && i!=((ncIN)-1))    
	     {
	       a[0] =   IN(i-1,j-1);
	       a[1] =   IN(i,j-1);
	       a[2] =   IN(i+1,j-1);
	       a[3] =   IN(i-1,j);
	       a[4] =   IN(i,j);
	       a[5] =   IN(i+1,j);
	       a[6] =   IN(i-1,j-1);
	       a[7] =   IN(i,j-1);
	       a[8] =   IN(i+1,j-1);
	     }
	   else  if (i==0 && j!=0 && j!=((in->nrow)-1))    
	     {
	       a[0] =   IN(i+1,j+1);
	       a[1] =   IN(i,j+1);
	       a[2] =   IN(i+1,j+1);
	       a[3] =   IN(i+1,j);
	       a[4] =   IN(i,j);
	       a[5] =   IN(i+1,j);
	       a[6] =   IN(i+1,j-1);
	       a[7] =   IN(i,j-1);
	       a[8] =   IN(i+1,j-1);
	     }
	   else  if (i==((ncIN)-1) && j!=0 && j!=((in->nrow)-1))    
	     {
	       a[0] =   IN(i-1,j+1);
	       a[1] =   IN(i,j+1);
	       a[2] =   IN(i-1,j+1);
	       a[3] =   IN(i-1,j);
	       a[4] =   IN(i,j);
	       a[5] =   IN(i-1,j);
	       a[6] =   IN(i-1,j-1);
	       a[7] =   IN(i,j-1);
	       a[8] =   IN(i-1,j-1);
	     }
	   else {
	     a[0] =  IN(i-1,j-1);
	     a[1] =   IN(i,j-1);
	     a[2] =   IN(i+1,j-1);
	     a[3] =   IN(i-1,j);
	     a[4] =   IN(i,j);
	     a[5] =   IN(i+1,j);
	     a[6] =   IN(i-1,j+1);
	     a[7] =   IN(i,j+1);
	     a[8] =   IN(i+1,j+1);
	   }
	   qsort(a,9,sizeof(float) ,compare);
	   *ptrOUT++ = a[*e];
	 }
   }
 free(a);
}
