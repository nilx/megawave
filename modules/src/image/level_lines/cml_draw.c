/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {cml_draw};
 version = {"1.0"};
 author = {"Jacques Froment, Georges Koepfler"};
 function = {"Draw cmorpho_lines of cmimage"};
 usage = {
   'b'->border           "draw border",
   'a':bimage->bimage    "add the original bitmap ccimage to the background",
   'o':movie<-movie      "color movie made with one b/w image per level",
   cmimage->cmimage      "input cmimage",
   image_out<-cml_draw   "color image of cmorpho_lines, size (2L-1)x(2C-1)"
};
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "mw.h"

#define POINT_OK(P,Y,X)  (((P)->x>=0)&&((P)->x<=X)&&((P)->y>=0)&&((P)->y<=Y))
#define BAD_POINT(P,Y,X) (!POINT_OK(P,Y,X))

static void draw_cmlines(Cmorpho_line mline, Ccimage image, int NL, int NC, char *border)
{
  Point_curve point_ptr;
  int BNL,BNC,l,c,dl,dc;
  unsigned long p;

  BNL=2*NL-1;
  BNC=2*NC-1;

  point_ptr=mline->first_point;
  if(BAD_POINT(point_ptr,NL,NC))
    mwerror(FATAL,1,"Point out of image.");
  if (border)
    {
      l=2*point_ptr->y;
      c=2*point_ptr->x;
      p = l * image->ncol + c;
      image->red[p]=image->green[p]=image->blue[p]=0;      
    }
  else
    {
      l=2*point_ptr->y-1;
      c=2*point_ptr->x-1;
      p = l * image->ncol + c;
      if (!((l==-1)||(l==BNL)||(c==-1)||(c==BNC)))
	image->red[p]=image->green[p]=image->blue[p]=0;      
    }

  while(point_ptr->next!=NULL) 
    {
      point_ptr=point_ptr->next;
      if(BAD_POINT(point_ptr,NL,NC))
	mwerror(FATAL,1,"Point out of image.");
      dl=point_ptr->y-point_ptr->previous->y;
      dc=point_ptr->x-point_ptr->previous->x;
      if(!(((abs(dl)==1)&&(dc==0))||((dl==0)&&(abs(dc)==1))))
	mwerror(FATAL,1,"Points are not 4-adjacent.");
      l+=dl; c+=dc;
      p = l * image->ncol + c;
      image->red[p]=image->green[p]=image->blue[p]=0;      
      l+=dl; c+=dc;
      p = l * image->ncol + c;
      if (border || !((l==-1)||(l==BNL)||(c==-1)||(c==BNC)))
	image->red[p]=image->green[p]=image->blue[p]=0;      
    }
  if(mline->open==0) 
    {                /* closed level line */
      dl=mline->first_point->y-point_ptr->y;
      dc=mline->first_point->x-point_ptr->x;
      if(!(((abs(dl)==1)&&(dc==0))||((dl==0)&&(abs(dc)==1))))
	mwerror(FATAL,1,"Points are not 4-adjacent.");
      l+=dl; c+=dc;
      p = l * image->ncol + c;
      image->red[p]=image->green[p]=image->blue[p]=0;      
    }
}

/* Set the original bitmap image bimage in the background of the cmorpho-line bitmap 
   mlimage
*/

static void  setimage(char *border, Ccimage bimage, Ccimage mlimage)
{
  int NCb,NLb,NCml,NLml;
  int xb,yb,xml,yml,x0,x1,y0,y1;
  unsigned char r,g,b;

  NLb = bimage->nrow;
  NCb = bimage->ncol;
  NLml = mlimage->nrow;
  NCml = mlimage->ncol;

  if ( ((border)&&((NLml != 2*NLb+1) || (NCml != 2*NCb+1)))
       ||
       ((!border)&&((NLml != 2*NLb-1) || (NCml != 2*NCb-1)))
       )
    mwerror(FATAL,1,"Bad size for input bitmap image... Are you sure these is the original one ?\n");

  if (border)
    {
      x0=1; x1=NCml-1; y0=1; y1=NLml-1;
    }
  else
    {
      x0=0; x1=NCml; y0=0; y1=NLml;      
    }

  for (xml=x0, xb=0; xml<x1; xml+=2, xb++)
    for (yml=y0, yb=0; yml<y1; yml+=2, yb++)
      {
	mw_getdot_ccimage(bimage,xb,yb,&r,&g,&b);
	mw_plot_ccimage(mlimage,xml,yml,r,g,b);
	if (xml+1<x1) mw_plot_ccimage(mlimage,xml+1,yml,r,g,b);
	if (yml+1<y1) mw_plot_ccimage(mlimage,xml,yml+1,r,g,b);
	if ((xml+1<x1)&&(yml+1<y1)) mw_plot_ccimage(mlimage,xml+1,yml+1,r,g,b);
      }
}

