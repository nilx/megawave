/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {cextract};
author = {"Lionel Moisan"};
function = {"Extract a subpart of a Cimage"};
version = {"1.8"};
usage = {
 'b':[b=0]->b  "background grey level",
 'r'->r        "if set, X2 and Y2 must be the SIZE of the extracted region",
 in->in        "input Cimage",
 out<-out      "output Cimage",
 X1->X1        "upleft corner of the region to extract from input (x)",
 Y1->Y1        "upleft corner of the region to extract from input (y)",
 X2->X2        "downright corner of the region to extract from input (x)",
 Y2->Y2        "downright corner of the region to extract from input (y)",
  {
    bg->bg       "background Cimage",
    [Xc=0]->Xc   "new location of X1 on the background",
    [Yc=0]->Yc   "new location of Y1 on the background"
  }
};
*/
/*-------------------------------------------------------------------------
 v1.6: module rewritten, extended parameter values, new -r option (L.Moisan)
 v1.8 (04/2007): simplified header (LM)
-------------------------------------------------------------------------*/

#include  "mw.h"

#define MAX(x,y) ((x)>(y)?(x):(y))


Cimage cextract(int *b, Cimage in, Cimage bg, Cimage out, int X1, int Y1, int X2, int Y2, int *Xc, int *Yc, char *r)
{
  int x,y,pos1,pos2;

  /* test relative coordinates */
  if (r) {X2+=X1-1; Y2+=Y1-1;}
  if (X2<=0) X2=in->ncol+X2-1;
  if (Y2<=0) Y2=in->nrow+Y2-1;

  if (X2<X1 || Y2<Y1) 
    mwerror(FATAL,1,"empty region to extract: (%d,%d)-(%d,%d)\n",X1,Y1,X2,Y2);

  if (bg) {
    out = mw_change_cimage(out,MAX(bg->nrow,*Yc+Y2-Y1+1),MAX(bg->ncol,*Xc+X2-X1+1));
    mw_clear_cimage(out,*b);
    for (x=0;x<bg->ncol;x++) 
      for (y=0;y<bg->nrow;y++) 
	out->gray[y*out->ncol+x] = bg->gray[y*bg->ncol+x];
  } else {
    out = mw_change_cimage(out,Y2-Y1+1,X2-X1+1);
    mw_clear_cimage(out,*b);
  }

  for (x=X1;x<=X2;x++)
    for (y=Y1;y<=Y2;y++) {
      pos1 = y*in->ncol+x;
      pos2 = (*Yc+y-Y1)*out->ncol+(*Xc+x-X1);
      if (*Yc+y-Y1>=0 && *Yc+y-Y1<out->nrow && 
	  *Xc+x-X1>=0 && *Xc+x-X1<out->ncol &&
	  x>=0 && x<in->ncol && y>=0 && y<in->nrow) {
	out->gray[pos2] = in->gray[pos1];
      }  
    }      

  return(out);
}

