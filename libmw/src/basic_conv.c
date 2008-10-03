/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  basic_conv.c
   
  Vers. 1.20
  Author : Jacques Froment
  Basic routines to convert internal MegaWave2 types (C structures)

  Warning : since this source file is read by make_type_conv to detect the
  existing functions, do not use extravagant syntax. In particular,
  do not set function declaration between comments.


  Main changes :
  1.19 (LM): include data fields in mw_*list*_to_*list*()
  1.20 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h>
#include <string.h>

#include "libmw-defs.h"
#include "utils.h"
#include "cimage.h"
#include "fimage.h"
#include "ccimage.h"
#include "cfimage.h"
#include "polygon.h"
#include "fpolygon.h"
#include "curve.h"
#include "fcurve.h"
#include "dcurve.h"
#include "mimage.h"
#include "cmimage.h"
#include "list.h"

#include "basic_conv.h"

int _mw_convert_struct_warning = 0;

/*========================= IMAGES ================================*/

/* float to uchar - Internal use - */

void _mw_float_to_uchar(register float *ptr_float, 
			register unsigned char *ptr_uchar, 
			int N, char *data_name)
{
     register int l;
     register int c;
     float f;
     int out,lost;

     lost=out=0;
     for (l=1; l <= N; l++,ptr_float++,ptr_uchar++)
     {
	  f=*ptr_float;
	  if (f >= 255.5) {c=255; out++;}
	  else if (f <= -0.5) {c=0; out++;}
	  else 
	  {
	       c= (unsigned char) floor(f + .5);
	       if (c != f) lost=1;
	  }
	  *ptr_uchar=c;
     }

     if ((out > 0)&&(_mw_convert_struct_warning >= 0))
     {
	  mwerror(WARNING,0,"%d %s levels were out of [0,255].\n",out,data_name);
	  _mw_convert_struct_warning++;
     }
     if ((lost == 1)&&(_mw_convert_struct_warning >= 0))
     {
	  mwerror(WARNING,0,"%s information has been lost due to quantization.\n",
		  data_name);
	  _mw_convert_struct_warning++;
     }
}

/* uchar to float - Internal use - */

void _mw_uchar_to_float(register unsigned char *ptr_uchar, 
			register float *ptr_float, int N)
{
     register int l;

     for (l=1; l <= N; l++, *(ptr_float++) = (float) *(ptr_uchar++));
}


/* 1 array of 24 bits -> 3 arrays of 8 bits - Internal use - */
/* The array of 24 bits has to be filled like the XV pic24 structure:
   i.e. Offset:   1    2    3    4    5    6    ...
   Pixel:    r    g    b    r    g    b    ...
*/
void _mw_1x24XV_to_3x8_ucharplanes(register unsigned char *ptr, 
				   register unsigned char *ptr1, 
				   register unsigned char *ptr2, 
				   register unsigned char *ptr3, int N)
{
     register int l;

     if ((N % 3) != 0) 
	  mwerror(INTERNAL,0,"[_mw_1x24XV_to_3x8_ucharplanes] Not a 24 bits plane\n");
     for (l=1;l<=N;l+=3)
     {
	  *ptr1++ = *ptr++;
	  *ptr2++ = *ptr++;
	  *ptr3++ = *ptr++;
     }
}

/* 3 arrays of 8 bits -> 1 array of 24 bits - Internal use - */
/* The array of 24 bits is filled like the XV pic24 structure:
   i.e. Offset:   1    2    3    4    5    6    ...
   Pixel:    r    g    b    r    g    b    ...
*/
void _mw_3x8_to_1x24XV_ucharplanes(register unsigned char *ptr1, 
				   register unsigned char *ptr2, 
				   register unsigned char *ptr3, 
				   register unsigned char *ptr, int N)
{
     register int l;

     if ((N % 3) != 0) 
	  mwerror(INTERNAL,0,"[_mw_3x8_to_1x24XV_ucharplanes] Not a 24 bits plane\n");
     for (l=1;l<=N;l+=3)
     {
	  *ptr++ = *ptr1++;
	  *ptr++ = *ptr2++;
	  *ptr++ = *ptr3++;
     }
}

/*--- Fimage to Cimage ---*/

Cimage mw_fimage_to_cimage(Fimage image_fimage, Cimage image)
{
     image = mw_change_cimage(image,image_fimage->nrow,image_fimage->ncol);
     if (image == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Cimage\n");
	  return(NULL);
     }

     _mw_float_to_uchar(image_fimage->gray,image->gray,image->nrow*image->ncol,
			"Gray");

     image->scale = image_fimage->scale;

     strcpy(image->cmt,image_fimage->cmt);
     strcpy(image->name,image_fimage->name);

     image->firstcol = image_fimage->firstcol;
     image->lastcol = image_fimage->lastcol;
     image->firstrow = image_fimage->firstrow;
     image->lastrow = image_fimage->lastrow;

     return (image);
}

/*--- Cimage to Fimage ---*/

Fimage mw_cimage_to_fimage(Cimage image_cimage, Fimage image)
{   
     image = mw_change_fimage(image,image_cimage->nrow,image_cimage->ncol);
     if (image == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fimage !\n");
	  return(NULL);
     }
  
     _mw_uchar_to_float(image_cimage->gray,image->gray,
			image_cimage->ncol*image_cimage->nrow);

     image->scale = image_cimage->scale;

     strcpy(image->cmt,image_cimage->cmt);
     strcpy(image->name,image_cimage->name);

     image->firstcol = image_cimage->firstcol;
     image->lastcol = image_cimage->lastcol;
     image->firstrow = image_cimage->firstrow;
     image->lastrow = image_cimage->lastrow;

     return (image);
}


/*--- Cfimage to Ccimage ---*/

Ccimage mw_cfimage_to_ccimage(Cfimage image_cfimage, Ccimage image)
{
     if (image_cfimage->model != MODEL_RGB) 
     {
	  mwerror(ERROR, 0,"Colors of input Cfimage don't follow the RGB model\n");
	  return(NULL);      
     }
     image = mw_change_ccimage(image,image_cfimage->nrow,image_cfimage->ncol);
     if (image == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Ccimage\n");
	  return(NULL);
     }

     _mw_float_to_uchar(image_cfimage->red,image->red,image->nrow*image->ncol,
			"Red");
     _mw_float_to_uchar(image_cfimage->green,image->green,image->nrow*image->ncol,
			"Green");
     _mw_float_to_uchar(image_cfimage->blue,image->blue,image->nrow*image->ncol,
			"Blue");

     image->scale = image_cfimage->scale;

     strcpy(image->cmt,image_cfimage->cmt);
     strcpy(image->name,image_cfimage->name);

     image->firstcol = image_cfimage->firstcol;
     image->lastcol = image_cfimage->lastcol;
     image->firstrow = image_cfimage->firstrow;
     image->lastrow = image_cfimage->lastrow;

     return (image);
}

/*--- Ccimage to Cfimage ---*/

Cfimage mw_ccimage_to_cfimage(Ccimage image_ccimage, Cfimage image)
{ 
     image = mw_change_cfimage(image,image_ccimage->nrow,image_ccimage->ncol);
     if (image == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Cfimage !\n");
	  return(NULL);
     }
  
     _mw_uchar_to_float(image_ccimage->red,image->red,
			image_ccimage->ncol*image_ccimage->nrow);

     _mw_uchar_to_float(image_ccimage->green,image->green,
			image_ccimage->ncol*image_ccimage->nrow);

     _mw_uchar_to_float(image_ccimage->blue,image->blue,
			image_ccimage->ncol*image_ccimage->nrow);

     image->scale = image_ccimage->scale;

     strcpy(image->cmt,image_ccimage->cmt);
     strcpy(image->name,image_ccimage->name);

     image->firstcol = image_ccimage->firstcol;
     image->lastcol = image_ccimage->lastcol;
     image->firstrow = image_ccimage->firstrow;
     image->lastrow = image_ccimage->lastrow;

     return (image);
}

/*--- Cfimage to Fimage ---*/

/* MONO, float case */
#define FMONO(r,g,b) ((r)*.33 + (g)*.5 + (b)*.17)

Fimage mw_cfimage_to_fimage(Cfimage image_cfimage, Fimage image)
{
     register float *gr,*r,*g,*b;
     register int i;
     int clost;

     image = mw_change_fimage(image,image_cfimage->nrow,image_cfimage->ncol);
     if (image == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fimage\n");
	  return(NULL);
     }

     clost = 0;
     for (i=1, gr=image->gray, r=image_cfimage->red, g=image_cfimage->green,
	       b=image_cfimage->blue;
	  i <= image->nrow * image->ncol;
	  i++, gr++, r++, g++, b++)
     {
	  *gr = FMONO(*r,*g,*b);
	  if ((*r != *g) || (*g != *b)) clost++;
     }
     if ((clost != 0)&&(_mw_convert_struct_warning >= 0))
     {
	  mwerror(WARNING,0,"Color information has been lost !\n");
	  _mw_convert_struct_warning++;
     }      

     image->scale = image_cfimage->scale;

     strcpy(image->cmt,image_cfimage->cmt);
     strcpy(image->name,image_cfimage->name);

     image->firstcol = image_cfimage->firstcol;
     image->lastcol = image_cfimage->lastcol;
     image->firstrow = image_cfimage->firstrow;
     image->lastrow = image_cfimage->lastrow;

     return (image);
}

/*--- Fimage to Cfimage ---*/

Cfimage mw_fimage_to_cfimage(Fimage image_fimage, Cfimage image)
{
     register float *gr,*r,*g,*b;
     register int i;

     image = mw_change_cfimage(image,image_fimage->nrow,image_fimage->ncol);
     if (image == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Cfimage !\n");
	  return(NULL);
     }

     for (i=1, gr=image_fimage->gray, r=image->red, g=image->green,b=image->blue;
	  i <= image->nrow * image->ncol;
	  i++, gr++, r++, g++, b++)
	  *r = *g = *b = *gr;

     image->scale = image_fimage->scale;

     strcpy(image->cmt,image_fimage->cmt);
     strcpy(image->name,image_fimage->name);

     image->firstcol = image_fimage->firstcol;
     image->lastcol = image_fimage->lastcol;
     image->firstrow = image_fimage->firstrow;
     image->lastrow = image_fimage->lastrow;

     return (image);
}


/*--- Ccimage to Fimage  ---*/

/* Useful despite ccimage -> cimage -> fimage chain to keep decimal in
   the conversion.
*/

Fimage mw_ccimage_to_fimage(Ccimage image_ccimage, Fimage image)
{
     register unsigned char *r,*g,*b;
     register float *gr;
     register int i;
     int clost;

     image = mw_change_fimage(image,image_ccimage->nrow,image_ccimage->ncol);
     if (image == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fimage\n");
	  return(NULL);
     }

     clost = 0;
     for (i=1, gr=image->gray, r=image_ccimage->red, g=image_ccimage->green,
	       b=image_ccimage->blue;
	  i <= image->nrow * image->ncol;
	  i++, gr++, r++, g++, b++)
     {
	  *gr = FMONO(*r,*g,*b);
	  if ((*r != *g) || (*g != *b)) clost++;
     }
     if ((clost != 0)&&(_mw_convert_struct_warning >= 0))
     {
	  mwerror(WARNING,0,"Color information has been lost !\n");
	  _mw_convert_struct_warning++;
     }      

     image->scale = image_ccimage->scale;

     strcpy(image->cmt,image_ccimage->cmt);
     strcpy(image->name,image_ccimage->name);

     image->firstcol = image_ccimage->firstcol;
     image->lastcol = image_ccimage->lastcol;
     image->firstrow = image_ccimage->firstrow;
     image->lastrow = image_ccimage->lastrow;

     return (image);
}

/*--- Ccimage to Cimage ---*/

/* MONO returns total intensity of r,g,b components */
#define MONO(rd,gn,bl) (((rd)*11 + (gn)*16 + (bl)*5) >> 5)  /*.33R+ .5G+ .17B*/

Cimage mw_ccimage_to_cimage(Ccimage image_ccimage, Cimage image)
{ 
     register unsigned char *gr,*r,*g,*b;
     register int i;
     int clost;

     image = mw_change_cimage(image,image_ccimage->nrow,image_ccimage->ncol);
     if (image == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Cimage !\n");
	  return(NULL);
     }
  
     clost = 0;
     for (i=1, gr=image->gray, r=image_ccimage->red, g=image_ccimage->green,
	       b=image_ccimage->blue;
	  i <= image->nrow * image->ncol;
	  i++, gr++, r++, g++, b++)
     {
	  *gr = MONO(*r,*g,*b);
	  if ((*r != *g) || (*g != *b)) clost++;
     }
     if ((clost != 0)&&(_mw_convert_struct_warning >= 0))
     {
	  mwerror(WARNING,0,"Color information has been lost !\n");
	  _mw_convert_struct_warning++;
     }      

     image->scale = image_ccimage->scale;

     strcpy(image->cmt,image_ccimage->cmt);
     strcpy(image->name,image_ccimage->name);

     image->firstcol = image_ccimage->firstcol;
     image->lastcol = image_ccimage->lastcol;
     image->firstrow = image_ccimage->firstrow;
     image->lastrow = image_ccimage->lastrow;

     return (image);
}

/*--- Cimage to Ccimage ---*/