Ccimage cml_draw(Cmimage cmimage, Ccimage bimage, char *border, Ccmovie movie)
{ 
  Ccimage cb=NULL,newcb,oldcb;
  Cmorpho_line mline_list,mline;
  int NC,NCO,NL,NLO,nm;

  if(cmimage==NULL)
    return(cb);

  NL=cmimage->nrow;
  NC=cmimage->ncol;
  if((NC<1)||(NL<1))
    mwerror(FATAL,1,"Bad dimensions in cmimage");

  if(cmimage->first_ml==NULL)
    mwerror(FATAL,1,"No cmorpho_lines in cmimage.");
  else
    mline_list=cmimage->first_ml;

  if (border)
    { NLO=2*NL+1; NCO=2*NC+1; }
  else
    { NLO=2*NL-1; NCO=2*NC-1; }

  if (movie)
    {  /* draw cmorpho lines level per level */
      movie->first = NULL;
      oldcb = NULL;
      nm = 0;
      for(mline=mline_list;mline!=NULL;) 
	{
	  /* new image */
	  newcb=mw_change_ccimage(NULL,NLO,NCO);
	  if(newcb==NULL) mwerror(FATAL,1,"Not enough memory.");
	  mw_clear_ccimage(newcb,255,255,255);
	  if (bimage != NULL) setimage(border,bimage,newcb);
	  sprintf(newcb->cmt,
		  "cmorpho lines of '%s' for "
		  "(%g, %g, %g) <= GL <= (%g, %g, %g)",
		  cmimage->name,
		  mline->minvalue.red, mline->minvalue.green, 
		  mline->minvalue.blue,
		  mline->maxvalue.red, mline->maxvalue.green, 
		  mline->maxvalue.blue);
	  nm++;
	  mwdebug("image #%d\t cmorpho lines for "
		  "(%g, %g, %g) <= GL <= (%g, %g, %g)\n",
		  nm,
		  mline->minvalue.red, mline->minvalue.green, 
		  mline->minvalue.blue,
		  mline->maxvalue.red, mline->maxvalue.green, 
		  mline->maxvalue.blue);
	  if (movie->first == NULL) movie->first=newcb;
	  else { oldcb->next = newcb; newcb->previous = oldcb; }
	  oldcb = newcb;

	  do
	    {
	      draw_cmlines(mline,newcb,NL,NC,border);
	      mline=mline->next;
	    }
	  while ((mline != NULL)
		 &&
		 (mline->previous->minvalue.red == mline->minvalue.red)
		 &&
		 (mline->previous->minvalue.green == mline->minvalue.green)
		 &&
		 (mline->previous->minvalue.blue == mline->minvalue.blue)
		 &&
		 (mline->previous->maxvalue.red == mline->maxvalue.red)
		 &&
		 (mline->previous->maxvalue.green == mline->maxvalue.green)
		 &&
		 (mline->previous->maxvalue.blue == mline->maxvalue.blue));
	}
    }

  /* Output cimage : draw all the cmorpho lines */
  cb=mw_change_ccimage(NULL,NLO,NCO);
  if(cb==NULL) mwerror(FATAL,1,"Not enough memory.");
  mw_clear_ccimage(cb,255,255,255);
  if (bimage != NULL) setimage(border,bimage,cb);

  for(mline=mline_list;mline!=NULL;mline=mline->next) 
    draw_cmlines(mline,cb,NL,NC,border);

  return(cb);
}








