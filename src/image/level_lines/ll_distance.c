/*--------------------------------------------------------------------------*/
/* mwcommand
name = {ll_distance};
version={"1.1"};
author={"Frederic Guichard, Lionel Moisan"};
function={"Compute signed distance image to a level line of an image"};
usage = {
'l':[level=128.0]->level    
    "gray level (default 128.0)",
'm':MaxDist->MaxDist
    "maximum distance to look for (integer, default=image size)",
'n':nearest<-nearest
    "image of nearest levels (to get Voronoï Diagram)",
in -> in 
    "input Fimage",
out <- out 
    "output Fimage"
};
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))


void ll_distance(in, out, level, MaxDist, nearest)
Fimage in,out,nearest;
float *level;
int *MaxDist;
{
  int i,j,X1,Y1,X2,Y2,kx,ky,dx,dy,pos,ok,maxdist;
  float d,cur,min,max;
  
  dy= in->nrow;
  dx= in->ncol;

  if (nearest) {
    mw_change_fimage(nearest,dy,dx);
    if (nearest==NULL) mwerror(FATAL,1,"ll_distance: Not enough memory.\n");
    mw_clear_fimage(nearest,*level);
  }
    
  out=mw_change_fimage(out,dy,dx);
  if (out==NULL) mwerror(FATAL,1,"ll_distance: Not enough memory.\n");

  if (MaxDist) maxdist = *MaxDist;
  else maxdist = MAX(dx,dy);
  mw_clear_fimage(out,(float)maxdist);
  
  /* PRIMARY LOOP : TEST ALL POINTS */

  for(j=0;j<dy;j++) {
    
    Y1=MAX(j-maxdist,0);
    Y2=MIN(j+maxdist+1,dy);
    
    for(i=0;i<dx;i++) { 
      
      pos= i+j*dx;      
      cur = in->gray[pos];
      if (cur<=*level) {

	/* compute maximum value in 4-neighborhood */
	if (i>0) max=in->gray[pos-1]; else max=cur;
	if (i<dx-1) if (in->gray[pos+1]>max) max=in->gray[pos+1];
	if (j>0) if (in->gray[pos-dx]>max) max=in->gray[pos-dx];
	if (j<dy-1) if (in->gray[pos+dx]>max) max=in->gray[pos+dx];
	ok = (max>cur);

	if (!ok && cur==*level) {
	  /* compute minimum value in 4-neighborhood */
	  if (i>0) min=in->gray[pos-1]; else min=cur;
	  if (i<dx-1) if (in->gray[pos+1]<min) min=in->gray[pos+1];
	  if (j>0) if (in->gray[pos-dx]<min) min=in->gray[pos-dx];
	  if (j<dy-1) if (in->gray[pos+dx]<min) min=in->gray[pos+dx];
	  ok = (min<cur);
	}

	if (ok) { /* that is, if (i,j) belongs to the level line */

	  X1 = MAX(i-maxdist,0);
	  X2 = MIN(i+maxdist+1,dx);	  

	  /* SECONDARY LOOP: TEST NEIGHBORHOOD */

	  for(ky=Y1;ky<Y2;ky++)
	    for(kx=X1;kx<X2;kx++) {
	      d= (float)sqrt((double)((kx-i)*(kx-i)+(ky-j)*(ky-j)));
	      if (d < out->gray[kx+ky*dx]) {
		out->gray[kx+ky*dx] = d;
		if (nearest) nearest->gray[kx+ky*dx] = cur;
	      }
	    }
	}
      }
    }
  }
  
  /* change sign where needed to get signed distance */
  for(i=dx*dy;i--;)
    if (in->gray[i]>=*level)
      out->gray[i] = -out->gray[i];
}

  









