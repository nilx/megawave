/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   fpolygon_io.c
   
   Vers. 1.3
   (C) 1995-96 Jacques Froment
   Input/Output functions for the fpolygon & fpolygons structures

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

#include "ascii_file.h"
#include "mw.h"


/* ---- I/O for Fpolygon ---- */

Fpolygon _mw_load_fpolygon_a_fpoly(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  Fpolygon fpoly;
  Point_fcurve newpc,oldpc;
  float px,py;
  int i,nc;
  char channel[20];

  fp = _mw_open_data_ascii_file(fname);
  if (fp == NULL) return(NULL);

  if (_mw_fascii_search_string(fp,"def Fpolygon\n") == EOF)
    {
      mwerror(ERROR, 0,
	      "No Fpolygon description found in the file \"%s\"",fname);
      fclose(fp);
      return(NULL);
    }

  fpoly = mw_new_fpolygon();
  if (fpoly == NULL) return(fpoly);

  if ((_mw_fascii_get_field(fp,fname,"nb_channels:","%d",&nc) != 1)
      || (nc <= 0))
    {
      mw_delete_fpolygon(fpoly);
      fclose(fp);
      return(NULL);
    }
     
  fpoly = mw_change_fpolygon(fpoly,nc);
  if (fpoly == NULL)
    {
      mw_delete_fpolygon(fpoly);
      fclose(fp);
      return(NULL);
    }

  for (i=0;i<nc;i++)
    {
      sprintf(channel,"channel #%d:",i+1);
      if (_mw_fascii_get_field(fp,fname,channel,"%f",&fpoly->channel[i]) != 1) 
	{
	  mw_delete_fpolygon(fpoly);
	  fclose(fp);
	  return(NULL);
	}
    }

  oldpc = NULL;
  while (fscanf(fp,"%f,%f\n",&px,&py) ==  2)
    {
      newpc = mw_new_point_fcurve();
      if (newpc == NULL)
	{
	  mw_delete_fpolygon(fpoly);
	  fclose(fp);
	  return(NULL);
	}
      if (fpoly->first == NULL) fpoly->first = newpc;
      if (oldpc != NULL) oldpc->next = newpc;
      newpc->previous = oldpc;
      newpc->next = NULL;
      newpc->x = px; newpc->y = py;
      oldpc = newpc;
    }

  fclose(fp);
  return(fpoly);
}

/* Load fpolygon from file of different types */

Fpolygon _mw_load_fpolygon(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Polygon poly;
  Fpolygon fpoly;
  Curve curve;
  Fcurve fcurve;
  Morpho_line ll;
  Mimage mimage;

  _mw_get_file_type(fname,Type,mtype);
  
  if (strcmp(Type,"A_FPOLY") == 0)
    return((Fpolygon) _mw_load_fpolygon_a_fpoly(fname));

  if (strcmp(Type,"MW2_CURVE") == 0)
    {
      curve = (Curve) _mw_load_curve_mw2_curve(fname);
      fpoly = (Fpolygon) mw_curve_to_fpolygon(curve);
      mw_delete_curve(curve);
      return(fpoly);
    }

  if (strcmp(Type,"MW2_FCURVE") == 0)
    {
      fcurve = (Fcurve) _mw_load_fcurve_mw2_fcurve(fname);
      fpoly = (Fpolygon) mw_fcurve_to_fpolygon(fcurve);
      mw_delete_fcurve(fcurve);
      return(fpoly);
    }

  if (strcmp(Type,"A_POLY") == 0)
    {
      poly = (Polygon) _mw_load_polygon_a_poly(fname);
      fpoly = (Fpolygon) mw_polygon_to_fpolygon(poly);
      mw_delete_polygon(poly);      
      return(fpoly);
    }

 if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      ll = (Morpho_line) _mw_load_ml_mw2_ml(fname);
      curve = (Curve) mw_morpho_line_to_curve(ll);
      fpoly = (Fpolygon) mw_curve_to_fpolygon(curve);
      mw_delete_curve(curve);
      mw_delete_morpho_line(ll);
      return(fpoly);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) _mw_load_mimage_mw2_mimage(fname);
      ll = (Morpho_line) mw_mimage_to_morpho_line(mimage);
      curve = (Curve) mw_morpho_line_to_curve(ll);
      fpoly = (Fpolygon) mw_curve_to_fpolygon(curve);
      mw_delete_curve(curve);
      mw_delete_morpho_line(ll);
      mw_delete_mimage(mimage);
      return(fpoly);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}  

