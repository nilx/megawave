/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {wp2dfreqorder};
version = {"1.0"};
author = {"Francois Malgouyres"};
function = {"Make the correspondence between a wavelet packet index and a frequency band"};
usage = {
   'i'->inverse_flag
      "To make the correspondance between a frequency band and a wavelet packet index",
   level->level    "Maximum decomposition level of the wavelet packet basis",
   Fsignal<-order  "Fsignal containing the correspondance"
};
*/
/*----------------------------------------------------------------------
 v1.0: adaptation from pack2frequencyOrder v.0.1 (fpack) (JF)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include "mw.h"

/***************************************************************/
static void switchOrder(Fsignal order)
{Fsignal tmp;
 int i,j;
 
 tmp=mw_change_fsignal(NULL,order->size);

 for(i=0;i<order->size;i++)
   {j=(int) order->values[i] - 1;

   tmp->values[j]=(float) (i+1);
   }

 mw_copy_fsignal(tmp,order);

 mw_delete_fsignal(tmp);
}
     
/*-------------------------------------------------------------*/
/*----------------MAIN PROGRAM--------------------*/
/*-------------------------------------------------------------*/

void wp2dfreqorder(int level, Fsignal order, char *inverse_flag)
{int i,x,xa,tj,k;
 int *work,*work_old;
 
 for(i=1,tj=1;i<=level;i++)
   tj*=2;

 if ((order = mw_change_fsignal(order,tj)) == NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");
  
  xa=1;
  work_old = (int *) malloc(xa*sizeof(int));

  work_old[0]=1;

  for (x=2;x<=tj;x*=2)
    { work = (int *) malloc(x*sizeof(int));
      
      for(k=0;k<xa;k++)
	{ if(k%2==0)
	    {work[2*k]=2*work_old[k]-1;
	     work[2*k+1]=2*work_old[k];}
	  else
	    {work[2*k]=2*work_old[k];
	     work[2*k+1]=2*work_old[k]-1;}
	}
      free(work_old);
      xa=x;
      work_old = (int *) malloc(xa*sizeof(int));
     
      for(k=0;k<x;k++)
	work_old[k]=work[k];
      free(work);
    }


  for(k=0;k<tj;k++)
    order->values[k]=(float) work_old[k];

  if(inverse_flag==NULL)
    switchOrder(order);
  
  free(work_old);
}
