/*----------------------Megawave2 Module-----------------------------------*/
/*mwcommand
  name = {segtxt};
  version = {"1.1"};
  author = {"Yann Guyonvarc'h"};
  function = {"Texture Segmentation using multi-scales multi-channels representation"};
  usage = {

'N':[N=1]->N      "# images per channel, default 1 (for local scale value)",
'S':[S=1]->S      "standard deviation of the smoothing filter, default 1",
'W':[W=1]->W      "pixel weight for the smoothing filter, default 1",
'p':[p=2]->p[1,2] "scalar distance: ABS (p=1) or Quadratic (p=2,default)",
'n':nr->nr        "number of desired regions",
in->in            "input Fimage",
mov<-mov          "output Fmovie",
out<-segtxt       "output segmented Cimage"

};
*/
/*----------------------------------------------------------------------
 v1.1: return, p option (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
 
extern Cimage msegct();
extern void mschannel();

Cimage segtxt(N,S,W,p,nr,in,mov)
     int    *N,*W,*nr,*S,*p;
     Fimage in;
     Fmovie mov;
{
  float f;
  int nbr;     /* not used by our application */
  int sgrid=1; /* used to fix a bug in msegct default values */ 
 
  mschannel(N,S,W,p,in,mov);   
  return(msegct(NULL,&sgrid,nr,NULL,NULL,NULL,&nbr,&f,mov));
 }