/* Write file in A_POLY format */  

short _mw_create_fpolygon_a_fpoly(fname,fpoly)

char  *fname;                        /* file name */
Fpolygon fpoly;

{
  FILE *fp;
  Point_fcurve pc;
  int i;

  if (fpoly == NULL)
    mwerror(INTERNAL,1,"Cannot create file: Fpolygon structure is NULL\n");

  if (fpoly->first == NULL)
    mwerror(INTERNAL,1,
	      "Cannot create file: No point in the Fpolygon structure\n");

  fp =_mw_create_data_ascii_file(fname);
  if (fp == NULL) return(-1);

  fprintf(fp,"%%\n");
  fprintf(fp,"%%----- Fpolygon -----\n");
  fprintf(fp,"def Fpolygon\n");
  fprintf(fp,"nb_channels: %d\n",fpoly->nb_channels);
  for (i=0;i<fpoly->nb_channels;i++)
    fprintf(fp,"channel #%d: %f\n",i+1,fpoly->channel[i]);
  for (pc=fpoly->first; pc; pc=pc->next)
    fprintf(fp,"%f,%f\n",pc->x,pc->y);
  
  fclose(fp);
  return(0);
}

/* Write file in different formats */

short _mw_create_fpolygon(fname,fpoly,Type)

char  *fname;                        /* file name */
Fpolygon fpoly;
char  *Type;                         /* Type de format du fichier */

{ 
  short ret;
  Polygon poly;
  Fcurve fcurve;
  Curve curve;
  Morpho_line ll;
  Mimage mimage;

  if (strcmp(Type,"A_FPOLY") == 0)
    return(_mw_create_fpolygon_a_fpoly(fname,fpoly));

  if (strcmp(Type,"MW2_CURVE") == 0)
    {
      curve = (Curve) mw_fpolygon_to_curve(fpoly);
      ret = _mw_create_curve_mw2_curve(fname,curve);
      mw_delete_curve(curve);
      return(ret);
    }

  if (strcmp(Type,"MW2_FCURVE") == 0)
    {
      fcurve = (Fcurve) mw_fpolygon_to_fcurve(fpoly);
      ret = _mw_create_fcurve_mw2_fcurve(fname,fcurve);
      mw_delete_fcurve(fcurve);
      return(ret);
    }

  if (strcmp(Type,"A_POLY") == 0)
    {
      poly = (Polygon) mw_fpolygon_to_polygon(fpoly);
      ret = _mw_create_polygon_a_poly(fname,poly);
      mw_delete_polygon(poly);
      return(ret);
    }

 if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      curve = (Curve) mw_fpolygon_to_curve(fpoly);
      ll = (Morpho_line) mw_curve_to_morpho_line(curve);
      ret = _mw_create_ml_mw2_ml(fname,ll);
      mw_delete_morpho_line(ll);
      mw_delete_curve(curve);
      return(ret);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      curve = (Curve) mw_fpolygon_to_curve(fpoly);
      ll = (Morpho_line) mw_curve_to_morpho_line(curve);
      mimage = (Mimage) mw_morpho_line_to_mimage(ll);
      ret = _mw_create_mimage_mw2_mimage(fname,mimage);
      mw_delete_mimage(mimage);
      mw_delete_morpho_line(ll);
      mw_delete_curve(curve);
      return(ret);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}

/* ---- I/O for Fpolygons ---- */

/* Load fpolygons from a file of A_FPOLY format */

