/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fft2dview};
   version = {"1.1"};
   author = {"Lionel Moisan"};
   function = {"Compute and Visualize the 2D-FFT of a Fimage"};
   usage = {
 't':[type=0]->type "0=modulus (default),1=phase,2=Re,3=Im,4=log(1+modulus)",
 's':[sd=50.0]->sd  "standart deviation for visualization (default: 50.0)",
 'i'->i_flag        "to compute inverse FFT",
 'h'->h_flag        "to apply a Hamming window first",
 'o':out<-out       "to create an output Cimage instead of calling cview",
 input->in          "input Fimage"
   };
*/
/*----------------------------------------------------------------------
 v1.1: fmeanvar() replaced by faxpb() (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

/* Include the window since we use windows facility */
#include "window.h"

extern void    fshrink2();
extern void    fhamming();
extern void    fft2d();
extern void    fft2dpol();

extern void    faxpb();
extern         cview();


#define PIXRANGE(c) ( (c)<0?0:((c)>255?255:(unsigned char)(c)) )


void fft2dview(type,sd,h_flag,in,out,i_flag)
int	*type;
float	*sd;
char    *h_flag;
Fimage	in;
Cimage  out;
char    *i_flag;
{
    Fimage      tmp;
    int		x,y,i,n,p,n0,p0, out_flag;
    float       v, mean;
    
    /*** for cview ***/
    int         x0,y0;
    float       zoom;
    Wframe      *ImageWindow;


    out_flag = (out!=NULL);

    /*** Prepare input image (shrink, hamming) ***/
    tmp = mw_new_fimage();

    n0 = in->nrow;
    p0 = in->ncol;
    fshrink2(in,tmp);
    n = tmp->nrow;
    p = tmp->ncol;
    if (n!=n0 || p!=p0) 
      mwerror(WARNING,0,"%dx%d input image has been shrinked to %dx%d\n",
	      p0,n0,p,n);

   if (h_flag) fhamming(tmp,tmp);

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

    /*** Normalize mean and variance ***/
    mean = 128.0;
    faxpb(tmp,tmp,NULL,sd,NULL,&mean,NULL);

    /*** Center output and sample to unsigned char values ***/
    out = mw_change_cimage(out,tmp->nrow,tmp->ncol);
    for (x=0; x<p; x++)
      for (y=0; y<n; y++) {
	v = tmp->gray[y*p+x];
	out->gray[((y+n/2)%n)*p+(x+p/2)%p] = PIXRANGE(v);
      }
    
    if (!out_flag) {
      ImageWindow = (Wframe *)
	mw_get_window((Wframe *)NULL,out->ncol,out->nrow,x0,y0,out->name);
      x0 = y0 = 50;
      zoom = 1.0;
      cview(out,&x0,&y0,&zoom,NULL,ImageWindow);
      mw_delete_cimage(out);
    };

    /*** free memory ***/
    mw_delete_fimage(tmp);
} 

 
