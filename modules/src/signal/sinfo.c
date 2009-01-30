/*------------------------- MegaWave2 module ------------------------------*/
/* mwcommand
 name = {sinfo};
 author = {"Lionel Moisan"};
 version = {"1.3"};
 function = {"Compute and display several measures on a Fsignal."};
 usage = { 
      s->s    "input Fsignal"
 };
*/

/*----------------------------------------------------------------------
 v1.1: print more field values (JF)
 v1.2: syntax fixed for mwcommand and version (JF)
 v1.3: added standart deviation (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for fvalues(), smean(), snorm() */

void sinfo(Fsignal s)
{  
  float min,max,p;
  double mean,std;
  int i,b;

  printf("-----------------------------------");
  printf("-----------------------------------\n");
  printf("name = %s\n",s->name);
  printf("cmt = %s\n",s->cmt);
  printf("size = %d\n",s->size); 
  printf("scale = %g\n",s->scale); 
  printf("shift = %g\n",s->shift); 
  printf("gain = %g\n",s->gain); 
  printf("sgrate = %g\n",s->sgrate); 
  printf("bpsample = %d\n",s->bpsample); 

  if (s->size) {
    min = max = s->values[0];
    mean = 0.;
    for (i=s->size;i--;) {
      mean += (double)s->values[i];
      if (s->values[i]<min) min = s->values[i];
      if (s->values[i]>max) max = s->values[i];
    }
    mean /= (double)s->size;
    std = 0.;
    for (i=s->size;i--;) 
      std += ((double)s->values[i]-mean)*((double)s->values[i]-mean);
    std = sqrt(std/(double)s->size);
    printf("min = %f\n",min);
    printf("max = %f\n",max);
    printf("mean = %f\n",mean);
    printf("standart deviation = %f\n",std);
  }
  b = 0; p = 2.;
  printf("normalized l2 norm = %f\n",
	 snorm(s,NULL,&p,NULL,NULL,&b,NULL,NULL));
  printf("normalized bv norm = %f\n",
	 snorm(s,NULL,NULL,NULL,(char *)1,&b,NULL,NULL));
  printf("-----------------------------------");
  printf("-----------------------------------\n");
}