Fpolygons _mw_load_fpolygons_a_fpoly(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  Fpolygons poly;
  Fpolygon newp,oldp;
  Point_fcurve newpc,oldpc;
  int i,nc;
  float px,py;
  char channel[20];

  fp = _mw_open_data_ascii_file(fname);
  if (fp == NULL) return(NULL);

  if (_mw_fascii_search_string(fp,"def Fpolygon\n") == EOF)
    {
      mwerror(ERROR, 0,
	      "No Fpolygon description found in the file \"%s\"",fname);
      fclose(fp);
      return(NULL);
    }

  poly = mw_new_fpolygons();
  if (poly == NULL) return(poly);
  oldp = newp = NULL;

  do 
    {
      if ((_mw_fascii_get_field(fp,fname,"nb_channels:","%d",&nc) != 1)
	  || (nc <= 0))
	{
	  mw_delete_fpolygons(poly);
	  fclose(fp);
	  return(NULL);
	}
      
      newp = mw_change_fpolygon(NULL,nc);
      if (newp == NULL)
	{
	  mw_delete_fpolygons(poly);
	  fclose(fp);
	  return(NULL);
	}
      if (poly->first == NULL) poly->first = newp;
      if (oldp != NULL) oldp->next = newp;
      newp->previous = oldp;
      newp->next = NULL;
      newp->first = NULL;
      oldpc = newpc = NULL;

      for (i=0;i<nc;i++)
	{
	  sprintf(channel,"channel #%d:",i+1);
	  if (_mw_fascii_get_field(fp,fname,channel,"%f",&newp->channel[i]) != 1) 
	    {
	      mw_delete_fpolygons(poly);
	      fclose(fp);
	      return(NULL);
	    }
	}

      while (fscanf(fp,"%f,%f\n",&px,&py) ==  2)
	{
	  newpc = mw_new_point_fcurve();
	  if (newpc == NULL)
	    {
	      mw_delete_fpolygons(poly);
	      fclose(fp);
	      return(NULL);
	    }
	  if (newp->first == NULL) newp->first = newpc;
	  if (oldpc != NULL) oldpc->next = newpc;
	  newpc->previous = oldpc;
	  newpc->next = NULL;
	  newpc->x = px; newpc->y = py;
	  oldpc = newpc;
	}
      oldp = newp;
    } while (_mw_fascii_search_string(fp,"def Fpolygon\n") != EOF);
      
  fclose(fp);

  return(poly);
}

/* Load fpolygons from file of different types */

Fpolygons _mw_load_fpolygons(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Polygons polys;
  Fpolygons fpolys;
  Curves curves;
  Curve curve;
  Fcurves fcurves;
  Morpho_line ll;
  Mimage mimage;
 
  _mw_get_file_type(fname,Type,mtype);
  
  if (strcmp(Type,"A_FPOLY") == 0)
    return(_mw_load_fpolygons_a_fpoly(fname));

  if (strcmp(Type,"MW2_CURVES") == 0)
    {
      curves = (Curves) _mw_load_curves_mw2_curves(fname);
      fpolys = (Fpolygons) mw_curves_to_fpolygons(curves);
      mw_delete_curves(curves);
      return(fpolys);
    }

  if (strcmp(Type,"MW2_FCURVES") == 0)
    {
      fcurves = (Fcurves) _mw_load_fcurves_mw2_fcurves(fname);
      fpolys = (Fpolygons) mw_fcurves_to_fpolygons(fcurves);
      mw_delete_fcurves(fcurves);
      return(fpolys);
    }

  if (strcmp(Type,"A_POLY") == 0)
    {
      polys = (Polygons) _mw_load_polygons_a_poly(fname);
      fpolys = (Fpolygons) mw_polygons_to_fpolygons(polys);
      mw_delete_polygons(polys);      
      return(fpolys);
    }

 if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      ll = (Morpho_line) _mw_load_ml_mw2_ml(fname);
      curve = (Curve) mw_morpho_line_to_curve(ll);
      curves = (Curves) mw_curve_to_curves(curve);
      fpolys = (Fpolygons) mw_curves_to_fpolygons(curves);
      mw_delete_curves(curves);
      mw_delete_curve(curve);
      mw_delete_morpho_line(ll);
      return(fpolys);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) _mw_load_mimage_mw2_mimage(fname);
      curves = (Curves) mw_mimage_to_curves(mimage);
      fpolys = (Fpolygons) mw_curves_to_fpolygons(curves);
      mw_delete_curves(curves);
      mw_delete_mimage(mimage);
      return(fpolys);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}  

