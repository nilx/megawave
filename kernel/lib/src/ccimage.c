/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ccimage.c
   
   Vers. 1.3
   (C) 1994-99 Jacques Froment
   Basic memory routines for the ccimage internal type

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>

#include "mw.h"

/* creates a new ccimage structure */

Ccimage mw_new_ccimage()
{
  Ccimage image;

  if(!(image = (Ccimage) (malloc(sizeof(struct ccimage)))))
    {
      mwerror(ERROR, 0, "[mw_new_ccimage] Not enough memory\n");
      return(NULL);
    }

  image->nrow = image->ncol = 0;
  image->allocsize = 0;
  image->firstcol = image->lastcol = image->firstrow = image->lastrow = 0.0;

  image->scale = 1.0;
  strcpy(image->cmt,"?");
  strcpy(image->name,"?");

  image->red = image->green = image->blue = NULL;

  image->previous = NULL;
  image->next = NULL;

  return (image);
}

/* allocates the red,green,blue arrays */ 

Ccimage mw_alloc_ccimage(image,nrow,ncol)

     Ccimage image;
     int nrow, ncol;
{
  int size;
  
  if (image == NULL)
    {
      mwerror(ERROR, 0, 
	      "[mw_alloc_ccimage] cannot alloc plane : ccimage structure is NULL\n");
      return(NULL);
    }

  size =  nrow*ncol*sizeof(unsigned char);
  if (size <= 0)
    {
      mwerror(ERROR, 0,
	      "[mw_alloc_ccimage] Attempts to alloc a ccimage with null size\n");
      return(NULL);
    }

  if ((image->red != NULL) || (image->green != NULL) || (image->blue != NULL))
    {
      mwerror(ERROR, 0,
	      "[mw_alloc_ccimage] Attempts to alloc a ccimage which is already allocated\n");
      return(NULL);
    }

  image->red = (unsigned char *) malloc(size);
  if (image->red == NULL)
    {
      image->nrow = image->ncol = 0;
      image->allocsize = 0;
      mwerror(ERROR, 0,"[mw_alloc_ccimage] Not enough memory\n");
      return(NULL);
    }

  image->green = (unsigned char *) malloc(size);
  if (image->green == NULL)
    {
      free(image->red);
      image->red = NULL;
      image->nrow = image->ncol = 0;
      image->allocsize = 0;
      mwerror(ERROR, 0,"[mw_alloc_ccimage] Not enough memory\n");
      return(NULL);
    }

  image->blue = (unsigned char *) malloc(size);
  if (image->blue == NULL)
    {
      free(image->red);
      image->red = NULL;
      free(image->green);
      image->green = NULL;
      image->nrow = image->ncol = 0;
      image->allocsize = 0;
      mwerror(ERROR, 0,"[mw_alloc_ccimage] Not enough memory\n");
      return(NULL);
    }

  image->nrow=nrow;
  image->ncol=ncol;
  image->allocsize = size;  
  return(image);
}

/* desallocate the array in the ccimage structure and the structure itself */

void mw_delete_ccimage(image)
     Ccimage image;

{
  if (image == NULL)
    {
      mwerror(ERROR, 0,
	      "[mw_delete_ccimage] cannot delete : ccimage structure is NULL\n");
      return;
    }

  if (image->red != NULL) free(image->red);
  image->red = NULL;
  if (image->green != NULL) free(image->green);
  image->green = NULL;
  if (image->blue != NULL) free(image->blue);
  image->blue = NULL;
  free(image);
  image=NULL;
}


/* Change the size of the allocated r,g,b planes */
/* May define the struct if not defined */
/* So you have to call it with image = mw_change_ccimage(image,...) */

Ccimage mw_change_ccimage(image, nrow, ncol)
     Ccimage image;
     int nrow, ncol;
{
  int size;
  
  if (image == NULL) image = mw_new_ccimage();
  if (image == NULL) return(NULL);

  size =  nrow*ncol*sizeof(unsigned char);
  if (size > image->allocsize)
    {
      if (image->red != NULL)
	{
	  free(image->red);
	  image->red = NULL;
	}
      if (image->green != NULL)
	{
	  free(image->green);
	  image->green = NULL;
	}
      if (image->blue != NULL)
	{
	  free(image->blue);
	  image->blue = NULL;
	}
      if (mw_alloc_ccimage(image,nrow,ncol) == NULL)
	{
	  mw_delete_ccimage(image);
	  return(NULL);
	}
    }
  else 
    {
      image->nrow = nrow;
      image->ncol = ncol;
    }
  return(image);
}

/* Return the r,g,b values of a ccimage at location (x,y) */
/* WARNING: this is a slow way to access to a pixel !        */

void mw_getdot_ccimage(image, x, y, r, g, b)

