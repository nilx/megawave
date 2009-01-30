/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {fsmooth};
   version = {"1.4"};
   author = {"Jacques Froment, Yann Guyonvarc'h"};
   function = {"Smoothing with a binomial filter"};
   usage = {
     'S':[S=2]->S  "standard deviation",
     'W':[W=1]->W  "weight of the considered pixel",
     in->in        "input Fimage",
     out<-out      "output Fimage"
   };
*/
/*----------------------------------------------------------------------
 v1.1: bug nrow/ncol fixed (L.Moisan)
 v1.2: mwcommand syntax fixed (JF)
 v1.3: convolution bug fixed (L.Garrido)
 v1.4 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

#define OUT(i,j) (ptrOUT[(j)*ncOUT + (i)])
#define TMP(i,j) (ptrTMP[(j)*ncOUT + (i)])

void fsmooth(int *S, int *W, Fimage in, Fimage out)
{
  Fimage tmp;
  register int i,j,ncOUT;
  register float *ptrOUT,*ptrTMP;
  int nrOUT,n,N;
  
  N=((2+(*W))*(*S)*(*S));
  
  out = mw_change_fimage(out,in->nrow,in->ncol);
  tmp = mw_change_fimage(NULL,in->nrow,in->ncol);
  if (!out || !tmp) mwerror(FATAL,1,"not enough memory\n"); 
  
  mw_copy_fimage(in,out);

  ptrOUT = out->gray;
  ptrTMP = tmp->gray;

  ncOUT = out->ncol;
  nrOUT = out->nrow;
  
  for (n=1;n<=N;n++)
    { 
      /*horizontal*/
      
      for (j=0;j<(nrOUT);j++)
	for (i=1;i<(ncOUT)-1;i++)
	  TMP(i,j) = (OUT((i-1),j)+OUT(i+1,j)+(*W)*OUT(i,j))/(2+(*W));
      
      for (j=0;j<(nrOUT);j++)
	TMP(ncOUT-1,j) = (OUT(((ncOUT)-2),j)+OUT((ncOUT)-1,j)
			  +(*W)*OUT(((ncOUT)-1),j))/(2+(*W));
      
      for (j=0;j<(nrOUT);j++)
	TMP(0,j)=(OUT(0,j)+OUT(1,j)+(*W)*OUT(0,j))/(2+(*W));
	

     /*vertical*/

     for (i=0;i<(ncOUT);i++)
       for (j=1;j<(nrOUT)-1;j++)
	 OUT(i,j) = (TMP(i,j-1)+TMP(i,j+1)+(*W)*TMP(i,j))/(2+(*W));
     
  
     for (i=0;i<(ncOUT);i++)
        OUT(i,nrOUT-1)=(TMP(i,((nrOUT)-2))+TMP(i,(nrOUT)-1)
			+(*W)*TMP(i,(nrOUT)-1))/(2+(*W));
     
     for (i=0;i<(ncOUT);i++)
       OUT(i,0) = (TMP(i,0)+TMP(i,1)+(*W)*TMP(i,0))/(2+(*W));
 
   }
}
