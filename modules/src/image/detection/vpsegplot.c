/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {vpsegplot};
  version = {"1.0"};
  author = {"Andres Almansa"};
  function = {"Display vanishing points and associated segments (output of vpoint)"};
  usage = {
  'l'->lines        "show supporting lines instead of segments",
  image->image      "input image (only used to obtain dimensions)",
  allsegs->allsegs  "input 5-Flist of all detected segments (x1 y1 x2 y2 -log10(nfa))",
  vpoints->vpoints  "input 9-Flist of vanishing regions detected by vpoint (x1 y1 x2 y2 x3 y3 x4 y4 -log10(nfa))",
  vsegs->vsegs      "input segments indexes associated to each detected vanishing region (Flists)",
  n->n              "input integer: index of vanishing region to display (for n out of bounds, segments not contributing to any vp will be displayed)",
  crv<-crv          "store n-th vp and contributing segments as curves (output, Flists)"
          };
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"

#define SegmentsAsFimage

#define SegX0 0
#define SegY0 1
#define SegX1 2
#define SegY1 3
#define SegMeaning 4
#define SegPrecision 5
#ifdef SegmentsAsFimage
#define SegListDim(S)       ((S)->ncol)
#define SegListSize(S)      ((S)->nrow)
#define SegList(S,i)        ((S)->gray+((i)*(S)->ncol))
#else
#define SegListDim(S)       ((S)->dim)
#define SegListSize(S)      ((S)->size)
#define SegList(S,i)        ((S)->values+(i)*(S)->dim))
#endif
#define SegListX0(S,i)        (SegList(S,i)[SegX0])
#define SegListY0(S,i)        (SegList(S,i)[SegY0])
#define SegListX1(S,i)        (SegList(S,i)[SegX1])
#define SegListY1(S,i)        (SegList(S,i)[SegY1])
#define SegListMeaning(S,j)   (SegListDim(S) > SegMeaning)? \
                               SegList(S,i)[SegMeaning] : \
                               NAN)
#define SegListPrecision(S,j) (SegListDim(S) > SegPrecision)? \
                               SegList(S,j)[SegPrecision] : \
                               NAN)



#define EPS 1.0e-16 /* Machine double precision */
#define Inf (-log(0.0))
/*#define Inf INFINITY*/

#define max(x,y) (x>y)?x:y
#define _(I,x,y) ((I)->gray[(y)*((I)->ncol)+(x)] /* 0 <= x < I->ncol
						    0 <= y < I->nrow */


/*------------------------------------------------------------*/
/*                         MAIN MODULE                        */
/*------------------------------------------------------------*/