Ccimage image;
int x,y;
unsigned char *r,*g,*b;
{
  int i;

  if (!image) 
    {
      mwerror(ERROR, 0,
	      "[mw_getdot_ccimage] NULL image struct... Return (0,0,0)\n");
      *r=*g=*b=0;
      return;
    }

  if ((x < 0) || (y < 0) || (x >= image->ncol) || (y >= image->nrow))
    {
      mwerror(ERROR, 0,
	      "[mw_getdot_ccimage] Point (%d,%d) out of image... Return (0,0,0)\n",x,y);
      *r=*g=*b=0;
      return;
    }

  i=y*image->ncol+x;

  if (!image->red)
    {
      mwerror(WARNING, 0,
	      "[mw_getdot_ccimage] NULL red plane... Return 0\n");
      *r = 0;
      return;
    } else *r = image->red[i];

  if (!image->green)
    {
      mwerror(WARNING, 0,
	      "[mw_getdot_ccimage] NULL green plane... Return 0\n");
      *g = 0;
      return;
    } else *g = image->green[i];

  if (!image->blue)
    {
      mwerror(WARNING, 0,
	      "[mw_getdot_ccimage] NULL blue plane... Return 0\n");
      *b = 0;
      return;
    } else *b = image->blue[i];
}

/* Set the r,g,b values of a ccimage at location (x,y) */
/* WARNING: this is a slow way to access to a pixel !     */

#ifdef __STDC__
void mw_plot_ccimage(Ccimage image,int x, int y, unsigned char r, 
	unsigned char g, unsigned char b)
#else
void mw_plot_ccimage(image, x, y, r, g, b)

Ccimage image;
int x,y;
unsigned char r,g,b;
#endif

{
  int i;

  if ((!image) || (!image->red) || (!image->green) || (!image->blue)) 
    {
      mwerror(ERROR, 0,
	      "[mw_plot_ccimage] NULL image struct or NULL r,g,b plane\n");
      return;
    }

  if ((x < 0) || (y < 0) || (x >= image->ncol) || (y >= image->nrow))
    {
      mwerror(ERROR, 0,
	      "[mw_plot_ccimage] Point (%d,%d) out of image\n",x,y);
      return;
    }

  i=y*image->ncol+x;
  image->red[i] = r;   image->green[i] = g;   image->blue[i] = b;
}


/* Draw a connex line with gray level c between (a0,b0) and (a1,b1) */

#ifdef __STDC__
void mw_draw_ccimage(Ccimage image,int a0,int b0,int a1,int b1, unsigned char r,
unsigned char g, unsigned char b)
#else
void mw_draw_ccimage(image, a0, b0, a1, b1, r, g, b)

Ccimage image;
int a0,b0,a1,b1;
unsigned char r,g,b;
#endif

{ int i,bdx,bdy;
  int sx,sy,dx,dy,x,y,z;
  
  if ((!image) || (!image->red) || (!image->green) || (!image->blue)) 
    {
      mwerror(ERROR, 0,
	      "[mw_draw_ccimage] NULL image struct or NULL r,g,b plane\n");
      return;
    }

  bdx = image->ncol;
  bdy = image->nrow;

  if (a0<0) a0=0; else if (a0>=bdx) a0=bdx-1;
    if (a1<0) 
      { 
	a1=0; 
	/*
	if (a0 == 0) mwerror(ERROR, 0,
		     "[mw_draw_ccimage] Illegal parameters (%d,%d)-(%d,%d)\n",
			     a0,b0,a1,b1);
	*/
      }
    else 
      if (a1>=bdx) 
	{
	  a1=bdx-1; 
	  /*
	  if (a0==bdx-1) mwerror(ERROR, 0,
		 "[mw_draw_ccimage] Illegal parameters (%d,%d)-(%d,%d)\n",
				 a0,b0,a1,b1);
	 */
	}
    if (b0<0) b0=0; else if (b0>=bdy) b0=bdy-1;
    if (b1<0) 
      { 
	b1=0; 
	/*
	if (b0==0) 
	  mwerror(ERROR, 0,
		  "[mw_draw_ccimage] Illegal parameters (%d,%d)-(%d,%d)\n",
		  a0,b0,a1,b1);
	 */
	}
    else if (b1>=bdy) 
      { b1=bdy-1; 
	/*
	if (b0==bdy-1)
	  mwerror(ERROR, 0,
		  "[mw_draw_ccimage] Illegal parameters (%d,%d)-(%d,%d)\n",
		  a0,b0,a1,b1);
          */
      }

  if (a0<a1) { sx = 1; dx = a1-a0; } else { sx = -1; dx = a0-a1; }
  if (b0<b1) { sy = 1; dy = b1-b0; } else { sy = -1; dy = b0-b1; }
  x=0; y=0;
  
  if (dx>=dy) 
    {
      z = (-dx) / 2;
      while (abs(x) <= dx) 
	{
	  i = (y+b0)*bdx+x+a0;
	  image->red[i] = r;
	  image->green[i] = g;
	  image->blue[i] = b;	  
	  x+=sx;
	  z+=dy;
	  if (z>0) { y+=sy; z-=dx; }
	} 
    }
  else 
    {
      z = (-dy) / 2;
      while (abs(y) <= dy) {
	i = (y+b0)*bdx+x+a0;
	image->red[i] = r;
	image->green[i] = g;
	image->blue[i] = b;	  
	y+=sy;
	z+=dx;
	if (z>0) { x+=sx; z-=dy; }
      }
    }
}

	      
/* Clear the r,g,b planes of a ccimage with the value r,g,b */

