/*----------------------------- MegaWave Module -----------------------------*/
/* mwcommand
  name = {cccrop};
  version = {"1.1"};
  author = {"Lionel Moisan"};
  function = {"Color image croping (with zoom) using interpolation."};
  usage = {
'x':sx->sx         "force x-size of output image",
'y':sy->sy         "force y-size of output image",
'z':z->z           "zoom factor (default 1.0)",
'b':[bg=0]->bg     "background grey value, default: 0",
'o':[o=3]->o       "order: 0,1=linear,-3=cubic,3,5..11=spline, default 3",
'p':p->p           "Keys' parameter (when o=-3), in [-1,0], default -0.5",
in->in             "input Ccimage",
out<-out           "output Ccimage",
X1->X1             "upleft corner",
Y1->Y1             "upleft corner",
X2->X2             "downright corner",
Y2->Y2             "downright corner"
};
*/

#include <stdio.h>
#include "mw.h"

extern Fimage fcrop();

/* NB : calling this module with out=in is possible */

Ccimage cccrop(in,out,sx,sy,z,bg,o,p,X1,Y1,X2,Y2)
     Ccimage        in,out;
     float          *sx,*sy,*z,*p,X1,Y1,X2,Y2;
     unsigned char  *bg;
     int            *o;
{
  Cimage aux_in,aux_out;
  Fimage tmp1,tmp2;
  float fbg;

  aux_in = mw_new_cimage(); 
  aux_out = mw_new_cimage(); 
  aux_in->nrow = in->nrow; 
  aux_in->ncol = in->ncol;
  fbg = (float)(*bg);

  /* red component */
  aux_in->gray = in->red;
  tmp1 = mw_cimage_to_fimage(aux_in,NULL);
  tmp2 = fcrop(tmp1,NULL,sx,sy,z,&fbg,o,p,X1,Y1,X2,Y2);
  out = mw_change_ccimage(out,tmp2->nrow,tmp2->ncol);
  aux_out->nrow = tmp2->nrow; 
  aux_out->ncol = tmp2->ncol;
  aux_out->gray = out->red;
  mw_fimage_to_cimage(tmp2,aux_out);

  /* green component */
  aux_in->gray = in->green;
  aux_out->gray = out->green;
  mw_cimage_to_fimage(aux_in,tmp1);
  fcrop(tmp1,tmp2,sx,sy,z,&fbg,o,p,X1,Y1,X2,Y2);
  mw_fimage_to_cimage(tmp2,aux_out);

  /* blue component */
  aux_in->gray = in->blue;
  aux_out->gray = out->blue;
  mw_cimage_to_fimage(aux_in,tmp1);
  fcrop(tmp1,tmp2,sx,sy,z,&fbg,o,p,X1,Y1,X2,Y2);
  mw_fimage_to_cimage(tmp2,aux_out);

  mw_delete_fimage(tmp2);
  mw_delete_fimage(tmp1);
  free(aux_out); 
  free(aux_in); 

  return(out);
}
