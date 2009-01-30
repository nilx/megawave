/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {osamss};
 version = {"1.2"};
 author = {"Lionel Moisan"};
 function = { "AMSS as a stack filter (Osher Sethian scheme)"};
 usage = {

  'i'->isotrop   "flag to cancel isotropic diffusion in smooth area",
  'p'->power     "flag to compute AMSS (power 1/3) instead of MCM (power 1)",
  'S':[Step=0.1]->Step [0.0,0.5]     "scale step for each iteration",
  'm':[MinGrad=0.5]->MinGrad [0.0,1e6] 
                 "Minimum gradient norm to compute the curvature",
  'f':[firstScale=0.0]->firstScale   "first scale of diffusion",
  'l':[lastScale=2.0]->lastScale     "last scale of diffusion",

  input->input   "original picture (input Fimage)",
  output<-output "result (Fimage)"

}; */
/*----------------------------------------------------------------------
 v1.1: upgrade for new fvalues() call (L.Moisan)
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h" /* for amss(), fvalues() */

void osamss(char *isotrop, char *power, float *Step, float *MinGrad, float *firstScale, float *lastScale, Fimage input, Fimage output)
              		/* isotropic diffusion if Grad==0 & isotrop != NULL */
            		/* power 1, if == NULL ; power 1/3, if != NULL 	    */
            		/* Step of the scale				    */
                        /* Minimum value of the gradient                    */
                   	/* first scale, equal to zero if not used	    */
                 	/* scale of the last iteration   		    */
             		/* Initial Picture 				    */
              		/* Result         				    */

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

    