Ccimage mw_cimage_to_ccimage(Cimage image_cimage, Ccimage image)
{
     register unsigned char *gr,*r,*g,*b;
     register int i;

     image = mw_change_ccimage(image,image_cimage->nrow,image_cimage->ncol);
     if (image == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Ccimage\n");
	  return(NULL);
     }

     for (i=1, gr=image_cimage->gray, r=image->red, g=image->green,b=image->blue;
	  i <= image->nrow * image->ncol;
	  i++, gr++, r++, g++, b++)
	  *r = *g = *b = *gr;

     image->scale = image_cimage->scale;

     strcpy(image->cmt,image_cimage->cmt);
     strcpy(image->name,image_cimage->name);

     image->firstcol = image_cimage->firstcol;
     image->lastcol = image_cimage->lastcol;
     image->firstrow = image_cimage->firstrow;
     image->lastrow = image_cimage->lastrow;

     return (image);
}


/*========================= CURVES / POLYGONS ===============================*/


/*--- Curves to Polys ---*/

Polygons mw_curves_to_polygons(Curves curves, Polygons polys)
{
     Polygon newpoly,oldpoly;
     Curve p;
     Point_curve pp, point, new_point;

     oldpoly = newpoly = NULL;
     polys = mw_change_polygons(polys);
     if (polys == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Polygons\n");
	  return(NULL);
     }

     for (p=curves->first; p; p=p->next)
     {
	  oldpoly = newpoly;
	  newpoly = mw_new_polygon();
	  if (newpoly == NULL)
	  {
	       mw_delete_polygons(polys);
	       mwerror(ERROR, 0,"Not enough memory to create Polygons\n");
	       return(NULL);	  
	  }
	  newpoly->first = p->first;
	  if (polys->first == NULL) polys->first = newpoly;
	  if (oldpoly != NULL) oldpoly->next = newpoly;
	  newpoly->previous = oldpoly;
	  newpoly->next = NULL;

	  point = NULL;
	  for (pp=p->first; pp; pp=pp->next)
	  {
	       if ((pp->next == NULL) && (pp->x == p->first->x) && 
		   (pp->y == p->first->y) && (p->first != pp))
		    /* Last point = first point : remove it in the polygon */
		    continue;

	       new_point = mw_new_point_curve();
	       if (new_point == NULL)
	       {
		    mw_delete_polygons(polys);
		    mwerror(ERROR, 0,"Not enough memory to create Polys\n");
		    return(NULL);
	       }
	       new_point->x = pp->x;
	       new_point->y = pp->y;
	       new_point->previous = point;
	       if (point == NULL) newpoly->first = new_point;
	       else point->next = new_point;
	       point = new_point;
	  }
     }
     return(polys);
}


/*--- Fcurves to Fpolys ---*/

Fpolygons mw_fcurves_to_fpolygons(Fcurves fcurves, Fpolygons fpolys)
{
     Fpolygon newfpoly,oldfpoly;
     Fcurve p;
     Point_fcurve pp, point, new_point;

     oldfpoly = newfpoly = NULL;
     fpolys = mw_change_fpolygons(fpolys);
     if (fpolys == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fpolygons\n");
	  return(NULL);
     }

     for (p=fcurves->first; p; p=p->next)
     {
	  oldfpoly = newfpoly;
	  newfpoly = mw_new_fpolygon();
	  if (newfpoly == NULL)
	  {
	       mw_delete_fpolygons(fpolys);
	       mwerror(ERROR, 0,"Not enough memory to create Fpolygons\n");
	       return(NULL);	  
	  }
	  newfpoly->first = p->first;
	  if (fpolys->first == NULL) fpolys->first = newfpoly;
	  if (oldfpoly != NULL) oldfpoly->next = newfpoly;
	  newfpoly->previous = oldfpoly;
	  newfpoly->next = NULL;

	  point = NULL;
	  for (pp=p->first; pp; pp=pp->next)
	  {
	       if ((pp->next == NULL) && (pp->x == p->first->x) && 
		   (pp->y == p->first->y) && (p->first != pp))
		    /* Last point = first point : remove it in the polygon */
		    continue;

	       new_point = mw_new_point_fcurve();
	       if (new_point == NULL)
	       {
		    mw_delete_fpolygons(fpolys);
		    mwerror(ERROR, 0,"Not enough memory to create Fpolys\n");
		    return(NULL);
	       }
	       new_point->x = pp->x;
	       new_point->y = pp->y;
	       new_point->previous = point;
	       if (point == NULL) newfpoly->first = new_point;
	       else point->next = new_point;
	       point = new_point;
	  }
     }
     return(fpolys);
}

/*--- Curves to Fcurves ---*/

Fcurves mw_curves_to_fcurves(Curves curves, Fcurves fcurves)
{
     Fcurve newfcurve,oldfcurve;
     Curve p;
     Point_curve pp;
     Point_fcurve point, new_point;

     oldfcurve = newfcurve = NULL;
     fcurves = mw_change_fcurves(fcurves);
     if (fcurves == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	  return(NULL);
     }

     for (p=curves->first; p; p=p->next)
     {
	  oldfcurve = newfcurve;
	  newfcurve = mw_new_fcurve();
	  if (newfcurve == NULL)
	  {
	       mw_delete_fcurves(fcurves);
	       mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	       return(NULL);	  
	  }
	  if (fcurves->first == NULL) fcurves->first = newfcurve;
	  if (oldfcurve != NULL) oldfcurve->next = newfcurve;
	  newfcurve->previous = oldfcurve;
	  newfcurve->next = NULL;

	  point = NULL;
	  for (pp=p->first; pp; pp=pp->next)
	  {
	       new_point = mw_new_point_fcurve();
	       if (new_point == NULL)
	       {
		    mw_delete_fcurves(fcurves);
		    mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
		    return(NULL);
	       }
	       new_point->x = pp->x;
	       new_point->y = pp->y;
	       new_point->previous = point;
	       if (point == NULL) newfcurve->first = new_point;
	       else point->next = new_point;
	       point = new_point;
	  }
     }
     return(fcurves);
}

/*--- Fcurves to Curves ---*/

Curves mw_fcurves_to_curves(Fcurves fcurves, Curves curves)
{
     Curve newcurve,oldcurve;
     Fcurve p;
     Point_fcurve pp;
     Point_curve point, new_point;
     char lost = 0;

     oldcurve = newcurve = NULL;
     curves = mw_change_curves(curves);
     if (curves == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Curves\n");
	  return(NULL);
     }

     for (p=fcurves->first; p; p=p->next)
     {
	  oldcurve = newcurve;
	  newcurve = mw_new_curve();
	  if (newcurve == NULL)
	  {
	       mw_delete_curves(curves);
	       mwerror(ERROR, 0,"Not enough memory to create Curves\n");
	       return(NULL);	  
	  }
	  if (curves->first == NULL) curves->first = newcurve;
	  if (oldcurve != NULL) oldcurve->next = newcurve;
	  newcurve->previous = oldcurve;
	  newcurve->next = NULL;

	  point = NULL;
	  for (pp=p->first; pp; pp=pp->next)
	  {
	       new_point = mw_new_point_curve();
	       if (new_point == NULL)
	       {
		    mw_delete_curves(curves);
		    mwerror(ERROR, 0,"Not enough memory to create Curves\n");
		    return(NULL);
	       }
	       new_point->x = (int) floor(pp->x + .5);
	       new_point->y = (int) floor(pp->y + .5);
	       if ((((float) new_point->x) != pp->x) || 
		   (((float) new_point->y) != pp->y))
		    lost = 1;
	       new_point->previous = point;
	       if (point == NULL) newcurve->first = new_point;
	       else point->next = new_point;
	       point = new_point;
	  }
     }
     if ((lost == 1)&&(_mw_convert_struct_warning >= 0))
     {
	  mwerror(WARNING,0,
		  "coordinates information has been lost due to quantization of the plane\n");
	  _mw_convert_struct_warning++;
     }
     return(curves);
}


/*--- Dcurves to Fcurves ---*/

Fcurves mw_dcurves_to_fcurves(Dcurves dcurves, Fcurves fcurves)
{
     Fcurve newfcurve,oldfcurve;
     Dcurve p;
     Point_dcurve pp;
     Point_fcurve point, new_point;
     int lost=0;

     oldfcurve = newfcurve = NULL;
     fcurves = mw_change_fcurves(fcurves);
     if (fcurves == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	  return(NULL);
     }

     for (p=dcurves->first; p; p=p->next)
     {
	  oldfcurve = newfcurve;
	  newfcurve = mw_new_fcurve();
	  if (newfcurve == NULL)
	  {
	       mw_delete_fcurves(fcurves);
	       mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	       return(NULL);	  
	  }
	  if (fcurves->first == NULL) fcurves->first = newfcurve;
	  if (oldfcurve != NULL) oldfcurve->next = newfcurve;
	  newfcurve->previous = oldfcurve;
	  newfcurve->next = NULL;

	  point = NULL;
	  for (pp=p->first; pp; pp=pp->next)
	  {
	       new_point = mw_new_point_fcurve();
	       if (new_point == NULL)
	       {
		    mw_delete_fcurves(fcurves);
		    mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
		    return(NULL);
	       }
	       new_point->x = pp->x;
	       new_point->y = pp->y;
	       if ((new_point->x !=  pp->x)||( new_point->y != pp->y)) lost=1;
	       new_point->previous = point;
	       if (point == NULL) newfcurve->first = new_point;
	       else point->next = new_point;
	       point = new_point;
	  }
     }

     if ((lost == 1)&&(_mw_convert_struct_warning >= 0))
     {
	  mwerror(WARNING,0,"coordinates accuracy has been reduced due to conversion from double to float.\n");
	  _mw_convert_struct_warning++;
     }

     return(fcurves);
}

/*--- Fcurves to Dcurves ---*/

Dcurves mw_fcurves_to_dcurves(Fcurves fcurves, Dcurves dcurves)
{
     Dcurve newcurve,oldcurve;
     Fcurve p;
     Point_fcurve pp;
     Point_dcurve point, new_point;

     oldcurve = newcurve = NULL;
     dcurves = mw_change_dcurves(dcurves);
     if (dcurves == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Dcurves\n");
	  return(NULL);
     }

     for (p=fcurves->first; p; p=p->next)
     {
	  oldcurve = newcurve;
	  newcurve = mw_new_dcurve();
	  if (newcurve == NULL)
	  {
	       mw_delete_dcurves(dcurves);
	       mwerror(ERROR, 0,"Not enough memory to create Dcurves\n");
	       return(NULL);	  
	  }
	  if (dcurves->first == NULL) dcurves->first = newcurve;
	  if (oldcurve != NULL) oldcurve->next = newcurve;
	  newcurve->previous = oldcurve;
	  newcurve->next = NULL;

	  point = NULL;
	  for (pp=p->first; pp; pp=pp->next)
	  {
	       new_point = mw_new_point_dcurve();
	       if (new_point == NULL)
	       {
		    mw_delete_dcurves(dcurves);
		    mwerror(ERROR, 0,"Not enough memory to create Dcurves\n");
		    return(NULL);
	       }
	       new_point->x = pp->x;
	       new_point->y = pp->y;
	       new_point->previous = point;
	       if (point == NULL) newcurve->first = new_point;
	       else point->next = new_point;
	       point = new_point;
	  }
     }
     return(dcurves);
}


/*--- Polys to Fpolys ---*/

Fpolygons mw_polygons_to_fpolygons(Polygons polys, Fpolygons fpolys)
{
     Fpolygon newfpoly,oldfpoly;
     Polygon p;
     Point_curve pp;
     Point_fcurve point, new_point;

     oldfpoly = newfpoly = NULL;
     fpolys = mw_change_fpolygons(fpolys);
     if (fpolys == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fpolys\n");
	  return(NULL);
     }


     for (p=polys->first; p; p=p->next)
     {
	  oldfpoly = newfpoly;
	  newfpoly = mw_new_fpolygon();
	  if (newfpoly == NULL)
	  {
	       mw_delete_fpolygons(fpolys);
	       mwerror(ERROR, 0,"Not enough memory to create Fpolys\n");
	       return(NULL);	  
	  }
	  if (fpolys->first == NULL) fpolys->first = newfpoly;
	  if (oldfpoly != NULL) oldfpoly->next = newfpoly;
	  newfpoly->previous = oldfpoly;
	  newfpoly->next = NULL;

	  point = NULL;
	  for (pp=p->first; pp; pp=pp->next)
	  {
	       new_point = mw_new_point_fcurve();
	       if (new_point == NULL)
	       {
		    mw_delete_fpolygons(fpolys);
		    mwerror(ERROR, 0,"Not enough memory to create Fpolys\n");
		    return(NULL);
	       }
	       new_point->x = pp->x;
	       new_point->y = pp->y;
	       new_point->previous = point;
	       if (point == NULL) newfpoly->first = new_point;
	       else point->next = new_point;
	       point = new_point;
	  }
     }
     return(fpolys);
}

/*--- Fpolys to Polys ---*/

