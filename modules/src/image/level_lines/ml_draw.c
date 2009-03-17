/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {ml_draw};
 version = {"8.1"};
 author = {"Jacques Froment, Georges Koepfler"};
 function = {"Draw morpho_lines of mimage"};
 usage = {
   'b'->border         "draw border",
  'a':bimage->bimage   "add the original bitmap image to the background",
  'o':movie<-movie     "movie made with one b/w image per level",
  mimage->m_image      "input m_image",
  image_out<-ml_draw   "b/w image of morpho_lines, size (2L-1)x(2C-1)"
};
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "mw.h"
#include "mw-modules.h"

#define POINT_OK(P,Y,X)  (((P)->x>=0)&&((P)->x<=X)&&((P)->y>=0)&&((P)->y<=Y))
#define BAD_POINT(P,Y,X) (!POINT_OK(P,Y,X))

static void draw_mlines(Morpho_line mline, unsigned char **im, int NL, int NC, char *border)
{
  Point_curve point_ptr;
  int BNL,BNC,l,c,dl,dc;

  BNL=2*NL-1;
  BNC=2*NC-1;

  point_ptr=mline->first_point;
  if(BAD_POINT(point_ptr,NL,NC))
    mwerror(FATAL,1,"Point out of image.");
  if (border)
    {
      l=2*point_ptr->y;
      c=2*point_ptr->x;
      im[l][c]=0;      
    }
  else
    {
      l=2*point_ptr->y-1;
      c=2*point_ptr->x-1;
      if (!((l==-1)||(l==BNL)||(c==-1)||(c==BNC))) im[l][c]=0;      
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
      im[l][c]=0;
      l+=dl; c+=dc;
      if (border || !((l==-1)||(l==BNL)||(c==-1)||(c==BNC)))
	im[l][c]=0;
    }
  if(mline->open==0) 
    {                /* closed level line */
      dl=mline->first_point->y-point_ptr->y;
      dc=mline->first_point->x-point_ptr->x;
      if(!(((abs(dl)==1)&&(dc==0))||((dl==0)&&(abs(dc)==1))))
	mwerror(FATAL,1,"Points are not 4-adjacent.");
      l+=dl; c+=dc;
      im[l][c]=0;
    }
}

/* Set the original bitmap image bimage in the background of the morpho-line bitmap 
   mlimage
*/

static void  setimage(char *border, Cimage bimage, Cimage mlimage)
{
  int NCb,NLb,NCml,NLml;
  int xb,yb,xml,yml,x0,x1,y0,y1;
  unsigned char c;

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
	c = mw_getdot_cimage(bimage,xb,yb);
	mw_plot_cimage(mlimage,xml,yml,c);
	if (xml+1<x1) mw_plot_cimage(mlimage,xml+1,yml,c);
	if (yml+1<y1) mw_plot_cimage(mlimage,xml,yml+1,c);
	if ((xml+1<x1)&&(yml+1<y1)) mw_plot_cimage(mlimage,xml+1,yml+1,c);
      }
}

Cimage ml_draw(Mimage m_image, Cimage bimage, char *border, Cmovie movie)
{ 
  Cimage cb=NULL,newcb,oldcb;
  Morpho_line mline_list=NULL,mline;
  int NC,NCO,NL,NLO,nm;
  long l;
  unsigned char **im;

  if(m_image==NULL)
    return(cb);

  NL=m_image->nrow;
  NC=m_image->ncol;
  if((NC<1)||(NL<1))
    mwerror(FATAL,1,"Bad dimensions in mimage");

  if(m_image->first_ml==NULL)
    mwerror(FATAL,1,"No morpho_lines in mimage.");
  else
    mline_list=m_image->first_ml;

  if (border)
    { NLO=2*NL+1; NCO=2*NC+1; }
  else
    { NLO=2*NL-1; NCO=2*NC-1; }

  /* get easy access to gray plane */
  im=(unsigned char **)malloc(NLO*sizeof(unsigned char*));
  if(im==NULL) mwerror(FATAL,1,"Not enough memory.");

  if (movie)
    {  /* draw morpho lines level per level */
      movie->first = NULL;
      oldcb = NULL;
      nm = 0;
      for(mline=mline_list;mline!=NULL;) 
	{
	  /* new image */
	  newcb=mw_change_cimage(NULL,NLO,NCO);
	  if(newcb==NULL) mwerror(FATAL,1,"Not enough memory.");
	  mw_clear_cimage(newcb,255);
	  if (bimage != NULL) setimage(border,bimage,newcb);
	  im[0]=newcb->gray;
	  for(l=1;l<newcb->nrow;l++) im[l]=im[l-1]+newcb->ncol;
	  sprintf(newcb->cmt,"Morpho lines of '%s' for %g <= GL <= %g",
		  m_image->name,mline->minvalue,mline->maxvalue);
	  nm++;
	  mwdebug("image #%d\t Morpho lines for %g <= GL <= %g\n",
		  nm,mline->minvalue,mline->maxvalue);
	  if (movie->first == NULL) movie->first=newcb;
	  else { oldcb->next = newcb; newcb->previous = oldcb; }
	  oldcb = newcb;

	  do
	    {
	      draw_mlines(mline,im,NL,NC,border);
	      mline=mline->next;
	    }
	  while ((mline != NULL)&&(mline->previous->minvalue == mline->minvalue)
		 &&(mline->previous->maxvalue == mline->maxvalue));
	}
    }

  /* Output cimage : draw all the morpho lines */
  cb=mw_change_cimage(NULL,NLO,NCO);
  if(cb==NULL) mwerror(FATAL,1,"Not enough memory.");
  mw_clear_cimage(cb,255);
  if (bimage != NULL) setimage(border,bimage,cb);

  im[0]=cb->gray;
  for(l=1;l<cb->nrow;l++) im[l]=im[l-1]+cb->ncol;
      
  for(mline=mline_list;mline!=NULL;mline=mline->next) 
    draw_mlines(mline,im,NL,NC,border);
  
  free((void*)im);
  return(cb);
}








