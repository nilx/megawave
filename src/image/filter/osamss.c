/*--------------------------------------------------------------------------*/
/* mwcommand
name = {osamss};
version = {"1.1"};
author = {"Lionel Moisan"};
function = { "AMSS as a stack filter (Osher Sethian scheme)"};
usage = {

'i'->isotrop      
     "flag to cancel isotropic diffusion in smooth area",
'p'->power        
     "flag to compute AMSS model (power 1/3) instead of MCM (power 1)",
'S':[Step=0.1]->Step [0.0,0.5] 
     "scale step for each iteration (default 0.1)",
'm':[MinGrad=0.5]->MinGrad [0.0,1e6] 
     "Minimum of the gradient norm to compute the curvature (default 0.5)",
'f':[firstScale=0.0]->firstScale 
     "first scale of diffusion (default 0.0)",
'l':[lastScale=2.0]->lastScale   
     "last scale of diffusion (default 2.0)",

input->input   "original picture (input Fimage)",
output<-output "result (Fimage)"

}; */
/*----------------------------------------------------------------------
 v1.1: upgrade for new fvalues() call (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"

extern void amss();
extern Fsignal fvalues();


void osamss(isotrop,power,Step,MinGrad,firstScale,lastScale,input,output)
char *isotrop;		/* isotropic diffusion if Grad==0 & isotrop != NULL */
char *power;		/* power 1, if == NULL ; power 1/3, if != NULL 	    */
float *Step;		/* Step of the scale				    */
float *MinGrad;         /* Minimum value of the gradient                    */
float *firstScale; 	/* first scale, equal to zero if not used	    */
float *lastScale;	/* scale of the last iteration   		    */
Fimage input;		/* Initial Picture 				    */
Fimage output;		/* Result         				    */

{
  Fsignal levels;
  Fimage null_image,evol,binary;
  float l,zero,min,max,mid,v;
  int img_size,i,adr;

  levels = fvalues(NULL,NULL,NULL,input);
  
  if (levels->size >= 50) 
    mwerror(WARNING,1,"%d different grey levels : osamss may take some time !\n",levels->size);

  min = levels->values[0];
  max = levels->values[levels->size-1];
  mid = 0.5*(min+max);
  null_image = NULL;
  img_size = input->nrow*input->ncol;
  zero = 0.0;

  evol = mw_new_fimage();
  output = mw_change_fimage(output,input->nrow,input->ncol);
  mw_clear_fimage(output,min);

  for (i=0;i<levels->size;i++) {
    l = levels->values[i];

    /* binarize input */
    binary = mw_change_fimage(NULL,input->nrow,input->ncol);
    for (adr=img_size;adr--;) 
      binary->gray[adr] = (input->gray[adr]>=l?max:min);
    
    amss(isotrop,power,Step,MinGrad,&zero,firstScale,lastScale,binary,
	 &evol,&null_image,&null_image,NULL,NULL,NULL,NULL);

    /* reconstruct output */    
    for (adr=img_size;adr--;) 
      if ((v=evol->gray[adr])>=mid) output->gray[adr] = l;
  }
}

    
