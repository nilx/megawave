/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   curve_io.c
   
   Vers. 1.8
   (C) 1993-97 Jacques Froment
   Input/Output functions for the curve & curves structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef Linux
#include <unistd.h>
#endif

#include "mw.h"

/* This value of int type reserved */
/*
  Old value to large on Linux :#define END_OF_CURVE -2147483648
*/
#define END_OF_CURVE -2147483647

/* ---- I/O for Curve ---- */

/* Load curve from a file of MW2_CURVE format */

Curve _mw_load_curve_mw2_curve(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  Curve cv;
  Point_curve newcvc,oldcvc;
  struct stat buf;
  int fsize,buf_size,i;
  char header[10];
  int vx,vy,*buffer;
  register int *ptr;
  char ftype[TYPE_SIZE],mtype[TYPE_SIZE];
  int need_flipping;

  need_flipping = _mw_get_file_type(fname,ftype,mtype)-1;
  
  if ( (need_flipping==-1) ||
       (!(fp = fopen(fname, "r"))) || (fstat(fileno(fp),&buf) != 0) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }

  fsize = buf.st_size-9; /* Size of the file - size of the header, in bytes */
                          /* header = "MW2_CURVE" */
  buf_size = (fsize / sizeof(int));

  if ((buf_size % 2) != 0)
	{
	  mwerror(ERROR, 0,"Error into the file \"%s\": not an even number of coordinates !\n",fname);
	  return(NULL);
	}
  
  if (!(buffer = (int *) malloc(buf_size * sizeof(int))))
    {
      mwerror(ERROR, 0,"Not enough memory to load file \"%s\" (size of file = %d bytes)\n",fname,buf.st_size);
      fclose(fp);
      return(NULL);
    }

  if ((fread(header,9,1,fp) == 0) || (fread(buffer,fsize,1,fp) == 0))
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	free(buffer);
	fclose(fp);
	return(NULL);
      }

  fclose(fp);

  if (strcmp(ftype,"MW2_CURVE") != 0)
    mwerror(INTERNAL, 0,"[_mw_load_curve_mw2_curve] File \"%s\" is not in the MW2_CURVE format\n",fname);

  cv = mw_new_curve();
  if (cv == NULL) return(cv);
  oldcvc = newcvc = NULL;

  for (ptr=buffer, i=1; i < buf_size; i+=2)
    {
      if (need_flipping==1)
	{	
	  vx = _mw_get_flip_b4(*ptr);
	  ptr++;
	  vy = _mw_get_flip_b4(*ptr);
	  ptr++;
	  /*printf("(flip) i=%d (%d,%d)\n",i,vx,vy);*/
	}
      else
	{	
	  vx = *ptr++;
	  vy = *ptr++;
	  /*printf("(no flip) i=%d (%d,%d)\n",i,vx,vy);*/
	}

      newcvc = mw_new_point_curve();
      if (newcvc == NULL)
	    {
	      mw_delete_curve(cv);
	      free(buffer);
	      return(NULL);
	    }
	  if (cv->first == NULL) cv->first = newcvc;
	  if (oldcvc != NULL) oldcvc->next = newcvc;
	  newcvc->previous = oldcvc;
	  newcvc->next = NULL;
	  newcvc->x = vx; newcvc->y = vy;
	  oldcvc = newcvc;
    }
  free(buffer);
  return(cv);
}

/* Load curve from file of different types */

