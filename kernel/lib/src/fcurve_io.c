/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   fcurve_io.c
   
   Vers. 1.8
   (C) 1995-99 Jacques Froment
   Input/Output functions for the fcurve & fcurves structure

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
#include <math.h>
#ifdef Linux
#include <unistd.h>
#endif

#include "mw.h"

/* Marker to end a curve */
#define END_MARKER 1e+37
/* Threshold above which the value is said to be END_MARKER */
#define END_MARKER_THRESHOLD 1e+36

/* ---- I/O for Fcurve ---- */

/* Load fcurve from a file of MW2_FCURVE format */

Fcurve _mw_load_fcurve_mw2_fcurve(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  Fcurve cv;
  Point_fcurve newcvc,oldcvc;
  struct stat buf;
  int fsize,buf_size,i;
  char header[11];
  float vx,vy,*buffer;
  register float *ptr;
  char ftype[TYPE_SIZE],mtype[TYPE_SIZE];
  int need_flipping;
  unsigned long * flip_float; /* buffer for macro _mw_in_flip_float */
  
  need_flipping = _mw_get_file_type(fname,ftype,mtype)-1;
  
  if ( (need_flipping==-1) ||
      (!(fp = fopen(fname, "r"))) || (fstat(fileno(fp),&buf) != 0) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }

  fsize = buf.st_size-10; /* Size of the file - size of the header, in bytes */
                          /* header = "MW2_FCURVE" */
  buf_size = fsize / sizeof(float); /* Number of coordinates */

  if ((buf_size % 2) != 0)
	{
	  mwerror(ERROR, 0,"Error into the file \"%s\": not an even number of coordinates !\n",fname);
	  return(NULL);
	}
  
  if (!(buffer = (float *) malloc(fsize)))
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

  if (strcmp(ftype,"MW2_FCURVE") != 0)
    mwerror(INTERNAL, 0,"[_mw_load_fcurve_mw2_fcurve] File \"%s\" is not in the MW2_FCURVE format\n",fname);

  cv = mw_new_fcurve();
  if (cv == NULL) return(cv);
  oldcvc = newcvc = NULL;


  for (ptr=buffer, i=1; i < buf_size; i+=2)
    {
      if (need_flipping==1)
	{	
	  _mw_in_flip_float(ptr);
	  vx=*ptr;
	  ptr++;
	  _mw_in_flip_float(ptr);
	  vy=*ptr;
	  ptr++;
	  /* printf("(flip) i=%d (%f,%f)\n",i,vx,vy); */
	}
      else
	{	
	  vx = *ptr++;
	  vy = *ptr++;
	  /* printf("(no flip) i=%d (%f,%f)\n",i,vx,vy); */
	}
      newcvc = mw_new_point_fcurve();
      if (newcvc == NULL)
	    {
	      mw_delete_fcurve(cv);
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

/* Load fcurve from file of different types */

Fcurve _mw_load_fcurve(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Polygon poly;
  Fpolygon fpoly;
  Curve curve;  
  Curves curves;  
  Fcurve fcurve;
  Fcurves fcurves;
  Morpho_line ll;
  Fmorpho_line fll;
  Mimage mimage;

#ifdef __STDC__
  Fcurves _mw_load_fcurves_mw2_fcurves(char *);
#else  
  Fcurves _mw_load_fcurves_mw2_fcurves();
#endif

  _mw_get_file_type(fname,Type,mtype);
  
  if (strcmp(Type,"MW2_FCURVE") == 0)
    return(_mw_load_fcurve_mw2_fcurve(fname));

  if (strcmp(Type,"MW2_CURVE") == 0)
    {
      curve = (Curve) _mw_load_curve_mw2_curve(fname);
      fcurve = (Fcurve) mw_curve_to_fcurve(curve);
      mw_delete_curve(curve);
      return(fcurve);
    }

  if (strcmp(Type,"A_POLY") == 0)
    {
      poly = (Polygon) _mw_load_polygon_a_poly(fname);
      fcurve = (Fcurve) mw_polygon_to_fcurve(poly);
      mw_delete_polygon(poly);
      return(fcurve);
    }

  if (strcmp(Type,"A_FPOLY") == 0)
    {
      fpoly = (Fpolygon) _mw_load_fpolygon_a_fpoly(fname);
      fcurve = (Fcurve) mw_fpolygon_to_fcurve(fpoly);
      mw_delete_fpolygon(fpoly);      
      return(fcurve);
    }

  if (strcmp(Type,"MW2_FCURVES") == 0)
    {
      fcurves = (Fcurves) _mw_load_fcurves_mw2_fcurves(fname);
      fcurve = (Fcurve) mw_fcurves_to_fcurve(fcurves);
      mw_delete_fcurves(fcurves);
      return(fcurve);
    }

  if (strcmp(Type,"MW2_CURVES") == 0)
    {
      curves = (Curves) _mw_load_curves_mw2_curves(fname);
      curve = (Curve) mw_curves_to_curve(curves);
      fcurve = (Fcurve) mw_curve_to_fcurve(curve);      
      mw_delete_curve(curve);
      mw_delete_curves(curves);
      return(fcurve);
    }

 if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      ll = (Morpho_line) _mw_load_ml_mw2_ml(fname);
      curve = (Curve) mw_morpho_line_to_curve(ll);
      fcurve = (Fcurve) mw_curve_to_fcurve(curve);      
      mw_delete_curve(curve);
      mw_delete_morpho_line(ll);
      return(fcurve);
    }

  if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    {
      fll = (Fmorpho_line) _mw_load_fml_mw2_fml(fname);
      fcurve = (Fcurve) mw_fmorpho_line_to_fcurve(fll);
      mw_delete_fmorpho_line(fll);
      return(fcurve);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) _mw_load_mimage_mw2_mimage(fname);
      fll = (Fmorpho_line) mw_mimage_to_fmorpho_line(mimage);
      fcurve = (Fcurve) mw_fmorpho_line_to_fcurve(fll);
      mw_delete_fmorpho_line(fll);
      mw_delete_mimage(mimage);
      return(fcurve);
    }

 mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}  

/* Write file in MW2_FCURVE format */  

short _mw_create_fcurve_mw2_fcurve(fname,cv)

char  *fname;                        /* file name */
Fcurve cv;

{
  FILE *fp;
  Point_fcurve pc;
  int n;
  float vx=0.0,vy=0.0;

  if (cv == NULL)
    mwerror(INTERNAL,1,"[_mw_create_fcurve_mw2_fcurve] Cannot create file: Fcurve structure is NULL\n");

  fp=_mw_write_header_file(fname,"MW2_FCURVE");
  if (fp == NULL) return(-1);
  
  if (cv->first == NULL)
    mwerror(INTERNAL,1,"[_mw_create_fcurve_mw2_fcurve] Fcurve has no point fcurve !\n");
  
  for (pc=cv->first; pc; pc=pc->next)
    {
      vx = pc->x;
      vy = pc->y;
      fwrite(&vx,sizeof(float),1,fp);
      fwrite(&vy,sizeof(float),1,fp);
    }

  fclose(fp);
  return(0);
}
   
/* Write file in different formats */
   
short _mw_create_fcurve(fname,cv,Type)

char  *fname;                        /* file name */
Fcurve cv;
char  *Type;                         /* Type de format du fichier */

{  
  short ret;
  Polygon poly;
  Fpolygon fpoly;
  Curve curve;
  Curves curves;
  Fcurves fcurves;
  Morpho_line ll;
  Fmorpho_line fll;
  Mimage mimage;

#ifdef __STDC__
  short _mw_create_fcurves_mw2_fcurves(char *, Fcurves);
#else
  short _mw_create_fcurves_mw2_fcurves();
#endif

  if (strcmp(Type,"MW2_FCURVE") == 0)
    return(_mw_create_fcurve_mw2_fcurve(fname,cv));

  if (strcmp(Type,"MW2_CURVE") == 0)
    {
      curve = (Curve) mw_fcurve_to_curve(cv);
      ret = _mw_create_curve_mw2_curve(fname,curve);
      mw_delete_curve(curve);
      return(ret);
    }

  if (strcmp(Type,"A_POLY") == 0)
    {
      poly = (Polygon) mw_fcurve_to_polygon(cv);
      ret = _mw_create_polygon_a_poly(fname,poly);
      mw_delete_polygon(poly);
      return(ret);
    }

  if (strcmp(Type,"A_FPOLY") == 0)
    {
      fpoly = (Fpolygon) mw_fcurve_to_fpolygon(cv);
      ret = _mw_create_fpolygon_a_fpoly(fname,fpoly);
      mw_delete_fpolygon(fpoly);
      return(ret);
    }

  if (strcmp(Type,"MW2_FCURVES") == 0)
    {
      fcurves = (Fcurves) mw_fcurve_to_fcurves(cv);
      ret = _mw_create_fcurves_mw2_fcurves(fname,fcurves);
      mw_delete_fcurves(fcurves);
      return(ret);
    }

  if (strcmp(Type,"MW2_CURVES") == 0)
    {
      fcurves = (Fcurves) mw_fcurve_to_fcurves(cv);
      curves = (Curves) mw_fcurves_to_curves(fcurves);
      ret = _mw_create_curves_mw2_curves(fname,curves);
      mw_delete_curves(curves);
      mw_delete_fcurves(fcurves);
      return(ret);
    }

  if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      curve = (Curve) mw_fcurve_to_curve(cv);
      ll = (Morpho_line) mw_curve_to_morpho_line(curve);
      ret = _mw_create_ml_mw2_ml(fname,ll);
      mw_delete_morpho_line(ll);
      mw_delete_curve(curve);
      return(ret);
    }

  if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    {
      fll = (Fmorpho_line) mw_fcurve_to_fmorpho_line(cv);
      ret = _mw_create_fml_mw2_fml(fname,fll);
      mw_delete_fmorpho_line(fll);
      return(ret);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      fll = (Fmorpho_line) mw_fcurve_to_fmorpho_line(cv);
      mimage = (Mimage) mw_fmorpho_line_to_mimage(fll);
      ret = _mw_create_mimage_mw2_mimage(fname,mimage);
      mw_delete_mimage(mimage);
      mw_delete_fmorpho_line(fll);
      return(ret);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  

}

/* ---- I/O for Fcurves ---- */

/* Load fcurves from a file of MW2_CURVES format */

Fcurves _mw_load_fcurves_mw2_fcurves(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  Fcurves cvs;
  Fcurve newcv,oldcv;
  Point_fcurve newcvc,oldcvc;
  struct stat buf;
  int fsize,buf_size,i;
  char header[12];
  float vx,vy,*buffer;
  register float *ptr;
  char new_fcurve;
  char ftype[TYPE_SIZE],mtype[TYPE_SIZE];
  int need_flipping;
  unsigned long * flip_float; /* buffer for macro _mw_in_flip_float */

  need_flipping = _mw_get_file_type(fname,ftype,mtype)-1;
  
  if ( (need_flipping==-1) ||
      (!(fp = fopen(fname, "r"))) || (fstat(fileno(fp),&buf) != 0) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }

  fsize = buf.st_size-11; /* Size of the file - size of the header, in bytes */
                          /* header = "MW2_FCURVES" */
  buf_size = fsize / sizeof(float); /* Number of coordinates */

  if ((sizeof(float)*buf_size) != fsize)
    {
      mwerror(ERROR, 0,"File \"%s\" seems to be corrupted\n",fname);
      fclose(fp);
      return(NULL);
    }

  if (!(buffer = (float *) malloc(fsize)))
    {
      mwerror(ERROR, 0,"Not enough memory to load file \"%s\" (size of file = %d bytes)\n",fname,buf.st_size);
      fclose(fp);
      return(NULL);
    }

  if ((fread(header,11,1,fp) == 0) || 
      (fread(buffer,sizeof(float),buf_size,fp) != buf_size))
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	free(buffer);
	fclose(fp);
	return(NULL);
      }

  fclose(fp);

  if (strcmp(ftype,"MW2_FCURVES") != 0)
    mwerror(INTERNAL, 0,"[_mw_load_fcurves_mw2_fcurves] File \"%s\" is not in the MW2_FCURVES format\n",fname);

  cvs = mw_new_fcurves();
  if (cvs == NULL) return(cvs);
  oldcv = newcv = NULL;

  new_fcurve = 1;
  for (ptr=buffer, i=1; i < buf_size; i+=2)
    {
              
      if (new_fcurve == 1)  /* Begin a new fcurve */
	{
	  oldcv = newcv;
	  newcv = mw_new_fcurve();
	  if (newcv == NULL)
	    {
	      mw_delete_fcurves(cvs);
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
	  _mw_in_flip_float(ptr);
	  vx=*ptr;
	  ptr++;
	  _mw_in_flip_float(ptr);
	  vy=*ptr;
	  ptr++;
	  /*printf("(flip) i=%d (%f,%f)\n",i,vx,vy);*/
	}
      else
	{	
	  vx = *ptr++;
	  vy = *ptr++;
	  /*printf("(no flip) i=%d (%f,%f)\n",i,vx,vy);*/
	}

      if ((vy >= END_MARKER_THRESHOLD) && (i < buf_size-1))
	{
	  mwerror(ERROR, 0,"Error into the file \"%s\"...\n",fname);
	  mw_delete_fcurves(cvs);
	  free(buffer);
	  return(NULL);
	}
      
      if (vx >= END_MARKER_THRESHOLD) 
	{ 
	  new_fcurve=1; ptr--; i--; 
	} /* Mark end of fcurve */
      else
	{
	  new_fcurve = 0;
	  newcvc = mw_new_point_fcurve();
	  if (newcvc == NULL)
	    {
	      mw_delete_fcurves(cvs);
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

/* Load fcurves from file of different types */

Fcurves _mw_load_fcurves(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Polygons polys;
  Fpolygons fpolys;
  Curves curves;
  Curve curve;
  Fcurves fcurves;
  Fcurve fcurve;
  Morpho_line ll;
  Fmorpho_line fll;
  Mimage mimage;

  _mw_get_file_type(fname,Type,mtype);
  
  if (strcmp(Type,"MW2_FCURVES") == 0)
    return(_mw_load_fcurves_mw2_fcurves(fname));

  if (strcmp(Type,"MW2_CURVES") == 0)
    {
      curves = (Curves) _mw_load_curves_mw2_curves(fname);
      fcurves = (Fcurves) mw_curves_to_fcurves(curves);
      mw_delete_curves(curves);
      return(fcurves);
    }

  if (strcmp(Type,"A_POLY") == 0)
    {
      polys = (Polygons) _mw_load_polygons_a_poly(fname);
      fcurves = (Fcurves) mw_polygons_to_fcurves(polys);
      mw_delete_polygons(polys);
      return(fcurves);
    }

  if (strcmp(Type,"A_FPOLY") == 0)
    {
      fpolys = (Fpolygons) _mw_load_fpolygons_a_fpoly(fname);
      fcurves = (Fcurves) mw_fpolygons_to_fcurves(fpolys);
      mw_delete_fpolygons(fpolys);      
      return(fcurves);
    }

  if (strcmp(Type,"MW2_FCURVE") == 0)
    {
      fcurve = (Fcurve) _mw_load_fcurve_mw2_fcurve(fname);
      fcurves = (Fcurves) mw_fcurve_to_fcurves(fcurve);
      mw_delete_fcurve(fcurve);
      return(fcurves);
    }

  if (strcmp(Type,"MW2_CURVE") == 0)
    {
      curve = (Curve) _mw_load_curve_mw2_curve(fname);
      curves = (Curves) mw_curve_to_curves(curve);
      fcurves = (Fcurves) mw_curves_to_fcurves(curves);      
      mw_delete_curves(curves);
      mw_delete_curve(curve);
      return(fcurves);
    }

  if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      ll = (Morpho_line) _mw_load_ml_mw2_ml(fname);
      curve = (Curve) mw_morpho_line_to_curve(ll);
      curves = (Curves) mw_curve_to_curves(curve);
      fcurves = (Fcurves) mw_curves_to_fcurves(curves);      
      mw_delete_curves(curves);
      mw_delete_curve(curve);
      mw_delete_morpho_line(ll);
      return(fcurves);
    }

   if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    {
      fll = (Fmorpho_line) _mw_load_fml_mw2_fml(fname);
      fcurve = (Fcurve) mw_fmorpho_line_to_fcurve(fll);
      fcurves = (Fcurves) mw_fcurve_to_fcurves(fcurve);
      mw_delete_fcurve(fcurve);
      mw_delete_fmorpho_line(fll);
      return(fcurves);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) _mw_load_mimage_mw2_mimage(fname);
      fcurves = (Fcurves) mw_mimage_to_fcurves(mimage);
      mw_delete_mimage(mimage);
      return(fcurves);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}  

/* Write file in MW2_FCURVES format */  

short _mw_create_fcurves_mw2_fcurves(fname,cvs)

char  *fname;                        /* file name */
Fcurves cvs;

{
  FILE *fp;
  Fcurve pl;
  Point_fcurve pc;
  int n;
  float vx=0.0,vy=0.0,end_of_fcurve=END_MARKER;

  if (cvs == NULL)
    mwerror(INTERNAL,1,"[_mw_create_fcurves_mw2_fcurves] Cannot create file: Fcurves structure is NULL\n");

  if (cvs->first == NULL)
    mwerror(INTERNAL,1,
	      "[_mw_create_fcurves_mw2_fcurves] Cannot create file: No fcurve in the Fcurves structure\n");

  fp=_mw_write_header_file(fname,"MW2_FCURVES");
  if (fp == NULL) return(-1);
  
  for (pl=cvs->first, n=1; pl; pl=pl->next, n++)
    {
      if (pl->first == NULL)
	mwerror(INTERNAL,1,"[_mw_create_fcurves_mw2_fcurves] Fcurve #%d has no point fcurve\n",n);
      for (pc=pl->first; pc; pc=pc->next)
	{
	  if ((pc->x >= END_MARKER_THRESHOLD) ||
	      (pc->y >= END_MARKER_THRESHOLD))
	    mwerror(INTERNAL,1,"[_mw_create_fcurves_mw2_fcurves] Fcurve #%d has a point which coordinates (%f,%f) exceed float capacity\n",n,pc->x,pc->y);	    
	  vx = pc->x;
	  vy = pc->y;
	  fwrite(&vx,sizeof(float),1,fp);
	  fwrite(&vy,sizeof(float),1,fp);
	}
      fwrite(&end_of_fcurve,sizeof(float),1,fp);
    }      
  fwrite(&end_of_fcurve,sizeof(float),1,fp);
  fclose(fp);
  return(0);
}

/* Write file in different formats */
   
short _mw_create_fcurves(fname,cvs,Type)

char  *fname;                        /* file name */
Fcurves cvs;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Polygons polys;
  Fpolygons fpolys;
  Curves curves;
  Curve curve;
  Fcurve fcurve;
  Morpho_line ll;
  Fmorpho_line fll;
  Mimage mimage;

  if (strcmp(Type,"MW2_FCURVES") == 0)
    return(_mw_create_fcurves_mw2_fcurves(fname,cvs));

  if (strcmp(Type,"MW2_CURVES") == 0)
    {
      curves = (Curves) mw_fcurves_to_curves(cvs);
      ret = _mw_create_curves_mw2_curves(fname,curves);
      mw_delete_curves(curves);
      return(ret);
    }

  if (strcmp(Type,"A_POLY") == 0)
    {
      polys = (Polygons) mw_fcurves_to_polygons(cvs);
      ret = _mw_create_polygons_a_poly(fname,polys);
      mw_delete_polygons(polys);
      return(ret);
    }

  if (strcmp(Type,"A_FPOLY") == 0)
    {
      fpolys = (Fpolygons) mw_fcurves_to_fpolygons(cvs);
      ret = _mw_create_fpolygons_a_fpoly(fname,fpolys);
      mw_delete_fpolygons(fpolys);
      return(ret);
    }

  if (strcmp(Type,"MW2_FCURVE") == 0)
    {
      fcurve = (Fcurve) mw_fcurves_to_fcurve(cvs);
      ret = _mw_create_fcurve_mw2_fcurve(fname,fcurve);
      mw_delete_fcurve(fcurve);
      return(ret);
    }

  if (strcmp(Type,"MW2_CURVE") == 0)
    {
      fcurve = (Fcurve) mw_fcurves_to_fcurve(cvs);
      curve = (Curve) mw_fcurve_to_curve(fcurve);
      ret = _mw_create_curve_mw2_curve(fname,curve);
      mw_delete_curve(curve);
      mw_delete_fcurve(fcurve);
      return(ret);
    }

  if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      curves = (Curves) mw_fcurves_to_curves(cvs);
      curve = (Curve) mw_curves_to_curve(curves);
      ll = (Morpho_line) mw_curve_to_morpho_line(curve);
      ret = _mw_create_ml_mw2_ml(fname,ll);
      mw_delete_morpho_line(ll);
      mw_delete_curve(curve);
      mw_delete_curves(curves);
      return(ret);
    }

   if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    {
      fcurve = (Fcurve) mw_fcurves_to_fcurve(cvs);
      fll = (Fmorpho_line) mw_fcurve_to_fmorpho_line(fcurve);
      ret = _mw_create_fml_mw2_fml(fname,fll);
      mw_delete_fmorpho_line(fll);
      mw_delete_fcurve(fcurve);
      return(ret);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) mw_fcurves_to_mimage(cvs);
      ret = _mw_create_mimage_mw2_mimage(fname,mimage);
      mw_delete_mimage(mimage);
      return(ret);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}








