/*----------------------Megawave2 Module----------------------*/
/*mwcommand
  name = {fsmooth};
  version = {"1.1"};
  author = {"Jacques Froment, Yann Guyonvarc'h"};
  function = {"Smoothing with a binomial filter"};
  usage = {
            'S':[S=2]->S  "standard deviation (default 2)",
            'W':[W=1]->W  "Weight of the considered pixel (default 1)",
            in->in        "Input Fimage",
            out<-out      "Output Fimage"
          };
*/
/*----------------------------------------------------------------------
 v1.1: bug nrow/ncol fixed (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

#define OUT(i,j) (ptrOUT[(j)*ncOUT + (i)])

void fsmooth(S,W,in,out)

int *S,*W;
Fimage in,out;

{

 register int i,j,ncOUT;
 register float *ptrOUT;
 int nrOUT,n,N;
 
 N=((2+(*W))*(*S)*(*S));

 out=mw_change_fimage(out,in->nrow,in->ncol);
 if (!out) mwerror(FATAL,1,"not enough memory\n"); 

 mw_copy_fimage(in,out);
 ptrOUT = out->gray;
 ncOUT = out->ncol;
 nrOUT = out->nrow;

 for (n=1;n<=N;n++)
   { 

     /*horizontal*/

     for (j=0;j<(nrOUT);j++)
       for (i=1;i<(ncOUT)-1;i++)
	 OUT(i,j) = (OUT((i-1),j)+OUT(i+1,j)+(*W)*OUT(i,j))/(2+(*W));

     for (j=0;j<(nrOUT);j++)
       OUT(ncOUT-1,j) = (OUT(((ncOUT)-2),j)+OUT((ncOUT)-1,j)+(*W)*OUT(((ncOUT)-1),j))/(2+(*W));

     for (j=0;j<(nrOUT);j++)
       OUT(0,j)=(OUT(0,j)+OUT(1,j)+(*W)*OUT(0,j))/(2+(*W));
	

     /*Vertical*/

     for (i=0;i<(ncOUT);i++)
       for (j=1;j<(nrOUT)-1;j++)
	 OUT(i,j) = (OUT(i,j-1)+OUT(i,j+1)+(*W)*OUT(i,j))/(2+(*W));
     
  
     for (i=0;i<(ncOUT);i++)
        OUT(i,nrOUT-1)=(OUT(i,((nrOUT)-2))+OUT(i,(nrOUT)-1)+(*W)*OUT(i,(nrOUT)-1))/(2+(*W));
     
     for (i=0;i<(ncOUT);i++)
       OUT(i,0) = (OUT(i,0)+OUT(i,1)+(*W)*OUT(i,0))/(2+(*W));
 
   }
}