void vpsegplot(Fimage image, Fimage allsegs, Flist vpoints, Flists vsegs, int n, Flists crv, char *lines)
{
  int i,j, m, seg_idx, *ismember;
  double R,X0,Y0,dx,dy;
  float *csegs;
  int    nsegs;
  float x0,x1;
  float y0,y1;
  int vdim, sdim;
  double xbar, ybar, vp_xbar, vp_ybar, fact, target_length;
  
  /* Radius and center of circumscribed circle containing image domain */
     R = sqrt((double) image->nrow * (double) image->nrow
	      + (double)image->ncol * (double) image->ncol) / 2.0; 
     X0 = (double)image->ncol/2.0;
     Y0 = (double)image->nrow/2.0;


  /* Convert segments and vanishing points to a list of curves
     to be displayed by fkview */
     /*
     if (n>=vpoints->size)
       mwerror(ERROR,1,"The number n should be smaller than the total number of detected vanishing points!");
     */
     vdim   = vpoints->dim;
     if (vdim<8)
       mwerror(ERROR,1,"vpoints should be an Flist of dimension >= 8 !");
     sdim   = SegListDim(allsegs);
     if (sdim<4)
       mwerror(ERROR,1,"allsegs should be a list (Fimage or Flist) of dimension >= 4 !");
     if ((n>=vpoints->size) || (n<0)) {
     /* For n out of bounds display segments not contributing to any vpoint */
	 m = SegListSize(allsegs);
	 ismember = (int*) calloc(m,sizeof(int));
	 for (i=0; i<m; i++)
           ismember[i] = 0;
	 nsegs = 0;
	 for (i=0; i<vsegs->size; i++)
           for (j=0; j<vsegs->list[i]->size; j++) {
             seg_idx = (int) vsegs->list[i]->values[j];
             ismember[seg_idx] = 1;
             nsegs++;
          }

       n=vsegs->size;
       if ((n>=vsegs->max_size) && (!mw_enlarge_flists(vsegs)))
         mwerror(ERROR,1,"Not enough memory to enlarge Flists!");
       vsegs->size++;
       vsegs->list[n] = mw_change_flist(NULL,nsegs,0,1);
       for (i=0; i<m; i++)
         if (!ismember[i])
            vsegs->list[n]->values[vsegs->list[n]->size++] = i;
     }
     nsegs = vsegs->list[n]->size;
     csegs = vsegs->list[n]->values;
     crv = mw_change_flists(crv,1+nsegs,1+nsegs);
     if (n<vpoints->size) {
     /* plot the boundary of the vanishing region */
     i=nsegs;
     crv->list[i] = mw_change_flist(NULL,5,5,2);
     for (j=0;j<8;j++)
       crv->list[i]->values[j] = vpoints->values[vdim*n+j]+.5;
     for (j=0;j<2;j++)
       crv->list[i]->values[j+8] = vpoints->values[vdim*n+j]+.5;
     /* plot all the segments (or supporting lines) */
     if (lines) {
       /* elongate segments by a factor fact,
	  such that the elongated segment meets the vanishing region */
       for (vp_xbar=0.0, vp_ybar=0.0, j=0; j<8; j+=2) {
	 vp_xbar += vpoints->values[vdim*n+j  ]+.5;
	 vp_ybar += vpoints->values[vdim*n+j+1]+.5;
       }
       vp_xbar *= 0.25; vp_ybar *= 0.25;
       dx = vp_xbar - X0;
       dy = vp_ybar - Y0;
       target_length = sqrt(dx * dx + dy * dy);
       target_length = 2.0*R*(1.0+target_length/R);
     }
     } else crv->size--;
     for (i=0;i<nsegs;i++) {
       crv->list[i] = mw_change_flist(NULL,2,2,2);
       if (lines) {
       /*
	 x0 = allsegs->values[sdim*(int)(csegs[i])+0]+.5;
	 y0 = allsegs->values[sdim*(int)(csegs[i])+1]+.5;
	 x1 = allsegs->values[sdim*(int)(csegs[i])+2]+.5;
	 y1 = allsegs->values[sdim*(int)(csegs[i])+3]+.5;
       */
         x0 = SegListX0(allsegs,(int)(csegs[i]))+.5;
         y0 = SegListY0(allsegs,(int)(csegs[i]))+.5;
	 x1 = SegListX1(allsegs,(int)(csegs[i]))+.5;
	 y1 = SegListY1(allsegs,(int)(csegs[i]))+.5;
	 xbar = (x0+x1)/2.0;
	 ybar = (y0+y1)/2.0;
	 dx = x1 - x0;
	 dy = y1 - y0;
	 fact = target_length / sqrt(dx * dx + dy * dy);
	 crv->list[i]->values[0] = fact*(x0-xbar)+xbar;
	 crv->list[i]->values[1] = fact*(y0-ybar)+ybar;
	 crv->list[i]->values[2] = fact*(x1-xbar)+xbar;
	 crv->list[i]->values[3] = fact*(y1-ybar)+ybar;
       } else {
         for (j=0;j<4;j++)
           crv->list[i]->values[j] = 
	        SegList(allsegs,(int)(csegs[i]))[j]+.5;
       }
     }

}