/* Write file in A_FPOLY format */  

short _mw_create_fpolygons_a_fpoly(fname,poly)

char  *fname;                        /* file name */
Fpolygons poly;

{
  FILE *fp;
  Fpolygon pl;
  Point_fcurve pc;
  int i,n;

  if (poly == NULL)
    mwerror(INTERNAL,1,"Cannot create file: Fpolygons structure is NULL\n");

  if (poly->first == NULL)
    mwerror(INTERNAL,1,
	      "Cannot create file: No fpolygon in the Fpolygons structure\n");

  fp =_mw_create_data_ascii_file(fname);
  if (fp == NULL) return(-1);

  for (pl=poly->first, n=1; pl; pl=pl->next, n++)
    {
      if (pl->first == NULL)
	mwerror(INTERNAL,1,"Fpolygon #%d has no point fcurve\n",n);
      fprintf(fp,"%%\n");
      fprintf(fp,"%%----- Fpolygon #%d -----\n",n);
      fprintf(fp,"def Fpolygon\n");
      fprintf(fp,"nb_channels: %d\n",pl->nb_channels);
      for (i=0;i<pl->nb_channels;i++)
	fprintf(fp,"channel #%d: %f\n",i+1,pl->channel[i]);
      
      for (pc=pl->first; pc; pc=pc->next)
	fprintf(fp,"%f,%f\n",pc->x,pc->y);
    }
      
  fclose(fp);
  return(0);
}
   
/* Write file in different formats */

short _mw_create_fpolygons(fname,fpolys,Type)

char  *fname;                        /* file name */
Fpolygons fpolys;
char  *Type;                         /* Type de format du fichier */

{ 
  short ret;
  Polygons polys;
  Fcurves fcurves;
  Curves curves;
  Curve curve;
  Morpho_line ll;
  Mimage mimage;

  if (strcmp(Type,"A_FPOLY") == 0)
    return(_mw_create_fpolygons_a_fpoly(fname,fpolys));

  if (strcmp(Type,"MW2_CURVES") == 0)
    {
      curves = (Curves) mw_fpolygons_to_curves(fpolys);
      ret = _mw_create_curves_mw2_curves(fname,curves);
      mw_delete_curves(curves);
      return(ret);
    }

  if (strcmp(Type,"MW2_FCURVES") == 0)
    {
      fcurves = (Fcurves) mw_fpolygons_to_fcurves(fpolys);
      ret = _mw_create_fcurves_mw2_fcurves(fname,fcurves);
      mw_delete_fcurves(fcurves);
      return(ret);
    }

  if (strcmp(Type,"A_POLY") == 0)
    {
      polys = (Polygons) mw_fpolygons_to_polygons(fpolys);
      ret = _mw_create_polygons_a_poly(fname,polys);
      mw_delete_polygons(polys);
      return(ret);
    }

 if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      curves = (Curves) mw_fpolygons_to_curves(fpolys);
      curve = (Curve) mw_curves_to_curve(curves);
      ll = (Morpho_line) mw_curve_to_morpho_line(curve);
      ret = _mw_create_ml_mw2_ml(fname,ll);
      mw_delete_morpho_line(ll);
      mw_delete_curve(curve);
      mw_delete_curves(curves);
      return(ret);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      curves = (Curves) mw_fpolygons_to_curves(fpolys);
      mimage = (Mimage) mw_curves_to_mimage(curves);
      ret = _mw_create_mimage_mw2_mimage(fname,mimage);
      mw_delete_mimage(mimage);
      mw_delete_curves(curves);
      return(ret);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}








