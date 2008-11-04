/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {amle3d_init};
 version = {"1.0"};
 author = {"Frederic Cao"};
 function = {"Compute initial data for the level line image interpolation scheme in the 3d case(AMLE)"};
 usage = {
   in->in
         "Input uniformly quantized fmovie",
   delta->delta
         "Width step of the uniform quantization used for the input movie",
   out<-out
         "Output fimage as initial data (input) for the AMLE3d model"
};
*/

#include <stdio.h>
#include "mw.h"


/* Compute the 4 neighbour pixels of the current pixel p              */
/* When p touches the border of the image, a mirror effect is applied */

static void neighbor_4(x,y,xmax,ymax,p,left,right,up,down)
     register int x,y,xmax,ymax;
     register float *p;
     float **left,**right,**up,**down;
{
  if (x>0)
    {
      *left = p-1;
      if (x < xmax)
        {
          *right = p+1;
          if (y>0)
            {
              *up =p-xmax-1;
              if (y < ymax)
                /* 0 < x < xmax  0 < y < ymax */
		*down=p+xmax+1;
              else /* 0 < x < xmax   y = ymax */
		*down=*up;
            }
          else /* 0 < x < xmax   y = 0 */
            {
              *down= p+xmax+1;
              *up=*down;
            }
        }
      else /* x = xmax */
        {
          *right=*left;
          if (y>0)
            {
              *up=p-xmax-1;
              if (y < ymax)
		/* x = xmax  0 < y < ymax */
		*down=p+xmax+1;
              else /* x = xmax   y = ymax */
		*down=*up;
            }
          else /* x = xmax  y = 0 */
            {
              *down=p+xmax+1;
              *up=*down;
            }
        }
    }
  else /* x = 0 */
    {
      *right=p+1;
      *left=*right;
      if (y>0)
        {
          *up=p-xmax-1;
          if (y < ymax)
	    /* x = 0  0 < y < ymax */
	    *down=p+xmax+1;
          else /* x = 0   y = ymax */
	    *down=*up;
        }
      else /* x = 0   y = 0 */
        {
          *down=p+xmax+1;
          *up=*down;
        }
    }
}

void amle3d_init(in,delta,out)
     Fmovie in,out;
     float delta;
{ 
  Fimage im,ima,imout;
  int NC,NL,x,y;
  register float *I,*O;
  float *left,*right,*up,*down;
  float previ,nex;
  float Max;

  if (in==NULL) mwerror(FATAL,1,"Null input fmovie.\n");

  NL=in->first->nrow;
  NC=in->first->ncol;

  out = mw_change_fmovie(out);
  if (out == NULL) mwerror(FATAL,1,"Not enough memory.\n");
  imout=mw_new_fimage();
  im=in->first;
  out->first=imout;

  while (im != NULL)
    {
      I=im->gray;
      imout = mw_change_fimage(imout,NL,NC);
      if (imout == NULL) mwerror(FATAL,1,"Not enough memory.\n");
      mw_clear_fimage(imout,0.0);
      O=imout->gray;

      for (y=0; y<NL; y++)
	for (x=0; x<NC; x++, I++,O++)
	  {
	    neighbor_4(x,y,NC-1,NL-1,I,&left,&right,&up,&down);
	    if 
	      (im->previous != NULL) previ=mw_getdot_fimage(im->previous,x,y);
	    else
	      previ=mw_getdot_fimage(im->next,x,y);
	    if 
	      (im->next != NULL) nex=mw_getdot_fimage(im->next,x,y);
	    else
	      nex=mw_getdot_fimage(im->previous,x,y);
	    /*if im is the first or last image, apply a mirror effect */
	    if ((*I != *left) || (*I != *right) || (*I != *up) || (*I != *down) || (*I != previ) || (*I != nex))
	      {
		Max = *I;
		if (*left > Max) Max=*left;
		if (*right > Max) Max=*right;
		if (*up > Max) Max=*up;
		if (*down > Max) Max=*down;
		if (previ > Max) Max=previ;
		if (nex > Max) Max=nex;
		
		if (*I < Max) *O = *I + delta;
		else *O = *I;
	      }
	  }
      im=im->next;
      ima=mw_new_fimage();
      ima->previous=imout;
      imout->next=ima;
      imout=ima;
    }
  imout->previous->next=NULL;
}