Polygons mw_fpolygons_to_polygons(Fpolygons fpolys, Polygons polys)
{
     Polygon newpoly,oldpoly;
     Fpolygon p;
     Point_fcurve pp;
     Point_curve point, new_point;
     char lost = 0;

     oldpoly = newpoly = NULL;
     polys = mw_change_polygons(polys);
     if (polys == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Polys\n");
	  return(NULL);
     }

     for (p=fpolys->first; p; p=p->next)
     {
	  oldpoly = newpoly;
	  newpoly = mw_new_polygon();
	  if (newpoly == NULL)
	  {
	       mw_delete_polygons(polys);
	       mwerror(ERROR, 0,"Not enough memory to create Polys\n");
	       return(NULL);	  
	  }
	  if (polys->first == NULL) polys->first = newpoly;
	  if (oldpoly != NULL) oldpoly->next = newpoly;
	  newpoly->previous = oldpoly;
	  newpoly->next = NULL;

	  point = NULL;
	  for (pp=p->first; pp; pp=pp->next)
	  {
	       new_point = mw_new_point_curve();
	       if (new_point == NULL)
	       {
		    mw_delete_polygons(polys);
		    mwerror(ERROR, 0,"Not enough memory to create Polys\n");
		    return(NULL);
	       }
	       new_point->x = (int) floor(pp->x + .5);
	       new_point->y = (int) floor(pp->y + .5);
	       if ((((float) new_point->x) != pp->x) || 
		   (((float) new_point->y) != pp->y))
		    lost = 1;
	       new_point->previous = point;
	       if (point == NULL) newpoly->first = new_point;
	       else point->next = new_point;
	       point = new_point;
	  }
     }
     if ((lost == 1)&&(_mw_convert_struct_warning >= 0))
     {
	  mwerror(WARNING,0,
		  "coordinates information has been lost due to quantization of the plane\n");
	  _mw_convert_struct_warning++;
     }
     return(polys);
}


/*--- Fpolys to Fcurves ---*/

Fcurves mw_fpolygons_to_fcurves(Fpolygons fpolys, Fcurves fcurves)
{ 
     Fcurve newcv,oldcv;
     Fpolygon p;
     Point_fcurve pp, old_pp, point, new_point;
     char no_channel=0;

     oldcv = newcv = NULL;
     fcurves = mw_change_fcurves(fcurves);
     if (fcurves == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	  return(NULL);
     }
  
     for (p=fpolys->first; p; p=p->next)
     {
	  oldcv = newcv;
	  newcv = mw_new_fcurve();
	  if (newcv == NULL)
	  {
	       mw_delete_fcurves(fcurves);
	       mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	       return(NULL);
	  }
	  newcv->first = p->first;
	  if (fcurves->first == NULL) fcurves->first = newcv;
	  if (oldcv != NULL) oldcv->next = newcv;
	  newcv->previous = oldcv;
	  newcv->next = NULL;

	  point = NULL;
	  for (pp=p->first, old_pp=NULL; pp || old_pp;)
	  {
	       new_point = mw_new_point_fcurve();
	       if (new_point == NULL)
	       {
		    mw_delete_fcurves(fcurves);
		    mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
		    return(NULL);
	       }
	       if (pp)  /* Copy the point */
	       {
		    new_point->x = pp->x;
		    new_point->y = pp->y;
	       }
	       else /* Close the curve since a polygon is a closed curve */
	       {
		    new_point->x = p->first->x;
		    new_point->y = p->first->y;
	       }
	       new_point->previous = point;
	       if (point == NULL) newcv->first = new_point;
	       else point->next = new_point;
	       point = new_point;

	       old_pp = pp;
	       if (pp) pp=pp->next;
	  }

	  if ((p->nb_channels > 0)&&(no_channel==0))
	  {
	       mwerror(WARNING,0,"Channel information has been lost !\n");
	       no_channel=1;
	  }
     }
     return(fcurves);
}

/*--- Polys to Curves ---*/

Curves mw_polygons_to_curves(Polygons polys, Curves curves)
{ 
     Curve newcv,oldcv;
     Polygon p;
     Point_curve pp, old_pp, point, new_point;
     char no_channel=0;

     oldcv = newcv = NULL;
     curves = mw_change_curves(curves);
     if (curves == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Curves\n");
	  return(NULL);
     }
  
     for (p=polys->first; p; p=p->next)
     {
	  oldcv = newcv;
	  newcv = mw_new_curve();
	  if (newcv == NULL)
	  {
	       mw_delete_curves(curves);
	       mwerror(ERROR, 0,"Not enough memory to create Curves\n");
	       return(NULL);
	  }
	  if (curves->first == NULL) curves->first = newcv;
	  if (oldcv != NULL) oldcv->next = newcv;
	  newcv->previous = oldcv;
	  newcv->next = NULL;

	  point = NULL;
	  for (pp=p->first, old_pp=NULL; pp || old_pp;)
	  {
	       new_point = mw_new_point_curve();
	       if (new_point == NULL)
	       {
		    mw_delete_curves(curves);
		    mwerror(ERROR, 0,"Not enough memory to create Curves\n");
		    return(NULL);
	       }
	       if (pp)  /* Copy the point */
	       {
		    new_point->x = pp->x;
		    new_point->y = pp->y;
	       }
	       else /* Close the curve since a polygon is a closed curve */
	       {
		    new_point->x = p->first->x;
		    new_point->y = p->first->y;
	       }
	       new_point->previous = point;
	       if (point == NULL) newcv->first = new_point;
	       else point->next = new_point;
	       point = new_point;

	       old_pp = pp;
	       if (pp) pp=pp->next;
	  }
	  if ((p->nb_channels > 0)&&(no_channel==0))
	  {
	       mwerror(WARNING,0,"Channel information has been lost !\n");
	       no_channel=1;
	  }
     }
     return(curves);
}

/*========================= CURVE / POLYGON ===============================*/


/*--- Curve to Poly ---*/

Polygon mw_curve_to_polygon(Curve curve, Polygon poly)
{
     Point_curve pp, point, new_point;

     poly = mw_change_polygon(poly,0);
     if (poly == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Polygon\n");
	  return(NULL);
     }

     point = NULL;
     for (pp=curve->first; pp; pp=pp->next)
     {
	  if ((pp->next == NULL) && (pp->x == curve->first->x) && 
	      (pp->y == curve->first->y) && (curve->first != pp))
	       /* Last point = first point : remove it in the polygon */
	       continue;
      
	  new_point = mw_new_point_curve();
	  if (new_point == NULL)
	  {
	       mw_delete_polygon(poly);
	       mwerror(ERROR, 0,"Not enough memory to create Poly\n");
	       return(NULL);
	  }
	  new_point->x = pp->x;
	  new_point->y = pp->y;
	  new_point->previous = point;
	  if (point == NULL) poly->first = new_point;
	  else point->next = new_point;
	  point = new_point;
     }
     return(poly);
}


/*--- Fcurve to Fpoly ---*/

Fpolygon mw_fcurve_to_fpolygon(Fcurve fcurve, Fpolygon fpoly)
{
     Point_fcurve pp, point, new_point;

     fpoly = mw_change_fpolygon(fpoly,0);
     if (fpoly == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fpolygon\n");
	  return(NULL);
     }

     point = NULL;
     for (pp=fcurve->first; pp; pp=pp->next)
     {
	  if ((pp->next == NULL) && (pp->x == fcurve->first->x) && 
	      (pp->y == fcurve->first->y) && (fcurve->first != pp))
	       /* Last point = first point : remove it in the fpolygon */
	       continue;
      
	  new_point = mw_new_point_fcurve();
	  if (new_point == NULL)
	  {
	       mw_delete_fpolygon(fpoly);
	       mwerror(ERROR, 0,"Not enough memory to create Fpoly\n");
	       return(NULL);
	  }
	  new_point->x = pp->x;
	  new_point->y = pp->y;
	  new_point->previous = point;
	  if (point == NULL) fpoly->first = new_point;
	  else point->next = new_point;
	  point = new_point;
     }
     return(fpoly);
}

/*--- Point_curve to Point_fcurve ---*/

Point_fcurve mw_point_curve_to_point_fcurve(Point_curve pcurve, 
					    Point_fcurve first_point)
{
     Point_curve pp;
     Point_fcurve point, new_point;

     point = NULL;
     for (pp=pcurve; pp; pp=pp->next)
     {
	  if ((pp==pcurve)&&(first_point)) 
	       new_point=first_point;
	  else
	       new_point = mw_new_point_fcurve();
	  if (new_point == NULL)
	  {
	       if (first_point) mw_delete_point_fcurve(first_point);
	       mwerror(ERROR, 0,"Not enough memory to create a point_fcurve\n");
	       return(NULL);
	  }
	  new_point->x = pp->x;
	  new_point->y = pp->y;
	  new_point->previous = point;
	  if (point == NULL) first_point = new_point;
	  else point->next = new_point;
	  point = new_point;
     }
     return(first_point);
}

/*--- Curve to Fcurve ---*/

Fcurve mw_curve_to_fcurve(Curve curve, Fcurve fcurve)
{

     fcurve = mw_change_fcurve(fcurve);
     if (fcurve == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fcurve\n");
	  return(NULL);
     }
     fcurve->first = mw_point_curve_to_point_fcurve(curve->first,NULL);
     return(fcurve);
}

/*--- Point_fcurve to Point_curve ---*/

Point_curve mw_point_fcurve_to_point_curve(Point_fcurve pfcurve, 
					   Point_curve first_point)
{
     Point_fcurve pp;
     Point_curve point, new_point;
     char lost = 0;

     point = first_point = NULL;
     for (pp=pfcurve; pp; pp=pp->next)
     {
	  if ((pp==pfcurve)&&(first_point)) 
	       new_point=first_point;
	  else
	       new_point = mw_new_point_curve();
	  if (new_point == NULL)
	  {
	       if (first_point) mw_delete_point_curve(first_point);
	       mwerror(ERROR, 0,"Not enough memory to create point_curve\n");
	       return(NULL);
	  }
	  new_point->x = (int) floor(pp->x + .5);
	  new_point->y = (int) floor(pp->y + .5);
	  if ((((float) new_point->x) != pp->x) || 
	      (((float) new_point->y) != pp->y))
	       lost = 1;
	  new_point->previous = point;
	  if (point == NULL) first_point = new_point;
	  else point->next = new_point;
	  point = new_point;
     }

     if ((lost == 1)&&(_mw_convert_struct_warning >= 0))
     {
	  mwerror(WARNING,0,
		  "coordinates information has been lost due to quantization of the plane\n");
	  _mw_convert_struct_warning++;
     }
     return(first_point);
}


/*--- Fcurve to Curve ---*/

Curve mw_fcurve_to_curve(Fcurve fcurve, Curve curve)
{
     curve = mw_change_curve(curve);
     if (curve == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Curve\n");
	  return(NULL);
     }

     curve->first = mw_point_fcurve_to_point_curve(fcurve->first,NULL);
     return(curve);
}

/*--- Point_dcurve to Point_fcurve ---*/

Point_fcurve mw_point_dcurve_to_point_fcurve(Point_dcurve pcurve, 
					     Point_fcurve first_point)
{
     Point_dcurve pp;
     Point_fcurve point, new_point;
     char lost = 0;

     point = first_point = NULL;
     for (pp=pcurve; pp; pp=pp->next)
     {
	  if ((pp==pcurve)&&(first_point)) 
	       new_point=first_point;
	  else
	       new_point = mw_new_point_fcurve();
	  if (new_point == NULL)
	  {
	       if (first_point) mw_delete_point_fcurve(first_point);
	       mwerror(ERROR, 0,"Not enough memory to create a point_fcurve\n");
	       return(NULL);
	  }
	  new_point->x = pp->x;
	  new_point->y = pp->y;
	  if ((new_point->x !=  pp->x)||( new_point->y != pp->y)) lost=1;
	  new_point->previous = point;
	  if (point == NULL) first_point = new_point;
	  else point->next = new_point;
	  point = new_point;
     }

     if ((lost == 1)&&(_mw_convert_struct_warning >= 0))
     {
	  mwerror(WARNING,0,"coordinates accuracy has been reduced due to conversion from double to float.\n");
	  _mw_convert_struct_warning++;
     }

     return(first_point);
}

/*--- Dcurve to Fcurve ---*/

Fcurve mw_dcurve_to_fcurve(Dcurve dcurve, Fcurve fcurve)
{
     fcurve = mw_change_fcurve(fcurve);
     if (fcurve == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fcurve\n");
	  return(NULL);
     }
     fcurve->first = mw_point_dcurve_to_point_fcurve(dcurve->first,NULL);
     return(fcurve);
}

/*--- Point_fcurve to Point_dcurve ---*/

Point_dcurve mw_point_fcurve_to_point_dcurve(Point_fcurve pfcurve, 
					     Point_dcurve first_point)
{
     Point_fcurve pp;
     Point_dcurve point, new_point;

     point = first_point = NULL;
     for (pp=pfcurve; pp; pp=pp->next)
     {
	  if ((pp==pfcurve)&&(first_point)) 
	       new_point=first_point;
	  else
	       new_point = mw_new_point_dcurve();
	  if (new_point == NULL)
	  {
	       if (first_point) mw_delete_point_dcurve(first_point);
	       mwerror(ERROR, 0,"Not enough memory to create point_dcurve\n");
	       return(NULL);
	  }
	  new_point->x = pp->x;
	  new_point->y = pp->y;
	  new_point->previous = point;
	  if (point == NULL) first_point = new_point;
	  else point->next = new_point;
	  point = new_point;
     }
     return(first_point);
}