Curve _mw_load_curve(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Polygon poly;
  Fpolygon fpoly;
  Curve curve;
  Morpho_line ll;
  Fmorpho_line fll;
  Mimage mimage;
  Fcurve fcurve;
  Curves curves;
  Fcurves fcurves;

#ifdef __STDC__
  Curves _mw_load_curves_mw2_curves(char *);
#else  
  Curves _mw_load_curves_mw2_curves();
#endif

  _mw_get_file_type(fname,Type,mtype);
  
  if (strcmp(Type,"MW2_CURVE") == 0)
    return(_mw_load_curve_mw2_curve(fname));

  if (strcmp(Type,"MW2_FCURVE") == 0)
    {
      fcurve = (Fcurve) _mw_load_fcurve_mw2_fcurve(fname);
      curve = (Curve) mw_fcurve_to_curve(fcurve);
      mw_delete_fcurve(fcurve);
      return(curve);
    }

  if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      ll = (Morpho_line) _mw_load_ml_mw2_ml(fname);
      curve = (Curve) mw_morpho_line_to_curve(ll);
      mw_delete_morpho_line(ll);
      return(curve);
    }


  if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    {
      fll = (Fmorpho_line) _mw_load_fml_mw2_fml(fname);
      fcurve = (Fcurve) mw_fmorpho_line_to_fcurve(fll);
      curve = (Curve) mw_fcurve_to_curve(fcurve);
      mw_delete_fcurve(fcurve);
      mw_delete_fmorpho_line(fll);
      return(curve);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) _mw_load_mimage_mw2_mimage(fname);
      ll = (Morpho_line) mw_mimage_to_morpho_line(mimage);
      curve = (Curve) mw_morpho_line_to_curve(ll);
      mw_delete_morpho_line(ll);
      mw_delete_mimage(mimage);
      return(curve);
    }

  if (strcmp(Type,"A_POLY") == 0)
    {
      poly = (Polygon) _mw_load_polygon_a_poly(fname);
      curve = (Curve) mw_polygon_to_curve(poly);
      mw_delete_polygon(poly);
      return(curve);
    }

  if (strcmp(Type,"A_FPOLY") == 0)
    {
      fpoly = (Fpolygon) _mw_load_fpolygon_a_fpoly(fname);
      curve = (Curve) mw_fpolygon_to_curve(fpoly);
      mw_delete_fpolygon(fpoly);      
      return(curve);
    }

  if (strcmp(Type,"MW2_CURVES") == 0)
    {
      curves = (Curves) _mw_load_curves_mw2_curves(fname);
      curve = (Curve) mw_curves_to_curve(curves);
      mw_delete_curves(curves);
      return(curve);
    }

  if (strcmp(Type,"MW2_FCURVES") == 0)
    {
      fcurves = (Fcurves) _mw_load_fcurves_mw2_fcurves(fname);
      curves = (Curves) mw_fcurves_to_curves(fcurves);
      curve = (Curve) mw_curves_to_curve(curves);
      mw_delete_curves(curves);
      mw_delete_fcurves(fcurves);
      return(curve);
    }


  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}  


/* Write file in MW2_CURVE format */  

short _mw_create_curve_mw2_curve(fname,cv)

char  *fname;                        /* file name */
Curve cv;

{
  FILE *fp;
  Point_curve pc;
  int n;

  if (cv == NULL)
    mwerror(INTERNAL,1,"[_mw_create_curve_mw2_curve] Cannot create file: Curve structure is NULL\n");

  fp=_mw_write_header_file(fname,"MW2_CURVE");
  if (fp == NULL) return(-1);

  if (cv->first == NULL)
    mwerror(INTERNAL,1,"[_mw_create_curve_mw2_curve] Curve has no point curve !\n");
  
  for (pc=cv->first; pc; pc=pc->next)
    {
      fwrite(&(pc->x),sizeof(int),1,fp);
      fwrite(&(pc->y),sizeof(int),1,fp);
    }

  fclose(fp);
  return(0);
}

/* Write file in different formats */
   
short _mw_create_curve(fname,cv,Type)

char  *fname;                        /* file name */
Curve cv;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Polygon poly;
  Fpolygon fpoly;
  Fcurve fcurve;
  Curves curves;
  Fcurves fcurves;
  Morpho_line ll;
  Fmorpho_line fll;
  Mimage mimage;

#ifdef __STDC__
  short _mw_create_curves_mw2_curves(char *, Curves);
#else
  short _mw_create_curves_mw2_curves();
