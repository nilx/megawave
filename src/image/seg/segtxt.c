/*----------------------Megawave2 Module-----------------------------------*/
/*mwcommand
  name = {segtxt};
  version = {"1.0"};
  author = {"Yann Guyonvarc'h"};
  function = {"Texture Segmentation using multi-scales multi-channels representation"};
  usage = {
            'N':[N=1]->N "Number of images per channel - involved in the local scale value",
            'S':[S=1]->S "Sigma - Standard deviation of the smoothing filter",
            'W':[W=1]->W "Weight of the pixel in the smoothing filter",
            'p':[p=2]->p " - 1 for ABS - 2 for Quadratic difference",
            'n':nr->nr "Number of desired regions",
            in->in "Input Fimage",
            movieout<-fmovieout "output Fmovie",
            out<-out "Output segmented Cimage"
          };
*/
/*-------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
 
#ifdef __STDC__
Cimage msegct(Fsignal,int*,int*,float*,Curves,Fmovie,int*,float*,Fmovie);
void channel(int*,float*,int*,float*,Fimage,Fmovie);
#else
Cimage msegct();
void channel();
#endif


void segtxt(N,S,W,p,nr,in,fmovieout,out)
 int *N,*W,*nr,*S;
 float *p;
 Fimage in;
 Fmovie fmovieout;
 Cimage out;

{
  float f;
  int nbr;     /* not used by our application */
  int sgrid=1; /* used to fix a bug in msegct default values */ 
 
  mschannel(N,S,W,p,in,fmovieout);   
 
  out=mw_change_cimage(out,in->nrow,in->ncol);
  if (!out) mwerror(FATAL,1,"Not enough memory.\n");

  *out=*((Cimage) msegct(NULL,&sgrid,nr,NULL,NULL,NULL,&nbr,&f,fmovieout));
 }

