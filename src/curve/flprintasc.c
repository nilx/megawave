/*--------------------------- MegaWave2 Command -----------------------------*/
/* mwcommand
   name = {flprintasc};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"Print the content of a Flists"};
   usage = {            
    in->in    "input Flists"
   };
*/

#include <stdio.h>
#include "mw.h"

void flprintasc(in)
Flists in;

{
  Flist l;
  int i,j,k;
  
  for (i=0;i<in->size;i++) {
    l = in->list[i];
    printf("\tFlist #%d (dim %d, size %d): \n",i+1,l->dim,l->size);
    for (j=0;j<l->size;j++) {
      printf("element #%d : ",j+1);
      for (k=0;k<l->dim;k++) 
	printf("%g ",l->values[j*l->dim+k]);
      printf("\n");
    }
    printf("\n");
  }
}