#endif

  if (strcmp(Type,"MW2_CURVE") == 0)
    return(_mw_create_curve_mw2_curve(fname,cv));

  if (strcmp(Type,"MW2_FCURVE") == 0)
    {
      fcurve = (Fcurve) mw_curve_to_fcurve(cv);
      ret = _mw_create_fcurve_mw2_fcurve(fname,fcurve);
      mw_delete_fcurve(fcurve);
      return(ret);
    }

  if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      ll = (Morpho_line) mw_curve_to_morpho_line(cv);
      ret = _mw_create_ml_mw2_ml(fname,ll);
      mw_delete_morpho_line(ll);
      return(ret);
    }

  if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    {
      fcurve = (Fcurve) mw_curve_to_fcurve(cv);      
      fll = (Fmorpho_line) mw_fcurve_to_fmorpho_line(fcurve);
      ret = _mw_create_fml_mw2_fml(fname,fll);
      mw_delete_fmorpho_line(fll);
      mw_delete_fcurve(fcurve);
      return(ret);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      ll = (Morpho_line) mw_curve_to_morpho_line(cv);
      mimage = (Mimage) mw_morpho_line_to_mimage(ll);
      ret = _mw_create_mimage_mw2_mimage(fname,mimage);
      mw_delete_mimage(mimage);
      mw_delete_morpho_line(ll);
      return(ret);
    }

  if (strcmp(Type,"A_POLY") == 0)
    {
      poly = (Polygon) mw_curve_to_polygon(cv);
      ret = _mw_create_polygon_a_poly(fname,poly);
      mw_delete_polygon(poly);
      return(ret);
    }

  if (strcmp(Type,"A_FPOLY") == 0)
    {
      fpoly = (Fpolygon) mw_curve_to_fpolygon(cv);
      ret = _mw_create_fpolygon_a_fpoly(fname,fpoly);
      mw_delete_fpolygon(fpoly);
      return(ret);
    }

  if (strcmp(Type,"MW2_CURVES") == 0)
    {
      curves = (Curves) mw_curve_to_curves(cv);
      ret = _mw_create_curves_mw2_curves(fname,curves);
      mw_delete_curves(curves);
      return(ret);
    }

  if (strcmp(Type,"MW2_FCURVES") == 0)
    {
      curves = (Curves) mw_curve_to_curves(cv);
      fcurves = (Fcurves) mw_curves_to_fcurves(curves);
      ret = _mw_create_fcurves_mw2_fcurves(fname,fcurves);
      mw_delete_fcurves(fcurves);
      mw_delete_curves(curves);
      return(ret);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}


/* ---- I/O for Curves ---- */

/* Load curves from a file of MW2_CURVES format */

Curves _mw_load_curves_mw2_curves(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  Curves cvs;
  Curve newcv,oldcv;
  Point_curve newcvc,oldcvc;
  struct stat buf;
  int fsize,buf_size,i;
  char header[11];
  int vx,vy,*buffer;
  register int *ptr;
  char new_curve;
  char ftype[TYPE_SIZE],mtype[TYPE_SIZE];
  int need_flipping;

  need_flipping = _mw_get_file_type(fname,ftype,mtype)-1;
  
  if ( (need_flipping==-1) ||
       (!(fp = fopen(fname, "r"))) || (fstat(fileno(fp),&buf) != 0) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }

  fsize = buf.st_size-10; /* Size of the file - size of the header, in bytes */
                          /* header = "MW2_CURVES" */
  buf_size = (fsize / sizeof(int));

  if (!(buffer = (int *) malloc(buf_size * sizeof(int))))
    {
      mwerror(ERROR, 0,"Not enough memory to load file \"%s\" (size of file = %d bytes)\n",fname,buf.st_size);
      fclose(fp);
      return(NULL);
    }

  if ((fread(header,10,1,fp) == 0) || (fread(buffer,fsize,1,fp) == 0))
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	free(buffer);
	fclose(fp);
	return(NULL);
      }
  fclose(fp);

  if (strcmp(ftype,"MW2_CURVES") != 0)
    mwerror(INTERNAL, 0,"[_mw_load_curves_mw2_curves] File \"%s\" is not in the MW2_CURVES format\n",fname);
  
  cvs = mw_new_curves();
  if (cvs == NULL) return(cvs);
  oldcv = newcv = NULL;

  new_curve = 1;
  for (ptr=buffer, i=1; i < buf_size; i+=2)
    {
      
      if (new_curve == 1)  /* Begin a new curve */
	{
	  oldcv = newcv;
	  newcv = mw_new_curve();
	  if (newcv == NULL)
	    {
	      mw_delete_curves(cvs);
	      free(buffer);
	      return(NULL);
	    }
	  if (cvs->first == NULL) cvs->first = newcv;
	  if (oldcv != NULL) oldcv->next = newcv;
	  newcv->previous = oldcv;
	  newcv->next = NULL;
	  newcv->first = NULL;
	  oldcvc = newcvc = NULL;
	}

      if (need_flipping==1)
	{	
	  vx = _mw_get_flip_b4(*ptr);
	  ptr++;
	  vy = _mw_get_flip_b4(*ptr);
	  ptr++;
	  /*printf("(flip) i=%d (%d,%d)\n",i,vx,vy);*/
	}
      else
	{	
	  vx = *ptr++;
	  vy = *ptr++;
	  /*printf("(no flip) i=%d (%d,%d)\n",i,vx,vy);*/
	}

      if ((vy == END_OF_CURVE) && (i < buf_size-1))
	{
	  mwerror(ERROR, 0,"Error into the file \"%s\"...\n",fname);
	  mw_delete_curves(cvs);
	  free(buffer);
	  return(NULL);
	}
      
      if (vx == END_OF_CURVE) { new_curve=1; ptr--; i--; } /* Mark end of curve */
      else
	{
	  new_curve = 0;
	  newcvc = mw_new_point_curve();
	  if (newcvc == NULL)
	    {
	      mw_delete_curves(cvs);
	      free(buffer);
	      return(NULL);
	    }
	  if (newcv->first == NULL) newcv->first = newcvc;
	  if (oldcvc != NULL) oldcvc->next = newcvc;
	  newcvc->previous = oldcvc;
	  newcvc->next = NULL;
	  newcvc->x = vx; newcvc->y = vy;
	  oldcvc = newcvc;
	}
    }      
  free(buffer);
  return(cvs);
}

