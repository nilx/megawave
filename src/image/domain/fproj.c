/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fproj};
   version = {"1.01"};
   author = {"Lionel Moisan"};
   function = {"affine or projective mapping using bilinear interpolation"};
   usage = {  
     'x':[sx=512]->sx     "x-size of output image (default: 512)",
     'y':[sy=512]->sy     "y-size of output image (default: 512)",
     'b':[bg=0.0]->bg     "background grey value (default: 0.0)",
     'z'->z               "for zero order interpolation (instead of bilinear)",
     'i'->i               "for inverse transformation",
     in->in               "input Fimage",
     out<-out             "output Fimage",
     X1->X1               "upleft corner",
     Y1->Y1               "upleft corner",
     x2->x2               "upright corner",
     y2->y2               "upright corner",
     x3->x3               "downleft corner",
     y3->y3               "downleft corner",
{
     x4->x4               "downright corner (for projective transformation)",
     y4->y4               "downright corner (for projective transformation)"
}
   };
   */
/*-- MegaWave - Copyright (C) 1994 Jacques Froment. All Rights Reserved. --*/

#include <stdio.h>
#include <math.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

extern double floor();

/* NB : calling this module with out=in is nonsense */

/*----------------------------------------------------------------------*/

void fproj(in,out,sx,sy,bg,z,i,X1,Y1,x2,y2,x3,y3,x4,y4)
Fimage in,out;
int    *sx,*sy;
float  *bg;
char   *z,*i;
float  X1,Y1,x2,y2,x3,y3,*x4,*y4;
{
  int    nx,ny,x,y,xi,yi,adr,tx1,ty1,tx2,ty2;
  float  xx,yy,x12,y12,x13,y13,xp,yp,a11,a12,a21,a22,ux,uy,a,b,d;
  
  /* ALLOCATE NEW IMAGE */
  nx = in->ncol;
  ny = in->nrow;
  out = mw_change_fimage(out,*sy,*sx);
  if (!out) mwerror(FATAL,1,"not enough memory\n");
  
  /* COMPUTE NEW BASIS */
  if (i) {
    x12 = (x2-X1)/(float)nx;
    y12 = (y2-Y1)/(float)nx;
    x13 = (x3-X1)/(float)ny;
    y13 = (y3-Y1)/(float)ny;
  } else {
    x12 = (x2-X1)/(float)(*sx);
    y12 = (y2-Y1)/(float)(*sx);
    x13 = (x3-X1)/(float)(*sy);
    y13 = (y3-Y1)/(float)(*sy);
  }
  if (y4) {
    xx=((*x4-X1)*(y3-Y1)-(*y4-Y1)*(x3-X1))/((x2-X1)*(y3-Y1)-(y2-Y1)*(x3-X1));
    yy=((*x4-X1)*(y2-Y1)-(*y4-Y1)*(x2-X1))/((x3-X1)*(y2-Y1)-(y3-Y1)*(x2-X1));
    a = (yy-1.0)/(1.0-xx-yy);
    b = (xx-1.0)/(1.0-xx-yy);
  } else a=b=0.0;

  /*** half-pixel translation for zero order interpolation */
  if (z) {
    X1 += 0.5;
    Y1 += 0.5;
  }

  /********** MAIN LOOP **********/

  for (x=0;x<*sx;x++) 
    for (y=0;y<*sy;y++) {
      
      /* COMPUTE LOCATION IN INPUT IMAGE */
      if (i) {
	xx = (((float)x-X1)*y13-((float)y-Y1)*x13)/(x12*y13-y12*x13);
	yy = -(((float)x-X1)*y12-((float)y-Y1)*x12)/(x12*y13-y12*x13);
	d = 1.0-(a/(a+1.0))*xx/(float)nx-(b/(b+1.0))*yy/(float)ny;
	xp = xx/((a+1.0)*d);
	yp = yy/((b+1.0)*d);

	/*d = a+b+1.0-b*(a+1.0)*yy/(float)ny+a*b*(1.0-xx/(float)nx);
	xp = xx*(b+1.0)/d;
	yp = yy*(a+1.0)/d;*/
      } else {
	/*printf("x=%d y=%d ->",x,y);*/
	d = a*(float)x/(float)(*sx)+b*(float)y/(float)(*sy)+1.0;
	xx = (a+1.0)*(float)(x)/d;
	yy = (b+1.0)*(float)(y)/d;
	xp = X1 + xx*x12 + yy*x13;
	yp = Y1 + xx*y12 + yy*y13;
      }

      /* INTERPOLATION */
      xi = (int)floor((double)xp); 
      yi = (int)floor((double)yp);
      adr = yi*nx+xi;
      tx1 = (xi>=0 && xi<nx);
      ty1 = (yi>=0 && yi<ny);
      a11 = (tx1 && ty1? in->gray[adr]:*bg);
      if (!z) {
	tx2 = (xi+1>=0 && xi+1<nx);
	ty2 = (yi+1>=0 && yi+1<ny);
	a12 = (tx1 && ty2? in->gray[adr+nx]:*bg);
	a21 = (tx2 && ty1? in->gray[adr+1]:*bg);
	a22 = (tx2 && ty2? in->gray[adr+nx+1]:*bg);
	ux = xp-(float)xi;
	uy = yp-(float)yi;
      }

      /* PUT RESULT IN NEW IMAGE */
      out->gray[y*(*sx)+x]=
	(z?a11:(1.0-uy)*((1.0-ux)*a11+ux*a21)+uy*((1.0-ux)*a12+ux*a22));
    }
}

