/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {mschannel};
 version = {"1.5"};
 author = {"Yann Guyonvarc'h"}; 
 function = {"Build a multi-scales multi-channels decomposition of an image"};
 usage = {
   'N':[N=1]->N      "# images per channel(for local scale value)",
   'S':[S=1]->S      "standard deviation of the smoothing filter",
   'W':[W=1]->W      "pixel weight for the smoothing filter",
   'p':[p=2]->p[1,2] "scalar distance: ABS (p=1) or Quadratic (p=2)",
   in->in            "input Fimage",
   mov<-mov          "output Fmovie"
};
*/
/*----------------------------------------------------------------------
 v1.2: call to fblur() -> fsepconvol() (L.Moisan)
 v1.3: main loop rewritten, bugs fixed, acceleration (L.Moisan)
 v1.4: mwcommand syntax fixed (JF)
 v1.5 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for fsmooth() */

#define ABS(x)       ( (x)>0?(x):-(x) )


static Fimage add_image(int nx, int ny, Fimage *prev, Fimage **next)
{
  Fimage u;
  
  u = mw_change_fimage(NULL,ny,nx);
  if (!u) mwerror(FATAL,1,"Not enough memory\n");
  u->previous = *prev;
  **next = *prev = u;
  *next = &(u->next);

  return(u);
}

/*------------------------------ MAIN MODULE ------------------------------*/

void mschannel(int *N, int *S, int *W, int *p, Fimage in, Fmovie mov)
{
  int      x,y,xl,xr,yl,yr,e,nx,ny,boundary=1;  
  Fimage   H,V,D,tmp,prev,*next;
  float    a,b,c;

  nx = in->ncol; ny = in->nrow;
  
  mov = mw_change_fmovie(mov);
  tmp = mw_change_fimage(in,ny,nx);
  if (!mov || !tmp) mwerror(FATAL,1,"Not enough memory\n");
  
  next = &(mov->first);
  prev = NULL;

  *N = abs(2*(*N)-1);
  mw_copy_fimage(in,tmp);
  
  for (e=1;e<=*N;e+=2) {

    H = add_image(nx,ny,&prev,&next);
    V = add_image(nx,ny,&prev,&next);
    D = add_image(nx,ny,&prev,&next);

    if (e!=1)
      fsepconvol(tmp,tmp,NULL,NULL,&e,NULL,&boundary);
      
    for (x=0;x<nx;x++)
      for (y=0;y<ny;y++) {

	a = tmp->gray[y*nx+x];
	xl = (x>=e?x-e:0); xr = (x+e<nx?x+e:nx-1);
	yl = (y>=e?y-e:0); yr = (y+e<ny?y+e:ny-1);

	/* Horizontal channel */
	b = tmp->gray[y*nx+xl] - a;
	c = tmp->gray[y*nx+xr] - a;
	H->gray[y*nx+x] = 0.5*(*p==1?ABS(b)+ABS(c):b*b+c*c);

	/* Vertical channel */
	b = tmp->gray[yl*nx+x] - a;
	c = tmp->gray[yr*nx+x] - a;
	V->gray[y*nx+x] = 0.5*(*p==1?ABS(b)+ABS(c):b*b+c*c);

	/* Diagonal channel */
	b = tmp->gray[yl*nx+xl] - a;
	c = tmp->gray[yr*nx+xr] - a;
	D->gray[y*nx+x] = 0.5*(*p==1?ABS(b)+ABS(c):b*b+c*c);

      }
    
    fsmooth(S,W,H,H);
    fsmooth(S,W,V,V);
    fsmooth(S,W,D,D);
  }
  
  *next=NULL;
}