/* Load curves from file of different types */

Curves _mw_load_curves(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Polygons polys;
  Fpolygons fpolys;
  Curves curves;
  Fcurves fcurves;
  Curve curve;
  Fcurve fcurve;
  Morpho_line ll;
  Fmorpho_line fll;
  Mimage mimage;

  _mw_get_file_type(fname,Type,mtype);
  
  if (strcmp(Type,"MW2_CURVES") == 0)
    return(_mw_load_curves_mw2_curves(fname));

  if (strcmp(Type,"MW2_FCURVES") == 0)
    {
      fcurves = (Fcurves) _mw_load_fcurves_mw2_fcurves(fname);
      curves = (Curves) mw_fcurves_to_curves(fcurves);
      mw_delete_fcurves(fcurves);
      return(curves);
    }

  if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      ll = (Morpho_line) _mw_load_ml_mw2_ml(fname);
      curve = (Curve) mw_morpho_line_to_curve(ll);
      curves = (Curves) mw_curve_to_curves(curve);
      mw_delete_curve(curve);
      mw_delete_morpho_line(ll);
      return(curves);
    }

  if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    {
      fll = (Fmorpho_line) _mw_load_fml_mw2_fml(fname);
      fcurve = (Fcurve) mw_fmorpho_line_to_fcurve(fll);
      curve = (Curve) mw_fcurve_to_curve(fcurve);
      curves = (Curves) mw_curve_to_curves(curve);
      mw_delete_curve(curve);
      mw_delete_fcurve(fcurve);
      mw_delete_fmorpho_line(fll);
      return(curves);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) _mw_load_mimage_mw2_mimage(fname);
      curves = (Curves) mw_mimage_to_curves(mimage);
      mw_delete_mimage(mimage);
      return(curves);
    }

  if (strcmp(Type,"A_POLY") == 0)
    {
      polys = (Polygons) _mw_load_polygons_a_poly(fname);
      curves = (Curves) mw_polygons_to_curves(polys);
      mw_delete_polygons(polys);
      return(curves);
    }

  if (strcmp(Type,"A_FPOLY") == 0)
    {
      fpolys = (Fpolygons) _mw_load_fpolygons_a_fpoly(fname);
      curves = (Curves) mw_fpolygons_to_curves(fpolys);
      mw_delete_fpolygons(fpolys);      
      return(curves);
    }

  if (strcmp(Type,"MW2_CURVE") == 0)
    {
      curve = (Curve) _mw_load_curve_mw2_curve(fname);
      curves = (Curves) mw_curve_to_curves(curve);
      mw_delete_curve(curve);
      return(curves);
    }

  if (strcmp(Type,"MW2_FCURVE") == 0)
    {
      fcurve = (Fcurve) _mw_load_fcurve_mw2_fcurve(fname);
      curve = (Curve) mw_fcurve_to_curve(fcurve);
      curves = (Curves) mw_curve_to_curves(curve);
      mw_delete_curve(curve);
      mw_delete_fcurve(fcurve);
      return(curves);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}  

/* Write file in MW2_CURVES format */  

short _mw_create_curves_mw2_curves(fname,cvs)

char  *fname;                        /* file name */
Curves cvs;

