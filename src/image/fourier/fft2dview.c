/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fft2dview};
 version = {"1.7"};
 author = {"Lionel Moisan"};
 function = {"Compute and Visualize the 2D-FFT of a Fimage"};
 usage = {
   't':[type=0]->type "0=modulus,1=phase,2=Re,3=Im,4=log(1+modulus)",
   'd':[d=1.]->d      "discard d percent of the extremal values",
   'i'->i_flag        "to compute inverse FFT",
   'h'->h_flag        "to apply a Hamming window first",
   'o':out<-out       "to create an output Fimage instead of calling fview",
   input->in          "input Fimage"
};
*/
/*----------------------------------------------------------------------
 v1.1: fmeanvar() replaced by faxpb() (L.Moisan)
 v1.2: cview syntax (L.Moisan)
 v1.3: min = 0 when type = 0 (L.Moisan)
 v1.4: upgrade faxpb() call (L.Moisan)
 v1.5: allow any image size (not only powers of two !) (LM)
 v1.6: remove -s and add -d option to improve display with fview (LM)
 v1.7 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

/* Include the window since we use windows facility */
#include "window.h"

extern void    fhamming();
extern void    fft2d();
extern void    fft2dpol();
extern void    fview();


void fft2dview(type,h_flag,in,out,i_flag,d)
     int     *type;
     float   *d;
     char    *h_flag,*i_flag;
     Fimage  in,out;
{
  Fimage      tmp;
  int	      x,y,i,n,p,out_flag;
  
  /*** for fview ***/
  int         x0=50,y0=50,order=0;
  float       zoom=1.0;
  Wframe      *ImageWindow;
  
  out_flag = (out!=NULL);
  
  /*** Prepare input image (hamming) ***/
  n = in->nrow;
  p = in->ncol;
  tmp = mw_change_fimage(NULL,n,p);
  if (!tmp) mwerror(FATAL,1,"Not enough memory\n");
  if (h_flag) fhamming(in,tmp); else mw_copy_fimage(in,tmp);
  
  /*** Compute desired part of FFT ***/
  switch(*type) {
    
  case 0: fft2dpol( tmp,NULL,tmp ,NULL,i_flag); break;
  case 1: fft2dpol( tmp,NULL,NULL,tmp ,i_flag); break;
  case 2: fft2d(    tmp,NULL,tmp ,NULL,i_flag); break;
  case 3: fft2d(    tmp,NULL,NULL,tmp ,i_flag); break;
  case 4: fft2dpol( tmp,NULL,tmp ,NULL,i_flag); 
    for (i=n*p; i--; ) 
      tmp->gray[i] = (float)log1p((double)tmp->gray[i]);
    break;
    
  default: mwerror(FATAL,1,"Unrecognized argument value : type.");
  }
  
  /*** Center output ***/
  out = mw_change_fimage(out,tmp->nrow,tmp->ncol);
  if (!out) mwerror(FATAL,1,"Not enough memory\n");
  for (x=0; x<p; x++)
    for (y=0; y<n; y++) 
      out->gray[((y+n/2)%n)*p+(x+p/2)%p] = tmp->gray[y*p+x];

  if (!out_flag) {
    ImageWindow = (Wframe *)
      mw_get_window((Wframe *)NULL,out->ncol,out->nrow,x0,y0,out->name);
    fview(out,&x0,&y0,&zoom,&order,NULL,ImageWindow,NULL,NULL,NULL,d);
    mw_delete_fimage(out);
  }
  mw_delete_fimage(tmp);
} 

 