/*--- Fcurve to Dcurve ---*/

Dcurve mw_fcurve_to_dcurve(Fcurve fcurve, Dcurve dcurve)
{
     dcurve = mw_change_dcurve(dcurve);
     if (dcurve == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Dcurve\n");
	  return(NULL);
     }

     dcurve->first = mw_point_fcurve_to_point_dcurve(fcurve->first,NULL);
     return(dcurve);
}


/*--- Poly to Fpoly ---*/

Fpolygon mw_polygon_to_fpolygon(Polygon poly, Fpolygon fpoly)
{
     Point_curve pp;
     Point_fcurve point, new_point;

     fpoly = mw_change_fpolygon(fpoly,0);
     if (fpoly == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fpoly\n");
	  return(NULL);
     }

     point = NULL;
     for (pp=poly->first; pp; pp=pp->next)
     {
	  new_point = mw_new_point_fcurve();
	  if (new_point == NULL)
	  {
	       mw_delete_fpolygon(fpoly);
	       mwerror(ERROR, 0,"Not enough memory to create Fpoly\n");
	       return(NULL);
	  }
	  new_point->x = pp->x;
	  new_point->y = pp->y;
	  new_point->previous = point;
	  if (point == NULL) fpoly->first = new_point;
	  else point->next = new_point;
	  point = new_point;
     }
     return(fpoly);
}

/*--- Fpoly to Poly ---*/

Polygon mw_fpolygon_to_polygon(Fpolygon fpoly, Polygon poly)
{
     Point_fcurve pp;
     Point_curve point, new_point;
     char lost = 0;

     poly = mw_change_polygon(poly,0);
     if (poly == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Poly\n");
	  return(NULL);
     }

     point = NULL;
     for (pp=fpoly->first; pp; pp=pp->next)
     {
	  new_point = mw_new_point_curve();
	  if (new_point == NULL)
	  {
	       mw_delete_polygon(poly);
	       mwerror(ERROR, 0,"Not enough memory to create Poly\n");
	       return(NULL);
	  }
	  new_point->x = (int) floor(pp->x + .5);
	  new_point->y = (int) floor(pp->y + .5);
	  if ((((float) new_point->x) != pp->x) || 
	      (((float) new_point->y) != pp->y))
	       lost = 1;
	  new_point->previous = point;
	  if (point == NULL) poly->first = new_point;
	  else point->next = new_point;
	  point = new_point;
     }
     if ((lost == 1)&&(_mw_convert_struct_warning >= 0))
     {
	  mwerror(WARNING,0,
		  "coordinates information has been lost due to quantization of the plane\n");
	  _mw_convert_struct_warning++;
     }
     return(poly);
}


/*--- Fpoly to Fcurve ---*/

Fcurve mw_fpolygon_to_fcurve(Fpolygon fpoly, Fcurve fcurve)
{ 
     Point_fcurve pp, old_pp, point, new_point;
     char no_channel=0;

     fcurve = mw_change_fcurve(fcurve);
     if (fcurve == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fcurve\n");
	  return(NULL);
     }
  
     point = NULL;
     for (pp=fpoly->first, old_pp=NULL; pp || old_pp;)
     {
	  new_point = mw_new_point_fcurve();
	  if (new_point == NULL)
	  {
	       mw_delete_fcurve(fcurve);
	       mwerror(ERROR, 0,"Not enough memory to create Fcurve\n");
	       return(NULL);
	  }
	  if (pp)  /* Copy the point */
	  {
	       new_point->x = pp->x;
	       new_point->y = pp->y;
	  }
	  else /* Close the curve since a polygon is a closed curve */
	  {
	       new_point->x = fpoly->first->x;
	       new_point->y = fpoly->first->y;
	  }
	  new_point->previous = point;
	  if (point == NULL) fcurve->first = new_point;
	  else point->next = new_point;
	  point = new_point;

	  old_pp = pp;
	  if (pp) pp=pp->next;
     }

     if ((fpoly->nb_channels > 0)&&(no_channel==0))
     {
	  mwerror(WARNING,0,"Channel information has been lost !\n");
	  no_channel=1;
     }
     return(fcurve);
}

/*--- Poly to Curve ---*/

Curve mw_polygon_to_curve(Polygon poly, Curve curve)
{ 
     Point_curve pp, old_pp, point, new_point;
     char no_channel=0;

     curve = mw_change_curve(curve);
     if (curve == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Curve\n");
	  return(NULL);
     }
  
     point = NULL;
     for (pp=poly->first, old_pp=NULL; pp || old_pp; )
     {
	  new_point = mw_new_point_curve();
	  if (new_point == NULL)
	  {
	       mw_delete_curve(curve);
	       mwerror(ERROR, 0,"Not enough memory to create Curve\n");
	       return(NULL);
	  }
	  if (pp)  /* Copy the point */
	  {
	       new_point->x = pp->x;
	       new_point->y = pp->y;
	  }
	  else /* Close the curve since a polygon is a closed curve */
	  {
	       new_point->x = poly->first->x;
	       new_point->y = poly->first->y;
	  }
	  new_point->previous = point;
	  if (point == NULL) curve->first = new_point;
	  else point->next = new_point;
	  point = new_point;

	  old_pp = pp;
	  if (pp) pp=pp->next;
     }
     if ((poly->nb_channels > 0)&&(no_channel==0))
     {
	  mwerror(WARNING,0,"Channel information has been lost !\n");
	  no_channel=1;
     }
     return(curve);
}


/*========================= CURVE / CURVES ===============================*/

/*--- Curves to Curve ---*/

Curve mw_curves_to_curve(Curves curves, Curve curve)
{
     Curve p;
     Point_curve pp, point, new_point;

     curve = mw_change_curve(curve);
     if (curve == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a curve\n");
	  return(NULL);
     }

     p=curves->first; 
     if (p->next)
	  mwerror(WARNING,0,"Only the first curve is kept !\n");

     point = NULL;
     for (pp=p->first; pp; pp=pp->next)
     {
	  new_point = mw_new_point_curve();
	  if (new_point == NULL)
	  {
	       mw_delete_curve(curve);
	       mwerror(ERROR, 0,"Not enough memory to create a curve\n");
	       return(NULL);
	  }
	  new_point->x = pp->x;
	  new_point->y = pp->y;
	  new_point->previous = point;
	  if (point == NULL) curve->first = new_point;
	  else point->next = new_point;
	  point = new_point;
     }
     return(curve);
}

/*--- Curve to Curves ---*/

Curves mw_curve_to_curves(Curve curve, Curves curves)
{ 
     Curve newcv;
     Point_curve pp, old_pp, point, new_point;

     newcv = NULL;
     curves = mw_change_curves(curves);
     if (curves == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Curves\n");
	  return(NULL);
     }
     newcv = mw_new_curve();
     if (newcv == NULL)
     {
	  mw_delete_curves(curves);
	  mwerror(ERROR, 0,"Not enough memory to create Curves\n");
	  return(NULL);
     }
     curves->first = newcv;
     newcv->next = newcv->previous = NULL;
  
     point = NULL;
     for (pp=curve->first, old_pp=NULL; pp ; old_pp=pp, pp=pp->next)
     {
	  new_point = mw_new_point_curve();
	  if (new_point == NULL)
	  {
	       mw_delete_curves(curves);
	       mwerror(ERROR, 0,"Not enough memory to create Curves\n");
	       return(NULL);
	  }
	  new_point->x = pp->x;
	  new_point->y = pp->y;
	  new_point->previous = point;
	  if (point == NULL) newcv->first = new_point;
	  else point->next = new_point;
	  point = new_point;
     }
     return(curves);
}


/*========================= FCURVE / FCURVES ===============================*/

/*--- Fcurves to Fcurve ---*/

Fcurve mw_fcurves_to_fcurve(Fcurves fcurves, Fcurve fcurve)
{
     Fcurve p;
     Point_fcurve pp, point, new_point;

     fcurve = mw_change_fcurve(fcurve);
     if (fcurve == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a fcurve\n");
	  return(NULL);
     }

     p=fcurves->first; 
     if (p->next)
	  mwerror(WARNING,0,"Only the first fcurve is kept !\n");

     point = NULL;
     for (pp=p->first; pp; pp=pp->next)
     {
	  new_point = mw_new_point_fcurve();
	  if (new_point == NULL)
	  {
	       mw_delete_fcurve(fcurve);
	       mwerror(ERROR, 0,"Not enough memory to create a fcurve\n");
	       return(NULL);
	  }
	  new_point->x = pp->x;
	  new_point->y = pp->y;
	  new_point->previous = point;
	  if (point == NULL) fcurve->first = new_point;
	  else point->next = new_point;
	  point = new_point;
     }
     return(fcurve);
}

/*--- Fcurve to Fcurves ---*/

Fcurves mw_fcurve_to_fcurves(Fcurve fcurve, Fcurves fcurves)
{ 
     Fcurve newcv;
     Point_fcurve pp, old_pp, point, new_point;

     newcv = NULL;
     fcurves = mw_change_fcurves(fcurves);
     if (fcurves == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	  return(NULL);
     }
     newcv = mw_new_fcurve();
     if (newcv == NULL)
     {
	  mw_delete_fcurves(fcurves);
	  mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	  return(NULL);
     }
     fcurves->first = newcv;
     newcv->next = newcv->previous = NULL;
  
     point = NULL;
     for (pp=fcurve->first, old_pp=NULL; pp ; old_pp=pp, pp=pp->next)
     {
	  new_point = mw_new_point_fcurve();
	  if (new_point == NULL)
	  {
	       mw_delete_fcurves(fcurves);
	       mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	       return(NULL);
	  }
	  new_point->x = pp->x;
	  new_point->y = pp->y;
	  new_point->previous = point;
	  if (point == NULL) newcv->first = new_point;
	  else point->next = new_point;
	  point = new_point;
     }
     return(fcurves);
}


/*========================= MORPHO_LINE / CURVE ===============================*/