{
  FILE *fp;
  Curve pl;
  Point_curve pc;
  int n;
  int end_of_curve = END_OF_CURVE;

  if (cvs == NULL)
    mwerror(INTERNAL,1,"[_mw_create_curves_mw2_curves] Cannot create file: Curves structure is NULL\n");

  if (cvs->first == NULL)
    mwerror(INTERNAL,1,
	      "[_mw_create_curves_mw2_curves] Cannot create file: No curve in the Curves structure\n");

  fp=_mw_write_header_file(fname,"MW2_CURVES");
  if (fp == NULL) return(-1);

  for (pl=cvs->first, n=1; pl; pl=pl->next, n++)
    {
      if (pl->first == NULL)
	mwerror(INTERNAL,1,"[_mw_create_curves_mw2_curves] Curve #%d has no point curve\n",n);
      for (pc=pl->first; pc; pc=pc->next)
	{
	  if ((pc->x == END_OF_CURVE) ||
	      (pc->y == END_OF_CURVE))
	    mwerror(INTERNAL,1,"[_mw_create_curves_mw2_curves] Curve #%d has a point which coordinates (%d,%d) has reserved value. Sorry !\n",n,pc->x,pc->y);	    
	  fwrite(&(pc->x),sizeof(int),1,fp);
	  fwrite(&(pc->y),sizeof(int),1,fp);
	}
      fwrite(&end_of_curve,sizeof(int),1,fp);
    }      
  fwrite(&end_of_curve,sizeof(int),1,fp);
  fclose(fp);
  return(0);
}
   
/* Write file in different formats */
   
short _mw_create_curves(fname,cvs,Type)

char  *fname;                        /* file name */
Curves cvs;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Polygons polys;
  Fpolygons fpolys;
  Fcurves fcurves;
  Curve curve;
  Fcurve fcurve;
  Morpho_line ll;
  Fmorpho_line fll;
  Mimage mimage;

  if (strcmp(Type,"MW2_CURVES") == 0)
    return(_mw_create_curves_mw2_curves(fname,cvs));

  if (strcmp(Type,"MW2_FCURVES") == 0)
    {
      fcurves = (Fcurves) mw_curves_to_fcurves(cvs);
      ret = _mw_create_fcurves_mw2_fcurves(fname,fcurves);
      mw_delete_fcurves(fcurves);
      return(ret);
    }

  if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      curve = (Curve) mw_curves_to_curve(cvs);
      ll = (Morpho_line) mw_curve_to_morpho_line(curve);
      ret = _mw_create_ml_mw2_ml(fname,ll);
      mw_delete_morpho_line(ll);
      mw_delete_curve(curve);
      return(ret);
    }

  if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    {
      fcurves = (Fcurves) mw_curves_to_fcurves(cvs);
      fcurve = (Fcurve) mw_fcurves_to_fcurve(fcurves);
      fll = (Fmorpho_line) mw_fcurve_to_fmorpho_line(fcurve);
      ret = _mw_create_fml_mw2_fml(fname,fll);
      mw_delete_fmorpho_line(fll);
      mw_delete_fcurve(fcurve);
      mw_delete_fcurves(fcurves);
      return(ret);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) mw_curves_to_mimage(cvs);
      ret = _mw_create_mimage_mw2_mimage(fname,mimage);
      mw_delete_mimage(mimage);
      return(ret);
    }

  if (strcmp(Type,"A_POLY") == 0)
    {
      polys = (Polygons) mw_curves_to_polygons(cvs);
      ret = _mw_create_polygons_a_poly(fname,polys);
      mw_delete_polygons(polys);
      return(ret);
    }

  if (strcmp(Type,"A_FPOLY") == 0)
    {
      fpolys = (Fpolygons) mw_curves_to_fpolygons(cvs);
      ret = _mw_create_fpolygons_a_fpoly(fname,fpolys);
      mw_delete_fpolygons(fpolys);
      return(ret);
    }

  if (strcmp(Type,"MW2_CURVE") == 0)
    {
      curve = (Curve) mw_curves_to_curve(cvs);
      ret = _mw_create_curve_mw2_curve(fname,curve);
      mw_delete_curve(curve);
      return(ret);
    }

  if (strcmp(Type,"MW2_FCURVE") == 0)
    {
      curve = (Curve) mw_curves_to_curve(cvs);
      fcurve = (Fcurve) mw_curve_to_fcurve(curve);
      ret = _mw_create_fcurve_mw2_fcurve(fname,fcurve);
      mw_delete_fcurve(fcurve);
      mw_delete_curve(curve);
      return(ret);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}