#ifdef __STDC__
void mw_clear_ccimage(Ccimage image,unsigned char r,unsigned char g,
	unsigned char b)
#else
void mw_clear_ccimage(image, r,g,b)

Ccimage image;
unsigned char r,g,b;
#endif

{
  int i;

  if ((!image) || (!image->red) || (!image->green) || (!image->blue)) 
    {
      mwerror(ERROR, 0,
	      "[mw_clear_ccimage] NULL image struct or NULL r,g,b plane\n");
      return;
    }
  i=image->ncol*image->nrow;
  memset(image->red,r,i);
  memset(image->green,g,i);
  memset(image->blue,b,i);
}

/* Copy the r,g,b planes of a ccimage into another ccimage */

void mw_copy_ccimage(in, out)

Ccimage in,out;

{ int i;

  if ((!in) || (!out) || (!in->red) || (!in->green) || (!in->blue)
      || (!out->red) || (!out->green) || (!out->blue)
      || (in->ncol != out->ncol) || (in->nrow != out->nrow)) 
    {
      mwerror(ERROR, 0,
	      "[mw_copy_ccimage] NULL input or output image or images of different sizes\n");
      return;
    }

  i = in->ncol*in->nrow;
  memcpy(out->red, in->red,i);
  memcpy(out->green, in->green,i);
  memcpy(out->blue, in->blue,i);
  strcpy(out->cmt,in->cmt);
}

/* Return a tab T so that T[i][j] = image->red[i*image->ncol+j] */

unsigned char ** mw_newtab_red_ccimage(image)

Ccimage image;

{
  unsigned char **im;
  register unsigned long l;

  if ((!image) || (!image->red)) 
    {
      mwerror(ERROR, 0,
	      "[mw_newtab_red_ccimage] NULL image struct or NULL red plane\n");
      return(NULL);
    }
  
  im=(unsigned char **)malloc(image->nrow*sizeof(unsigned char*));
  if (im == NULL)
    {
      mwerror(ERROR, 0, "[mw_newtab_red_ccimage] Not enough memory\n");
      return(NULL);
    }
    
  im[0]=image->red;
  for(l=1;l<image->nrow;l++) im[l]=im[l-1]+image->ncol;

  return(im);
}

/* Return a tab T so that T[i][j] = image->green[i*image->ncol+j] */

unsigned char ** mw_newtab_green_ccimage(image)

Ccimage image;

{
  unsigned char **im;
  register unsigned long l;

  if ((!image) || (!image->green)) 
    {
      mwerror(ERROR, 0,
	      "[mw_newtab_green_ccimage] NULL image struct or NULL green plane\n");
      return(NULL);
    }
  
  im=(unsigned char **)malloc(image->nrow*sizeof(unsigned char*));
  if (im == NULL)
    {
      mwerror(ERROR, 0, "[mw_newtab_green_ccimage] Not enough memory\n");
      return(NULL);
    }
    
  im[0]=image->green;
  for(l=1;l<image->nrow;l++) im[l]=im[l-1]+image->ncol;

  return(im);
}

/* Return a tab T so that T[i][j] = image->blue[i*image->ncol+j] */

unsigned char ** mw_newtab_blue_ccimage(image)

Ccimage image;

{
  unsigned char **im;
  register unsigned long l;

  if ((!image) || (!image->blue)) 
    {
      mwerror(ERROR, 0,
	      "[mw_newtab_blue_ccimage] NULL image struct or NULL blue plane\n");
      return(NULL);
    }
  
  im=(unsigned char **)malloc(image->nrow*sizeof(unsigned char*));
  if (im == NULL)
    {
      mwerror(ERROR, 0, "[mw_newtab_blue_ccimage] Not enough memory\n");
      return(NULL);
    }
    
  im[0]=image->blue;
  for(l=1;l<image->nrow;l++) im[l]=im[l-1]+image->ncol;

  return(im);
}