Morpho_line  mw_curve_to_morpho_line(Curve curve, Morpho_line ll)
{
     if (((ll = mw_change_morpho_line(ll)) == NULL)
	 ||
	 ((ll->first_point = mw_change_point_curve(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a morpho_line\n");
	  return(NULL);
     }

     mw_copy_point_curve(curve->first,ll->first_point);
     return(ll);
}

Curve mw_morpho_line_to_curve(Morpho_line ll, Curve cv)
{
     if (((cv = mw_change_curve(cv)) == NULL)
	 ||
	 ((cv->first = mw_change_point_curve(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a curve\n");
	  return(NULL);
     }
     mw_copy_point_curve(ll->first_point,cv->first);
     if ((ll->minvalue != 0.0)  || (ll->maxvalue != 0.0))
	  mwerror(WARNING,0,"Gray level values of the morpho line have been lost !\n");
     if (ll->first_type)
	  mwerror(WARNING,0,"Point types of the morpho line have been lost !\n");

     return(cv);
}

/*========================= FMORPHO_LINE / FCURVE ===============================*/

Fmorpho_line  mw_fcurve_to_fmorpho_line(Fcurve fcurve, Fmorpho_line fll)
{
     if (((fll = mw_change_fmorpho_line(fll)) == NULL)
	 ||
	 ((fll->first_point = mw_change_point_fcurve(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a fmorpho_line\n");
	  return(NULL);
     }

     mw_copy_point_fcurve(fcurve->first,fll->first_point);
     return(fll);
}

Fcurve mw_fmorpho_line_to_fcurve(Fmorpho_line fll, Fcurve cv)
{
     if (((cv = mw_change_fcurve(cv)) == NULL)
	 ||
	 ((cv->first = mw_change_point_fcurve(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a fcurve\n");
	  return(NULL);
     }
     mw_copy_point_fcurve(fll->first_point,cv->first);
     if ((fll->minvalue != 0.0) || (fll->maxvalue != 0.0))
	  mwerror(WARNING,0,"Gray level values of the fmorpho line have been lost !\n");
     if (fll->first_type)
	  mwerror(WARNING,0,"Point types of the fmorpho line have been lost !\n");

     return(cv);
}


/*========================= MORPHO_LINE / FMORPHO_LINE ===============================*/

/*--- Morpho_line to Fmorpho_line ---*/

Fmorpho_line mw_morpho_line_to_fmorpho_line(Morpho_line ll, Fmorpho_line fll)
{
     if ( ((fll = mw_change_fmorpho_line(fll)) == NULL) ||
	  ((fll->first_type = mw_new_point_type()) == NULL) )
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fmorpho_line\n");
	  return(NULL);
     }
     fll->minvalue = ll->minvalue;
     fll->maxvalue = ll->maxvalue;
     fll->open = ll->open;
     fll->data = ll->data;
     fll->first_point = mw_point_curve_to_point_fcurve(ll->first_point,NULL);
     mw_copy_point_type(ll->first_type,fll->first_type);

     return(fll);
}

/*--- Fmorpho_line to Morpho_line ---*/

Morpho_line mw_fmorpho_line_to_morpho_line(Fmorpho_line fll, Morpho_line ll)
{
     if ( ((ll = mw_change_morpho_line(ll)) == NULL) ||
	  ((ll->first_type = mw_new_point_type()) == NULL) )
     {
	  mwerror(ERROR, 0,"Not enough memory to create Morpho_line\n");
	  return(NULL);
     }
     ll->minvalue = fll->minvalue;
     ll->maxvalue = fll->maxvalue;
     ll->open = fll->open;
     ll->data = fll->data;
     ll->first_point = mw_point_fcurve_to_point_curve(fll->first_point,NULL);
     mw_copy_point_type(fll->first_type,ll->first_type);

     return(ll);
}

/*========================= MORPHO_LINE / MIMAGE ===============================*/


Mimage mw_morpho_line_to_mimage(Morpho_line ll, Mimage mimage)
{

     if (((mimage = mw_change_mimage(mimage)) == NULL)
	 ||
	 ((mimage->first_ml = mw_change_morpho_line(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a mimage\n");
	  return(NULL);
     }

     mw_copy_morpho_line(ll,mimage->first_ml);
     return(mimage);
}

Morpho_line mw_mimage_to_morpho_line(Mimage mimage, Morpho_line ll)
{
     if (!mimage->first_ml)
     {
	  mwerror(WARNING,0,"Conversion failed : input mimage does not contain any morpho_line.\n");
	  return(NULL);
     }

     ll = mw_change_morpho_line(ll);
     if (ll == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a morpho_line.\n");
	  return(NULL);
     }

     if (mimage->first_ml->next) 
	  mwerror(WARNING,0,"Only the first morpho line is kept !\n");
     mw_copy_morpho_line(mimage->first_ml,ll);
     return(ll);
}

/*========================= FMORPHO_LINE / MIMAGE ===============================*/


Mimage mw_fmorpho_line_to_mimage(Fmorpho_line fll, Mimage mimage)
{
     if (((mimage = mw_change_mimage(mimage)) == NULL)
	 ||
	 ((mimage->first_fml = mw_change_fmorpho_line(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a mimage\n");
	  return(NULL);
     }

     mw_copy_fmorpho_line(fll,mimage->first_fml);
     return(mimage);
}

Fmorpho_line mw_mimage_to_fmorpho_line(Mimage mimage, Fmorpho_line fll)
{
     if (!mimage->first_fml)
     {
	  mwerror(WARNING,0,"Conversion failed : input mimage does not contain any fmorpho_line.\n");
	  return(NULL);
     }

     fll = mw_change_fmorpho_line(fll);
     if (fll == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a fmorpho_line\n");
	  return(NULL);
     }

     if (mimage->first_fml->next) 
	  mwerror(WARNING,0,"Only the first fmorpho line is kept !\n");
     mw_copy_fmorpho_line(mimage->first_fml,fll);
     return(fll);
}

/*========================= CURVES / MIMAGE ===============================*/

Curves mw_mimage_to_curves(Mimage mimage, Curves curves)
{
     Curve newcurve,oldcurve;
     Morpho_line ll;
     Fmorpho_line fll;

     if (!mimage->first_ml && !mimage->first_fml)
     {
	  mwerror(WARNING,0,"Conversion failed : input mimage does not contain any morpho_line or fmorpho_line.\n");
	  return(NULL);
     }

     oldcurve = newcurve = NULL;
     curves = mw_change_curves(curves);
     if (curves == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Curves\n");
	  return(NULL);
     }      

     if (mimage->first_ml && mimage->first_fml)
	  mwerror(WARNING,0,"The mimage structure contains both morpho lines and fmorpho lines. Using morpho lines only.\n");

     /* Use morpho lines */
     for (ll=mimage->first_ml; ll; ll=ll->next)
     {
	  oldcurve = newcurve;
	  if ( ((newcurve = mw_new_curve()) == NULL)
	       || 
	       ((newcurve->first = mw_change_point_curve(NULL)) == NULL) )
	  {
	       mw_delete_curves(curves);
	       mwerror(ERROR, 0,"Not enough memory to create Curves\n");
	       return(NULL);	  
	  }
	  if (curves->first == NULL) curves->first = newcurve;
	  if (oldcurve != NULL) oldcurve->next = newcurve;
	  newcurve->previous = oldcurve;
	  newcurve->next = NULL;
	  mw_copy_point_curve(ll->first_point,newcurve->first);
     }
     if (mimage->first_ml) return(curves);

     /* Use fmorpho lines */
     for (fll=mimage->first_fml; fll; fll=fll->next)
     {
	  oldcurve = newcurve;
	  if ((newcurve = mw_new_curve()) == NULL)
	  {
	       mw_delete_curves(curves);
	       mwerror(ERROR, 0,"Not enough memory to create Curves\n");
	       return(NULL);	  
	  }
	  if (curves->first == NULL) curves->first = newcurve;
	  if (oldcurve != NULL) oldcurve->next = newcurve;
	  newcurve->previous = oldcurve;
	  newcurve->next = NULL;
	  newcurve->first = mw_point_fcurve_to_point_curve(fll->first_point,NULL);
     }
     return(curves);
}

Mimage mw_curves_to_mimage(Curves curves, Mimage mimage)
{
     Morpho_line newll,oldll;
     Curve cv;

     if ((mimage = mw_change_mimage(mimage)) == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a mimage\n");
	  return(NULL);
     }

     oldll = newll = NULL;
     for (cv=curves->first; cv; cv=cv->next)
     {
	  oldll = newll;
	  if ( ((newll = mw_new_morpho_line()) == NULL)
	       || 
	       ((newll->first_point = mw_change_point_curve(NULL)) == NULL) )
	  {
	       mw_delete_mimage(mimage);
	       mwerror(ERROR, 0,"Not enough memory to create a mimage\n");
	       return(NULL);	  
	  }
	  if (mimage->first_ml == NULL) mimage->first_ml = newll;
	  if (oldll != NULL) oldll->next = newll;
	  newll->previous = oldll;
	  newll->next = NULL;
	  mw_copy_point_curve(cv->first,newll->first_point);
     }

     return(mimage);
}

/*========================= FCURVES / MIMAGE ===============================*/

Fcurves mw_mimage_to_fcurves(Mimage mimage, Fcurves fcurves)
{
     Fcurve newfcurve,oldfcurve;
     Fmorpho_line fll;
     Morpho_line ll;

     if (!mimage->first_ml && !mimage->first_fml)
     {
	  mwerror(WARNING,0,"Conversion failed : input mimage does not contain any morpho_line or fmorpho_line.\n");
	  return(NULL);
     }

     oldfcurve = newfcurve = NULL;
     fcurves = mw_change_fcurves(fcurves);
     if (fcurves == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	  return(NULL);
     }

     if (mimage->first_ml && mimage->first_fml)
	  mwerror(WARNING,0,"The mimage structure contains both morpho lines and fmorpho lines. Using fmorpho lines only.\n");

     /* Use fmorpho lines */  
     for (fll=mimage->first_fml; fll; fll=fll->next)
     {
	  oldfcurve = newfcurve;
	  if ( ((newfcurve = mw_new_fcurve()) == NULL)
	       || 
	       ((newfcurve->first = mw_change_point_fcurve(NULL)) == NULL) )
	  {
	       mw_delete_fcurves(fcurves);
	       mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	       return(NULL);	  
	  }
	  if (fcurves->first == NULL) fcurves->first = newfcurve;
	  if (oldfcurve != NULL) oldfcurve->next = newfcurve;
	  newfcurve->previous = oldfcurve;
	  newfcurve->next = NULL;
	  mw_copy_point_fcurve(fll->first_point,newfcurve->first);
     }
     if (mimage->first_fml) return(fcurves);

     /* Use morpho lines */  
     for (ll=mimage->first_ml; ll; ll=ll->next)
     {
	  oldfcurve = newfcurve;
	  if ((newfcurve = mw_new_fcurve()) == NULL)
	  {
	       mw_delete_fcurves(fcurves);
	       mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	       return(NULL);	  
	  }
	  if (fcurves->first == NULL) fcurves->first = newfcurve;
	  if (oldfcurve != NULL) oldfcurve->next = newfcurve;
	  newfcurve->previous = oldfcurve;
	  newfcurve->next = NULL;
	  newfcurve->first = mw_point_curve_to_point_fcurve(ll->first_point,NULL);
     }

     return(fcurves);
}

Mimage mw_fcurves_to_mimage(Fcurves fcurves, Mimage mimage)
{
     Fmorpho_line newfll,oldfll;
     Fcurve cv;

     if ((mimage = mw_change_mimage(mimage)) == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a mimage\n");
	  return(NULL);
     }

     oldfll = newfll = NULL;
     for (cv=fcurves->first; cv; cv=cv->next)
     {
	  oldfll = newfll;
	  if ( ((newfll = mw_new_fmorpho_line()) == NULL)
	       || 
	       ((newfll->first_point = mw_change_point_fcurve(NULL)) == NULL) )
	  {
	       mw_delete_mimage(mimage);
	       mwerror(ERROR, 0,"Not enough memory to create a mimage\n");
	       return(NULL);	  
	  }
	  if (mimage->first_fml == NULL) mimage->first_fml = newfll;
	  if (oldfll != NULL) oldfll->next = newfll;
	  newfll->previous = oldfll;
	  newfll->next = NULL;
	  mw_copy_point_fcurve(cv->first,newfll->first_point);
     }

     return(mimage);
}


/*========================= MORPHO_SET / MORPHO_SETS ===============================*/


Morpho_sets mw_morpho_set_to_morpho_sets(Morpho_set is, Morpho_sets morpho_sets)
{
     if (((morpho_sets = mw_change_morpho_sets(morpho_sets)) == NULL)
	 ||
	 ((morpho_sets->morphoset = mw_change_morpho_set(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a morpho_sets\n");
	  return(NULL);
     }

     mw_copy_morpho_set(is,morpho_sets->morphoset);
     return(morpho_sets);
}

Morpho_set mw_morpho_sets_to_morpho_set(Morpho_sets morpho_sets, Morpho_set is)
{
     is = mw_change_morpho_set(is);
     if (is == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a morpho_set\n");
	  return(NULL);
     }

     if (morpho_sets->next) 
	  mwerror(WARNING,0,"Only the first morpho set is kept !\n");
     mw_copy_morpho_set(morpho_sets->morphoset,is);
     return(is);
}

/*========================= MORPHO_SETS / MIMAGE ===============================*/


Mimage mw_morpho_sets_to_mimage(Morpho_sets iss, Mimage mimage)
{
     if (((mimage = mw_change_mimage(mimage)) == NULL)
	 ||
	 ((mimage->first_ms = mw_change_morpho_sets(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a mimage\n");
	  return(NULL);
     }

     mw_copy_morpho_sets(iss,mimage->first_ms);
     return(mimage);
}

Morpho_sets mw_mimage_to_morpho_sets(Mimage mimage, Morpho_sets iss)
{
     if (!mimage->first_ms)
     {
	  mwerror(WARNING,0,"Conversion failed : input mimage does not contain any morpho_sets.\n");
	  return(NULL);
     }

     iss = mw_change_morpho_sets(iss);
     if (iss == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a morpho_sets\n");
	  return(NULL);
     }

     mw_copy_morpho_sets(mimage->first_ms,iss);
     return(iss);
}


/*========================= MORPHO_LINE / CMORPHO_LINE ===============================*/

/*--- Morpho_line to Cmorpho_line ---*/

Cmorpho_line mw_morpho_line_to_cmorpho_line(Morpho_line ll, Cmorpho_line cll)
{
     if ( ((cll = mw_change_cmorpho_line(cll)) == NULL) ||
	  ((cll->first_point = mw_new_point_curve()) == NULL) ||
	  ((cll->first_type = mw_new_point_type()) == NULL) )
     {
	  mwerror(ERROR, 0,"Not enough memory to create Cmorpho_line\n");
	  return(NULL);
     }
     cll->minvalue.model = MODEL_RGB;
     cll->minvalue.red = cll->minvalue.green = cll->minvalue.blue = ll->minvalue;
     cll->maxvalue.model = MODEL_RGB;
     cll->maxvalue.red = cll->maxvalue.green = cll->maxvalue.blue = ll->maxvalue;
     cll->open = ll->open;
     cll->data = ll->data;
     mw_copy_point_curve(ll->first_point,cll->first_point);
     mw_copy_point_type(ll->first_type,cll->first_type);
     return(cll);
}

/*--- Cmorpho_line to Morpho_line ---*/

Morpho_line mw_cmorpho_line_to_morpho_line(Cmorpho_line cll, Morpho_line ll)
{
     int clost;

     if ( ((ll = mw_change_morpho_line(ll)) == NULL) ||
	  ((ll->first_point = mw_new_point_curve()) == NULL) ||
	  ((ll->first_type = mw_new_point_type()) == NULL) )
     {
	  mwerror(ERROR, 0,"Not enough memory to create Morpho_line\n");
	  return(NULL);
     }

     clost=0;
     switch(cll->minvalue.model)
     {
     case MODEL_RGB:
	  if ((cll->minvalue.red != cll->minvalue.green) 
	      || (cll->minvalue.green != cll->minvalue.blue)) clost++;
	  ll->minvalue = FMONO(cll->minvalue.red,cll->minvalue.green,cll->minvalue.blue);
	  break;

     case MODEL_YUV:
	  if ((cll->minvalue.red == 0.0)||(cll->minvalue.blue == 0.0)) clost++;
	  ll->minvalue = cll->minvalue.green; /* Y in the Green channel */
	  break;

     case MODEL_HSI:
	  if (cll->minvalue.green == 0.0) clost++;
	  ll->minvalue = cll->minvalue.blue; /* Intensity in the Blue channel */
	  break;

     case MODEL_HSV:
	  if (cll->minvalue.green == 0.0) clost++;
	  ll->minvalue = cll->minvalue.blue; /* Value in the Blue channel */
	  break;

     default:
	  mwerror(ERROR, 0,"Unknown color model %d in Cmorpho_line\n",cll->minvalue.model);
	  return(NULL);
     }
     switch(cll->maxvalue.model)
     {
     case MODEL_RGB:
	  if ((cll->maxvalue.red != cll->maxvalue.green) 
	      || (cll->maxvalue.green != cll->maxvalue.blue)) clost++;
	  ll->maxvalue = FMONO(cll->maxvalue.red,cll->maxvalue.green,cll->maxvalue.blue);
	  break;

     case MODEL_YUV:
	  if ((cll->maxvalue.red == 0.0)||(cll->maxvalue.blue == 0.0)) clost++;
	  ll->maxvalue = cll->maxvalue.red; /* Y in the Green channel */
	  break;

     case MODEL_HSI:
	  if (cll->maxvalue.green == 0.0) clost++;
	  ll->maxvalue = cll->maxvalue.blue; /* Intensity in the Blue channel */
	  break;

     case MODEL_HSV:
	  if (cll->maxvalue.green == 0.0) clost++;
	  ll->maxvalue = cll->maxvalue.blue; /* Value in the Blue channel */
	  break;

     default:
	  mwerror(ERROR, 0,"Unknown color model %d in Cmorpho_line\n",cll->maxvalue.model);
	  return(NULL);
     }
     if ((clost != 0)&&(_mw_convert_struct_warning >= 0))
     {
	  mwerror(WARNING,0,"Color information has been lost !\n");
	  _mw_convert_struct_warning++;
     }      

     ll->open = cll->open;
     ll->data = cll->data;
     mw_copy_point_curve(cll->first_point,ll->first_point);
     mw_copy_point_type(cll->first_type,ll->first_type);

     return(ll);
}

/*========================= FMORPHO_LINE / CFMORPHO_LINE ===============================*/

/*--- Fmorpho_line to Cfmorpho_line ---*/

Cfmorpho_line mw_fmorpho_line_to_cfmorpho_line(Fmorpho_line ll, 
					       Cfmorpho_line cll)
{
     if ( ((cll = mw_change_cfmorpho_line(cll)) == NULL) ||
	  ((cll->first_point = mw_new_point_fcurve()) == NULL) ||
	  ((cll->first_type = mw_new_point_type()) == NULL) )
     {
	  mwerror(ERROR, 0,"Not enough memory to create Cfmorpho_line\n");
	  return(NULL);
     }
     cll->minvalue.model = MODEL_RGB;
     cll->minvalue.red = cll->minvalue.green = cll->minvalue.blue = ll->minvalue;
     cll->maxvalue.model = MODEL_RGB;
     cll->maxvalue.red = cll->maxvalue.green = cll->maxvalue.blue = ll->maxvalue;
     cll->open = ll->open;
     cll->data = ll->data;
     mw_copy_point_fcurve(ll->first_point,cll->first_point);
     mw_copy_point_type(ll->first_type,cll->first_type);

     return(cll);
}

/*--- Cfmorpho_line to Fmorpho_line ---*/

Fmorpho_line mw_cfmorpho_line_to_fmorpho_line(Cfmorpho_line cll, 
					      Fmorpho_line ll)
{
     int clost;

     if ( ((ll = mw_change_fmorpho_line(ll)) == NULL) ||
	  ((ll->first_point = mw_new_point_fcurve()) == NULL) ||
	  ((ll->first_type = mw_new_point_type()) == NULL) )
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fmorpho_line\n");
	  return(NULL);
     }

     clost=0;
     switch(cll->minvalue.model)
     {
     case MODEL_RGB:
	  if ((cll->minvalue.red != cll->minvalue.green) 
	      || (cll->minvalue.green != cll->minvalue.blue)) clost++;
	  ll->minvalue = FMONO(cll->minvalue.red,cll->minvalue.green,cll->minvalue.blue);
	  break;

     case MODEL_YUV:
	  if ((cll->minvalue.red == 0.0)||(cll->minvalue.blue == 0.0)) clost++;
	  ll->minvalue = cll->minvalue.green; /* Y in the Green channel */
	  break;

     case MODEL_HSI:
	  if (cll->minvalue.green == 0.0) clost++;
	  ll->minvalue = cll->minvalue.blue; /* Intensity in the Blue channel */
	  break;

     case MODEL_HSV:
	  if (cll->minvalue.green == 0.0) clost++;
	  ll->minvalue = cll->minvalue.blue; /* Value in the Blue channel */
	  break;

     default:
	  mwerror(ERROR, 0,"Unknown color model %d in Cfmorpho_line\n",cll->minvalue.model);
	  return(NULL);
     }
     switch(cll->maxvalue.model)
     {
     case MODEL_RGB:
	  if ((cll->maxvalue.red != cll->maxvalue.green) 
	      || (cll->maxvalue.green != cll->maxvalue.blue)) clost++;
	  ll->maxvalue = FMONO(cll->maxvalue.red,cll->maxvalue.green,cll->maxvalue.blue);
	  break;

     case MODEL_YUV:
	  if ((cll->maxvalue.red == 0.0)||(cll->maxvalue.blue == 0.0)) clost++;
	  ll->maxvalue = cll->maxvalue.red; /* Y in the Green channel */
	  break;

     case MODEL_HSI:
	  if (cll->maxvalue.green == 0.0) clost++;
	  ll->maxvalue = cll->maxvalue.blue; /* Intensity in the Blue channel */
	  break;

     case MODEL_HSV:
	  if (cll->maxvalue.green == 0.0) clost++;
	  ll->maxvalue = cll->maxvalue.blue; /* Value in the Blue channel */
	  break;

     default:
	  mwerror(ERROR, 0,"Unknown color model %d in Cfmorpho_line\n",cll->maxvalue.model);
	  return(NULL);
     }
     if ((clost != 0)&&(_mw_convert_struct_warning >= 0))
     {
	  mwerror(WARNING,0,"Color information has been lost !\n");
	  _mw_convert_struct_warning++;
     }      

     ll->open = cll->open;
     ll->data = cll->data;
     mw_copy_point_fcurve(cll->first_point,ll->first_point);
     mw_copy_point_type(cll->first_type,ll->first_type);

     return(ll);
}

/*========================= CMORPHO_LINE / CFMORPHO_LINE ===============================*/

/*--- Cmorpho_line to Cfmorpho_line ---*/

Cfmorpho_line mw_cmorpho_line_to_cfmorpho_line(Cmorpho_line ll, 
					       Cfmorpho_line fll)
{
     if ( ((fll = mw_change_cfmorpho_line(fll)) == NULL) ||
	  ((fll->first_type = mw_new_point_type()) == NULL) )
     {
	  mwerror(ERROR, 0,"Not enough memory to create Cfmorpho_line\n");
	  return(NULL);
     }
     fll->minvalue = ll->minvalue;
     fll->maxvalue = ll->maxvalue;
     fll->open = ll->open;
     fll->data = ll->data;
     fll->first_point = mw_point_curve_to_point_fcurve(ll->first_point,NULL);
     mw_copy_point_type(ll->first_type,fll->first_type);

     return(fll);
}

/*--- Cfmorpho_line to Cmorpho_line ---*/

Cmorpho_line mw_cfmorpho_line_to_cmorpho_line(Cfmorpho_line fll, 
					      Cmorpho_line ll)
{
     if ( ((ll = mw_change_cmorpho_line(ll)) == NULL) ||
	  ((ll->first_type = mw_new_point_type()) == NULL) )
     {
	  mwerror(ERROR, 0,"Not enough memory to create Cmorpho_line\n");
	  return(NULL);
     }
     ll->minvalue = fll->minvalue;
     ll->maxvalue = fll->maxvalue;
     ll->open = fll->open;
     ll->data = fll->data;
     ll->first_point = mw_point_fcurve_to_point_curve(fll->first_point,NULL);
     mw_copy_point_type(fll->first_type,ll->first_type);

     return(ll);
}


/*========================= CMORPHO_LINE / CMIMAGE ===============================*/


Cmimage mw_cmorpho_line_to_cmimage(Cmorpho_line ll, Cmimage cmimage)
{
     if (((cmimage = mw_change_cmimage(cmimage)) == NULL)
	 ||
	 ((cmimage->first_ml = mw_change_cmorpho_line(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a cmimage\n");
	  return(NULL);
     }

     mw_copy_cmorpho_line(ll,cmimage->first_ml);
     return(cmimage);
}

Cmorpho_line mw_cmimage_to_cmorpho_line(Cmimage cmimage, Cmorpho_line ll)
{
     if (!cmimage->first_ml)
     {
	  mwerror(WARNING,0,"Conversion failed : input dmimage does not contain any cmorpho_line.\n");
	  return(NULL);
     }

     ll = mw_change_cmorpho_line(NULL);
     if (ll == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a cmorpho_line\n");
	  return(NULL);
     }

     if (cmimage->first_ml->next) 
	  mwerror(WARNING,0,"Only the first cmorpho line is kept !\n");
     mw_copy_cmorpho_line(cmimage->first_ml,ll);
     return(ll);
}


/*========================= CMORPHO_LINE / CURVE ===============================*/

Cmorpho_line  mw_curve_to_cmorpho_line(Curve curve, Cmorpho_line ll)
{

     if (((ll = mw_change_cmorpho_line(ll)) == NULL)
	 ||
	 ((ll->first_point = mw_change_point_curve(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a cmorpho_line\n");
	  return(NULL);
     }

     mw_copy_point_curve(curve->first,ll->first_point);
     return(ll);
}

Curve mw_cmorpho_line_to_curve(Cmorpho_line ll, Curve cv)
{
     if (((cv = mw_change_curve(cv)) == NULL)
	 ||
	 ((cv->first = mw_change_point_curve(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a curve\n");
	  return(NULL);
     }
     mw_copy_point_curve(ll->first_point,cv->first);
     mwerror(WARNING,0,"Color of the cmorpho line have been lost !\n");
     if (ll->first_type)
	  mwerror(WARNING,0,"Point types of the cmorpho line have been lost !\n");

     return(cv);
}

/*========================= CFMORPHO_LINE / FCURVE ===============================*/

Cfmorpho_line  mw_fcurve_to_cfmorpho_line(Fcurve fcurve, Cfmorpho_line fll)
{
     if (((fll = mw_change_cfmorpho_line(fll)) == NULL)
	 ||
	 ((fll->first_point = mw_change_point_fcurve(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a cfmorpho_line\n");
	  return(NULL);
     }

     mw_copy_point_fcurve(fcurve->first,fll->first_point);
     return(fll);
}

Fcurve mw_cfmorpho_line_to_fcurve(Cfmorpho_line fll, Fcurve cv)
{
     if (((cv = mw_change_fcurve(cv)) == NULL)
	 ||
	 ((cv->first = mw_change_point_fcurve(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a fcurve\n");
	  return(NULL);
     }
     mw_copy_point_fcurve(fll->first_point,cv->first);
     mwerror(WARNING,0,"Gray level values of the cfmorpho line have been lost !\n");
     if (fll->first_type)
	  mwerror(WARNING,0,"Point types of the cfmorpho line have been lost !\n");

     return(cv);
}



/*========================= CURVES / CMIMAGE ===============================*/


Curves mw_cmimage_to_curves(Cmimage cmimage, Curves curves)
{
     Curve newcurve,oldcurve;
     Cmorpho_line ll;
     Cfmorpho_line fll;

     if (!cmimage->first_ml && !cmimage->first_fml)
     {
	  mwerror(WARNING,0,"Conversion failed : input cmimage does not contain any cmorpho_line or cfmorpho_line.\n");
	  return(NULL);
     }

     oldcurve = newcurve = NULL;
     curves = mw_change_curves(curves);
     if (curves == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Curves\n");
	  return(NULL);
     }

     if (cmimage->first_ml && cmimage->first_fml)
	  mwerror(WARNING,0,"The cmimage structure contains both cmorpho lines and cfmorpho lines. Using cmorpho lines only.\n");

     /* Use morpho lines */
     for (ll=cmimage->first_ml; ll; ll=ll->next)
     {
	  oldcurve = newcurve;
	  if ( ((newcurve = mw_new_curve()) == NULL)
	       || 
	       ((newcurve->first = mw_change_point_curve(NULL)) == NULL) )
	  {
	       mw_delete_curves(curves);
	       mwerror(ERROR, 0,"Not enough memory to create Curves\n");
	       return(NULL);	  
	  }
	  if (curves->first == NULL) curves->first = newcurve;
	  if (oldcurve != NULL) oldcurve->next = newcurve;
	  newcurve->previous = oldcurve;
	  newcurve->next = NULL;
	  mw_copy_point_curve(ll->first_point,newcurve->first);
     }
     if (cmimage->first_ml) return(curves);

     /* Use fmorpho lines */
     for (fll=cmimage->first_fml; fll; fll=fll->next)
     {
	  oldcurve = newcurve;
	  if ((newcurve = mw_new_curve()) == NULL)
	  {
	       mw_delete_curves(curves);
	       mwerror(ERROR, 0,"Not enough memory to create Curves\n");
	       return(NULL);	  
	  }
	  if (curves->first == NULL) curves->first = newcurve;
	  if (oldcurve != NULL) oldcurve->next = newcurve;
	  newcurve->previous = oldcurve;
	  newcurve->next = NULL;
	  newcurve->first = mw_point_fcurve_to_point_curve(fll->first_point,NULL);
     }

     return(curves);
}


Cmimage mw_curves_to_cmimage(Curves curves, Cmimage cmimage)
{
     Cmorpho_line newll,oldll;
     Curve cv;

     if ((cmimage = mw_change_cmimage(cmimage)) == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a cmimage\n");
	  return(NULL);
     }

     oldll = newll = NULL;
     for (cv=curves->first; cv; cv=cv->next)
     {
	  oldll = newll;
	  if ( ((newll = mw_new_cmorpho_line()) == NULL)
	       || 
	       ((newll->first_point = mw_change_point_curve(NULL)) == NULL) )
	  {
	       mw_delete_cmimage(cmimage);
	       mwerror(ERROR, 0,"Not enough memory to create a cmimage\n");
	       return(NULL);	  
	  }
	  if (cmimage->first_ml == NULL) cmimage->first_ml = newll;
	  if (oldll != NULL) oldll->next = newll;
	  newll->previous = oldll;
	  newll->next = NULL;
	  mw_copy_point_curve(cv->first,newll->first_point);
     }

     return(cmimage);
}

/*========================= FCURVES / CMIMAGE ===============================*/

Fcurves mw_cmimage_to_fcurves(Cmimage cmimage, Fcurves fcurves)
{
     Fcurve newfcurve,oldfcurve;
     Cfmorpho_line fll;
     Cmorpho_line ll;

     if (!cmimage->first_ml && !cmimage->first_fml)
     {
	  mwerror(WARNING,0,"Conversion failed : input cmimage does not contain any cmorpho_line or cfmorpho_line.\n");
	  return(NULL);
     }

     oldfcurve = newfcurve = NULL;
     fcurves = mw_change_fcurves(fcurves);
     if (fcurves == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	  return(NULL);
     }

     if (cmimage->first_ml && cmimage->first_fml)
	  mwerror(WARNING,0,"The cmimage structure contains both morpho lines and fmorpho lines. Using fmorpho lines only.\n");

     /* Use fmorpho lines */  
     for (fll=cmimage->first_fml; fll; fll=fll->next)
     {
	  oldfcurve = newfcurve;
	  if ( ((newfcurve = mw_new_fcurve()) == NULL)
	       || 
	       ((newfcurve->first = mw_change_point_fcurve(NULL)) == NULL) )
	  {
	       mw_delete_fcurves(fcurves);
	       mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	       return(NULL);	  
	  }
	  if (fcurves->first == NULL) fcurves->first = newfcurve;
	  if (oldfcurve != NULL) oldfcurve->next = newfcurve;
	  newfcurve->previous = oldfcurve;
	  newfcurve->next = NULL;
	  mw_copy_point_fcurve(fll->first_point,newfcurve->first);
     }
     if (cmimage->first_fml) return(fcurves);

     /* Use morpho lines */  
     for (ll=cmimage->first_ml; ll; ll=ll->next)
     {
	  oldfcurve = newfcurve;
	  if ((newfcurve = mw_new_fcurve()) == NULL)
	  {
	       mw_delete_fcurves(fcurves);
	       mwerror(ERROR, 0,"Not enough memory to create Fcurves\n");
	       return(NULL);	  
	  }
	  if (fcurves->first == NULL) fcurves->first = newfcurve;
	  if (oldfcurve != NULL) oldfcurve->next = newfcurve;
	  newfcurve->previous = oldfcurve;
	  newfcurve->next = NULL;
	  newfcurve->first = mw_point_curve_to_point_fcurve(ll->first_point,NULL);
     }

     return(fcurves);
}

Cmimage mw_fcurves_to_cmimage(Fcurves fcurves, Cmimage cmimage)
{
     Cfmorpho_line newfll,oldfll;
     Fcurve cv;

     if ((cmimage = mw_change_cmimage(cmimage)) == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a cmimage\n");
	  return(NULL);
     }

     oldfll = newfll = NULL;
     for (cv=fcurves->first; cv; cv=cv->next)
     {
	  oldfll = newfll;
	  if ( ((newfll = mw_new_cfmorpho_line()) == NULL)
	       || 
	       ((newfll->first_point = mw_change_point_fcurve(NULL)) == NULL) )
	  {
	       mw_delete_cmimage(cmimage);
	       mwerror(ERROR, 0,"Not enough memory to create a cmimage\n");
	       return(NULL);	  
	  }
	  if (cmimage->first_fml == NULL) cmimage->first_fml = newfll;
	  if (oldfll != NULL) oldfll->next = newfll;
	  newfll->previous = oldfll;
	  newfll->next = NULL;
	  mw_copy_point_fcurve(cv->first,newfll->first_point);
     }

     return(cmimage);
}

/*========================= CMORPHO_SETS / CMIMAGE ===============================*/


Cmimage mw_cmorpho_sets_to_cmimage(Cmorpho_sets iss, Cmimage cmimage)
{
     if (((cmimage = mw_change_cmimage(cmimage)) == NULL)
	 ||
	 ((cmimage->first_ms = mw_change_cmorpho_sets(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a cmimage\n");
	  return(NULL);
     }

     mw_copy_cmorpho_sets(iss,cmimage->first_ms);
     return(cmimage);
}

Cmorpho_sets mw_cmimage_to_cmorpho_sets(Cmimage cmimage, Cmorpho_sets iss)
{
     if (!cmimage->first_ms)
     {
	  mwerror(WARNING,0,"Conversion failed : input cmimage does not contain any morpho_sets.\n");
	  return(NULL);
     }

     iss = mw_change_cmorpho_sets(iss);
     if (iss == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a cmorpho_sets\n");
	  return(NULL);
     }

     mw_copy_cmorpho_sets(cmimage->first_ms,iss);
     return(iss);
}

/*========================= CMORPHO_SET / CMORPHO_SETS ===============================*/


Cmorpho_sets mw_cmorpho_set_to_cmorpho_sets(Cmorpho_set is, 
					    Cmorpho_sets cmorpho_sets)
{
     if (((cmorpho_sets = mw_change_cmorpho_sets(cmorpho_sets)) == NULL)
	 ||
	 ((cmorpho_sets->cmorphoset = mw_change_cmorpho_set(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a cmorpho_sets\n");
	  return(NULL);
     }

     mw_copy_cmorpho_set(is,cmorpho_sets->cmorphoset);
     return(cmorpho_sets);
}

Cmorpho_set mw_cmorpho_sets_to_cmorpho_set(Cmorpho_sets cmorpho_sets, 
					   Cmorpho_set is)
{
     is = mw_change_cmorpho_set(is);
     if (is == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a cmorpho_set\n");
	  return(NULL);
     }

     if (cmorpho_sets->next) 
	  mwerror(WARNING,0,"Only the first cmorpho set is kept !\n");
     mw_copy_cmorpho_set(cmorpho_sets->cmorphoset,is);
     return(is);
}


/*========================= CFMORPHO_LINE / CMIMAGE ===============================*/


Cmimage mw_cfmorpho_line_to_cmimage(Cfmorpho_line fll, Cmimage cmimage)
{
     if (((cmimage = mw_change_cmimage(cmimage)) == NULL)
	 ||
	 ((cmimage->first_fml = mw_change_cfmorpho_line(NULL)) == NULL))
     {
	  mwerror(ERROR, 0,"Not enough memory to create a cmimage\n");
	  return(NULL);
     }

     mw_copy_cfmorpho_line(fll,cmimage->first_fml);
     return(cmimage);
}

Cfmorpho_line mw_cmimage_to_cfmorpho_line(Cmimage cmimage, Cfmorpho_line fll)
{
     if (!cmimage->first_fml)
     {
	  mwerror(WARNING,0,"Conversion failed : input cmimage does not contain any cfmorpho_line.\n");
	  return(NULL);
     }

     fll = mw_change_cfmorpho_line(fll);
     if (fll == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a cfmorpho_line\n");
	  return(NULL);
     }

     if (cmimage->first_fml->next) 
	  mwerror(WARNING,0,"Only the first cfmorpho line is kept !\n");
     mw_copy_cfmorpho_line(cmimage->first_fml,fll);
     return(fll);
}

/*========================= MIMAGE / CMIMAGE ===============================*/


Cmimage mw_mimage_to_cmimage(Mimage mimage, Cmimage cmimage)
{
     Morpho_line ml;
     Cmorpho_line cml0,cml1;
     Fmorpho_line fml;
     Cfmorpho_line cfml0,cfml1;
/*     Morpho_sets ms;         */
/*     Cmorpho_sets cms0,cms1; */

     if ((cmimage = mw_change_cmimage(cmimage)) == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a cmimage\n");
	  return(NULL);
     }
  
     strcpy(cmimage->cmt,mimage->cmt);  
     strcpy(cmimage->name,mimage->name);  
     cmimage->nrow = mimage->nrow;
     cmimage->ncol = mimage->ncol;  
     cmimage->minvalue.model = MODEL_RGB;
     cmimage->minvalue.red = cmimage->minvalue.green = cmimage->minvalue.blue = mimage->minvalue;
     cmimage->maxvalue.model = MODEL_RGB;
     cmimage->maxvalue.red = cmimage->maxvalue.green = cmimage->maxvalue.blue = mimage->maxvalue;

     /* Convert Morpho_lines */
     cml0=NULL;
     for (ml=mimage->first_ml; ml; ml=ml->next)
     {
	  cml1 = mw_morpho_line_to_cmorpho_line(ml,NULL);
	  if (cml1==NULL) return(NULL);
	  if (cml0 != NULL) 
	  { cml1->previous=cml0; cml0->next=cml1; }
	  else 
	       cmimage->first_ml = cml1;
	  cml0=cml1;
     }

     /* Convert Fmorpho_lines */
     cfml0=NULL;
     for (fml=mimage->first_fml; fml; fml=fml->next)
     {
	  cfml1 = mw_fmorpho_line_to_cfmorpho_line(fml,NULL);
	  if (cfml1==NULL) return(NULL);
	  if (cfml0 != NULL) 
	  { cfml1->previous=cfml0; cfml0->next=cfml1; }
	  else 
	       cmimage->first_fml = cfml1;
	  cfml0=cfml1;
     }
  
     /* Convert Morpho_sets */
     /*  mw_morpho_set_to_cmorpho_set() NOT IMPLEMENTED ! 
	 cms0=NULL;
	 for (ms=mimage->first_ms; ms; ms=ms->next)
	 {
	 cms1 = mw_morpho_set_to_cmorpho_set(ms);
	 if (cms1==NULL) return(NULL);
	 if (cms0 != NULL) 
	 { cms1->previous=cms0; cms0->next=cms1; }
	 else 
	 cmimage->first_ms = cms1;
	 cms0=cms1;
	 }  
     */
     return(cmimage);
}

Mimage mw_cmimage_to_mimage(Cmimage cmimage, Mimage mimage)
{
     Morpho_line ml0,ml1;
     Cmorpho_line cml;
     Fmorpho_line fml0,fml1;
     Cfmorpho_line cfml;
/*     Morpho_sets ms0,ms1; */
/*     Cmorpho_sets cms;    */
     int clost;

     if ((mimage = mw_change_mimage(mimage)) == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to create a mimage\n");
	  return(NULL);
     }
  
     strcpy(mimage->cmt,cmimage->cmt);  
     strcpy(mimage->name,cmimage->name);  
     mimage->nrow = cmimage->nrow;
     mimage->ncol = cmimage->ncol;  
     clost=0;
     switch(cmimage->minvalue.model)
     {
     case MODEL_RGB:
	  if ((cmimage->minvalue.red != cmimage->minvalue.green) 
	      || (cmimage->minvalue.green != cmimage->minvalue.blue)) clost++;
	  mimage->minvalue = FMONO(cmimage->minvalue.red,cmimage->minvalue.green,cmimage->minvalue.blue);
	  break;

     case MODEL_YUV:
	  if ((cmimage->minvalue.red == 0.0)||(cmimage->minvalue.blue == 0.0)) clost++;
	  mimage->minvalue = cmimage->minvalue.green; /* Y in the Green channel */
	  break;

     case MODEL_HSI:
	  if (cmimage->minvalue.green == 0.0) clost++;
	  mimage->minvalue = cmimage->minvalue.blue; /* Intensity in the Blue channel */
	  break;

     case MODEL_HSV:
	  if (cmimage->minvalue.green == 0.0) clost++;
	  mimage->minvalue = cmimage->minvalue.blue; /* Value in the Blue channel */
	  break;

     default:
	  mwerror(ERROR, 0,"Unknown color model %d in Cfmorpho_line\n",cmimage->minvalue.model);
	  return(NULL);
     }
     switch(cmimage->maxvalue.model)
     {
     case MODEL_RGB:
	  if ((cmimage->maxvalue.red != cmimage->maxvalue.green) 
	      || (cmimage->maxvalue.green != cmimage->maxvalue.blue)) clost++;
	  mimage->maxvalue = FMONO(cmimage->maxvalue.red,cmimage->maxvalue.green,cmimage->maxvalue.blue);
	  break;

     case MODEL_YUV:
	  if ((cmimage->maxvalue.red == 0.0)||(cmimage->maxvalue.blue == 0.0)) clost++;
	  mimage->maxvalue = cmimage->maxvalue.red; /* Y in the Green channel */
	  break;

     case MODEL_HSI:
	  if (cmimage->maxvalue.green == 0.0) clost++;
	  mimage->maxvalue = cmimage->maxvalue.blue; /* Intensity in the Blue channel */
	  break;

     case MODEL_HSV:
	  if (cmimage->maxvalue.green == 0.0) clost++;
	  mimage->maxvalue = cmimage->maxvalue.blue; /* Value in the Blue channel */
	  break;

     default:
	  mwerror(ERROR, 0,"Unknown color model %d in Cfmorpho_line\n",cmimage->maxvalue.model);
	  return(NULL);
     }
     if ((clost != 0)&&(_mw_convert_struct_warning >= 0))
     {
	  mwerror(WARNING,0,"Color information has been lost !\n");
	  _mw_convert_struct_warning++;
     }      

     /* Convert Morpho_lines */
     ml0=NULL;
     for (cml=cmimage->first_ml; cml; cml=cml->next)
     {
	  ml1 = mw_cmorpho_line_to_morpho_line(cml,NULL);
	  if (ml1==NULL) return(NULL);
	  if (ml0 != NULL) 
	  { ml1->previous=ml0; ml0->next=ml1; }
	  else 
	       mimage->first_ml = ml1;
	  ml0=ml1;
     }

     /* Convert Fmorpho_lines */
     fml0=NULL;
     for (cfml=cmimage->first_fml; cfml; cfml=cfml->next)
     {
	  fml1 = mw_cfmorpho_line_to_fmorpho_line(cfml,NULL);
	  if (fml1==NULL) return(NULL);
	  if (fml0 != NULL) 
	  { fml1->previous=fml0; fml0->next=fml1; }
	  else 
	       mimage->first_fml = fml1;
	  fml0=fml1;
     }
  
     /* Convert Morpho_sets */
     /*  mw_cmorpho_set_to_morpho_set() NOT IMPLEMENTED ! 
	 ms0=NULL;
	 for (cms=cmimage->first_ms; cms; cms=cms->next)
	 {
	 ms1 = mw_cmorpho_set_to_morpho_set(cms);
	 if (ms1==NULL) return(NULL);
	 if (ms0 != NULL) 
	 { ms1->previous=ms0; ms0->next=ms1; }
	 else 
	 mimage->first_ms = ms1;
	 ms0=ms1;
	 }  
     */
     return(mimage);
}

/*========================= FLIST(S) ================================*/

Flist mw_flists_to_flist(Flists ls, Flist l)
{
     if (!ls) 
     {
	  mwerror(ERROR,0,"[mw_flists_to_flist] Flists structure is NULL\n");
	  return(NULL);
     }
     if (ls->size <= 0) return(NULL);  /* Empty Flists */
     l=mw_copy_flist(ls->list[0],l);
     return(l);
}

Flists mw_flist_to_flists(Flist l, Flists ls)
{
     if (!l)  /* Empty input : create empty Flists */
	  ls = mw_new_flists();
     else 
     {
	  ls = mw_change_flists(ls,1,1);
	  ls->list[0]=mw_copy_flist(l,ls->list[0]);
     }
     return(ls);
}


/* In the following original version by L. Moisan */

/* Fcurve -> Flist conversion 
   do not change dim if already set larger than 2 (x y ...) */

Flist mw_fcurve_to_flist(Fcurve c, Flist l)
{
     int i,size;
     Point_fcurve p;
     
     if (!c) {
	  mwerror(ERROR,0,"[mw_fcurve_to_flist] Fcurve structure is NULL\n");
	  return(NULL);
     }

     size = mw_length_fcurve(c);

     if (l && l->dim>=2) 
	  l = mw_change_flist(l,size,size,l->dim);
     else l = mw_change_flist(l,size,size,2);
     for (i=0,p=c->first;p;p=p->next,i++) {
	  l->values[i*l->dim  ] = p->x;
	  l->values[i*l->dim+1] = p->y;
     }

     return(l);
}


/* Flist -> Fcurve conversion */

Fcurve mw_flist_to_fcurve(Flist l, Fcurve c)
{
     int i;
     Point_fcurve p,*next,prev;

     if (!l) {
	  mwerror(ERROR,0,"[mw_flist_to_fcurve] flist structure is NULL\n");
	  return(NULL);
     }

     c = mw_change_fcurve(c);
     prev = NULL;
     next = &(c->first);

     for (i=0;i<l->size;i++) {
	  p = mw_new_point_fcurve();
	  p->x = l->values[i*l->dim  ];
	  p->y = l->values[i*l->dim+1];
	  p->previous = prev;
	  prev = *next = p;
	  next = &(p->next);
     }
     *next = NULL;

     return(c);
}


/* Fcurves -> Flists conversion */

Flists mw_fcurves_to_flists(Fcurves cs, Flists ls)
{
     int i,size;
     Fcurve c;
     
     if (!cs) {
	  mwerror(ERROR,0,"[mw_fcurves_to_flists] Fcurves structure is NULL\n");
	  return(NULL);
     }

     size = mw_length_fcurves(cs);

     ls = mw_change_flists(ls,size,size);
  
     for (i=0,c=cs->first;c;c=c->next,i++) 
	  ls->list[i] = mw_fcurve_to_flist(c,ls->list[i]);

     return(ls);
}


/* Flists -> Fcurves conversion */

Fcurves mw_flists_to_fcurves(Flists ls, Fcurves cs)
{
     int i;
     Fcurve c,*next,prev;

     if (!ls) {
	  mwerror(ERROR,0,"[mw_flists_to_fcurves] flists structure is NULL\n");
	  return(NULL);
     }

     cs = mw_change_fcurves(cs);
     prev = NULL;
     next = &(cs->first);

     for (i=0;i<ls->size;i++) {
	  c = mw_flist_to_fcurve(ls->list[i],NULL);
	  c->previous = prev;
	  prev = *next = c;
	  next = &(c->next);
     }
     *next = NULL;

     return(cs);
}


/*========================= DLIST(S) ================================*/

Dlist mw_dlists_to_dlist(Dlists ls, Dlist l)
{
     if (!ls) 
     {
	  mwerror(ERROR,0,"[mw_dlists_to_dlist] Dlists structure is NULL\n");
	  return(NULL);
     }
     if (ls->size <= 0) return(NULL);  /* Empty Dlists */
     l=mw_copy_dlist(ls->list[0],l);
     return(l);
}

Dlists mw_dlist_to_dlists(Dlist l, Dlists ls)
{
     if (!l)  /* Empty input : create empty Dlists */
	  ls = mw_new_dlists();
     else 
     {
	  ls = mw_change_dlists(ls,1,1);
	  ls->list[0]=mw_copy_dlist(l,ls->list[0]);
     }
     return(ls);
}

/* In the following original version by L. Moisan */

/* Dcurve -> Dlist conversion 
   do not change dim if already set larger than 2 (x y ...) */

Dlist mw_dcurve_to_dlist(Dcurve c, Dlist l)
{
     int i,size;
     Point_dcurve p;
     
     if (!c) {
	  mwerror(ERROR,0,"[mw_dcurve_to_dlist] Dcurve structure is NULL\n");
	  return(NULL);
     }

     size = mw_length_dcurve(c);

     if (l && l->dim>=2) 
	  l = mw_change_dlist(l,size,size,l->dim);
     else l = mw_change_dlist(l,size,size,2);
     for (i=0,p=c->first;p;p=p->next,i++) {
	  l->values[i*l->dim  ] = p->x;
	  l->values[i*l->dim+1] = p->y;
     }

     return(l);
}


/* Dlist -> Dcurve conversion */

Dcurve mw_dlist_to_dcurve(Dlist l, Dcurve c)
{
     int i;
     Point_dcurve p,*next,prev;

     if (!l) {
	  mwerror(ERROR,0,"[mw_dlist_to_dcurve] dlist structure is NULL\n");
	  return(NULL);
     }

     c = mw_change_dcurve(c);
     prev = NULL;
     next = &(c->first);

     for (i=0;i<l->size;i++) {
	  p = mw_new_point_dcurve();
	  p->x = l->values[i*l->dim  ];
	  p->y = l->values[i*l->dim+1];
	  p->previous = prev;
	  prev = *next = p;
	  next = &(p->next);
     }
     *next = NULL;

     return(c);
}


/* Dcurves -> Dlists conversion */

Dlists mw_dcurves_to_dlists(Dcurves cs, Dlists ls)
{
     int i,size;
     Dcurve c;
     
     if (!cs) {
	  mwerror(ERROR,0,"[mw_dcurves_to_dlists] Dcurves structure is NULL\n");
	  return(NULL);
     }

     size = mw_length_dcurves(cs);

     ls = mw_change_dlists(ls,size,size);
  
     for (i=0,c=cs->first;c;c=c->next,i++) 
	  ls->list[i] = mw_dcurve_to_dlist(c,ls->list[i]);

     return(ls);
}


/* Dlists -> Dcurves conversion */

Dcurves mw_dlists_to_dcurves(Dlists ls, Dcurves cs)
{
     int i;
     Dcurve c,*next,prev;

     if (!ls) {
	  mwerror(ERROR,0,"[mw_dlists_to_dcurves] dlists structure is NULL\n");
	  return(NULL);
     }

     cs = mw_change_dcurves(cs);
     prev = NULL;
     next = &(cs->first);

     for (i=0;i<ls->size;i++) {
	  c = mw_dlist_to_dcurve(ls->list[i],NULL);
	  c->previous = prev;
	  prev = *next = c;
	  next = &(c->next);
     }
     *next = NULL;

     return(cs);
}


/*======================== FLIST(S) / DLIST(S) ==============================*/

/* In the following original version by L. Moisan */
/* 01/2002 added copy of data field (LM) */

/* Flist -> Dlist conversion */

Dlist mw_flist_to_dlist(Flist in, Dlist out)
{
     int n;

     if (!in) {
	  mwerror(ERROR,0,"[mw_flist_to_dlist] input is NULL\n");
	  return(NULL);
     }
     out = mw_change_dlist(out,in->size,in->size,in->dim);
     for (n=in->size*in->dim;n--;)
	  out->values[n] = (double)in->values[n];
     if (in->data_size && in->data) {
	  if (out->data_size<in->data_size) {
	       out->data = realloc(out->data,in->data_size);
	       if (!out->data) {
		    mwerror(ERROR,0,"[mw_flist_to_dlist] Not enough memory\n");
		    return(out);
	       }
	  }
	  memcpy(out->data,in->data,in->data_size);
	  out->data_size = in->data_size;
     }
     return(out);
}

/* Dlist -> Flist conversion */

Flist mw_dlist_to_flist(Dlist in, Flist out)
{
     int n;

     if (!in) {
	  mwerror(ERROR,0,"[mw_dlist_to_flist] input is NULL\n");
	  return(NULL);
     }
     out = mw_change_flist(out,in->size,in->size,in->dim);
     for (n=in->size*in->dim;n--;)
	  out->values[n] = (float)in->values[n];
     if (in->data_size && in->data) {
	  if (out->data_size<in->data_size) {
	       out->data = realloc(out->data,in->data_size);
	       if (!out->data) {
		    mwerror(ERROR,0,"[mw_dlist_to_flist] Not enough memory\n");
		    return(out);
	       }
	  }
	  memcpy(out->data,in->data,in->data_size);
	  out->data_size = in->data_size;
     }

     return(out);
}

/* Flists -> Dlists conversion */

Dlists mw_flists_to_dlists(Flists in, Dlists out)
{
     int n;

     if (!in) {
	  mwerror(ERROR,0,"[mw_flists_to_dlists] input is NULL\n");
	  return(NULL);
     }
     out = mw_change_dlists(out,in->size,in->size);
     for (n=in->size;n--;)
	  out->list[n] = mw_flist_to_dlist(in->list[n],out->list[n]);
     if (in->data_size && in->data) {
	  if (out->data_size<in->data_size) {
	       out->data = realloc(out->data,in->data_size);
	       if (!out->data) {
		    mwerror(ERROR,0,"[mw_flists_to_dlists] Not enough memory\n");
		    return(out);
	       }
	  }
	  memcpy(out->data,in->data,in->data_size);
	  out->data_size = in->data_size;
     }

     return(out);
}

/* Dlists -> Flists conversion */

Flists mw_dlists_to_flists(Dlists in, Flists out)
{
     int n;

     if (!in) {
	  mwerror(ERROR,0,"[mw_dlists_to_flists] input is NULL\n");
	  return(NULL);
     }
     out = mw_change_flists(out,in->size,in->size);
     for (n=in->size;n--;)
	  out->list[n] = mw_dlist_to_flist(in->list[n],out->list[n]);
     if (in->data_size && in->data) {
	  if (out->data_size<in->data_size) {
	       out->data = realloc(out->data,in->data_size);
	       if (!out->data) {
		    mwerror(ERROR,0,"[mw_dlists_to_flists] Not enough memory\n");
		    return(out);
	       }
	  }
	  memcpy(out->data,in->data,in->data_size);
	  out->data_size = in->data_size;
     }

     return(out);
}
