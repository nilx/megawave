/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   polygon_io.c
   
   Vers. 1.6
   (C) 1993-96 Jacques Froment
   Input/Output functions for the polygon & polygons structure

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

/* ---- I/O for Polygon ---- */

/* Load polygon from a file of A_POLY format */

Polygon _mw_load_polygon_a_poly(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  Polygon poly;
  Point_curve newpc,oldpc;
  int px,py,i,nc;
  char channel[20];

  fp = _mw_open_data_ascii_file(fname);
  if (fp == NULL) return(NULL);

  if (_mw_fascii_search_string(fp,"def Polygon\n") == EOF)
    {
      mwerror(ERROR, 0,
	      "No Polygon description found in the file \"%s\"",fname);
      fclose(fp);
      return(NULL);
    }

  poly = mw_new_polygon();
  if (poly == NULL) return(poly);

  if ((_mw_fascii_get_field(fp,fname,"nb_channels:","%d",&nc) != 1)
      || (nc < 0))
    {
      mw_delete_polygon(poly);
      fclose(fp);
      return(NULL);
    }
      
  poly = mw_change_polygon(poly,nc);
  if (poly == NULL)
    {
      mw_delete_polygon(poly);
      fclose(fp);
      return(NULL);
    }

  for (i=0;i<nc;i++)
    {
      sprintf(channel,"channel #%d:",i+1);
      if (_mw_fascii_get_field(fp,fname,channel,"%f",&poly->channel[i]) != 1) 
	{
	  mw_delete_polygon(poly);
	  fclose(fp);
	  return(NULL);
	}
    }

  oldpc = NULL;
  while (fscanf(fp,"%d,%d\n",&px,&py) ==  2)
    {
      newpc = mw_new_point_curve();
      if (newpc == NULL)
	{
	  mw_delete_polygon(poly);
	  fclose(fp);
	  return(NULL);
	}
      if (poly->first == NULL) poly->first = newpc;
      if (oldpc != NULL) oldpc->next = newpc;
      newpc->previous = oldpc;
      newpc->next = NULL;
      newpc->x = px; newpc->y = py;
      oldpc = newpc;
    }
  fclose(fp);

  return(poly);
}


/* Load polygon from file of different types */

Polygon _mw_load_polygon(fname,Type)

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
  
  if (strcmp(Type,"A_POLY") == 0)
    return(_mw_load_polygon_a_poly(fname));

  if (strcmp(Type,"MW2_CURVE") == 0)
    {
      curve = (Curve) _mw_load_curve_mw2_curve(fname);
      poly = (Polygon) mw_curve_to_polygon(curve);
      mw_delete_curve(curve);
      return(poly);
    }

  if (strcmp(Type,"MW2_FCURVE") == 0)
    {
      fcurve = (Fcurve) _mw_load_fcurve_mw2_fcurve(fname);
      poly = (Polygon) mw_fcurve_to_polygon(fcurve);
      mw_delete_fcurve(fcurve);
      return(poly);
    }

  if (strcmp(Type,"A_FPOLY") == 0)
    {
      fpoly = (Fpolygon) _mw_load_fpolygon_a_fpoly(fname);
      poly = (Polygon) mw_fpolygon_to_polygon(fpoly);
      mw_delete_fpolygon(fpoly);      
      return(poly);
    }

  if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      ll = (Morpho_line) _mw_load_ml_mw2_ml(fname);
      curve = (Curve) mw_morpho_line_to_curve(ll);
      poly = (Polygon) mw_curve_to_polygon(curve);
      mw_delete_curve(curve);
      mw_delete_morpho_line(ll);
      return(poly);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) _mw_load_mimage_mw2_mimage(fname);
      ll = (Morpho_line) mw_mimage_to_morpho_line(mimage);
      curve = (Curve) mw_morpho_line_to_curve(ll);
      poly = (Polygon) mw_curve_to_polygon(curve);
      mw_delete_curve(curve);
      mw_delete_morpho_line(ll);
      mw_delete_mimage(mimage);
      return(poly);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}  

/* Write file in A_POLY format */  

short _mw_create_polygon_a_poly(fname,poly)

char  *fname;                        /* file name */
Polygon poly;

{
  FILE *fp;
  Point_curve pc;
  int i;

  if (poly == NULL)
    mwerror(INTERNAL,1,"Cannot create file: Polygon structure is NULL\n");

  if (poly->first == NULL)
    mwerror(INTERNAL,1,
	      "Cannot create file: No point in the Polygon structure\n");

  fp =_mw_create_data_ascii_file(fname);
  if (fp == NULL) return(-1);

  fprintf(fp,"%%\n");
  fprintf(fp,"%%----- Polygon -----\n");
  fprintf(fp,"def Polygon\n");
  fprintf(fp,"nb_channels: %d\n",poly->nb_channels);
  for (i=0;i<poly->nb_channels;i++)
    fprintf(fp,"channel #%d: %f\n",i+1,poly->channel[i]);
  for (pc=poly->first; pc; pc=pc->next)
    fprintf(fp,"%d,%d\n",pc->x,pc->y);
  
  fclose(fp);
  return(0);
}
   
/* Write file in different formats */

short _mw_create_polygon(fname,poly,Type)

char  *fname;                        /* file name */
Polygon poly;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Fpolygon fpoly;
  Fcurve fcurve;
  Curve curve;
  Morpho_line ll;
  Mimage mimage;

  if (strcmp(Type,"A_POLY") == 0)
    return(_mw_create_polygon_a_poly(fname,poly));

  if (strcmp(Type,"MW2_CURVE") == 0)
    {
      curve = (Curve) mw_polygon_to_curve(poly);
      ret = _mw_create_curve_mw2_curve(fname,curve);
      mw_delete_curve(curve);
      return(ret);
    }

  if (strcmp(Type,"MW2_FCURVE") == 0)
    {
      fcurve = (Fcurve) mw_polygon_to_fcurve(poly);
      ret = _mw_create_fcurve_mw2_fcurve(fname,fcurve);
      mw_delete_fcurve(fcurve);
      return(ret);
    }

  if (strcmp(Type,"A_FPOLY") == 0)
    {
      fpoly = (Fpolygon) mw_polygon_to_fpolygon(poly);
      ret = _mw_create_fpolygon_a_fpoly(fname,fpoly);
      mw_delete_fpolygon(fpoly);
      return(ret);
    }

 if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      curve = (Curve) mw_polygon_to_curve(poly);
      ll = (Morpho_line) mw_curve_to_morpho_line(curve);
      ret = _mw_create_ml_mw2_ml(fname,ll);
      mw_delete_morpho_line(ll);
      mw_delete_curve(curve);
      return(ret);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      curve = (Curve) mw_polygon_to_curve(poly);
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


/* --- I/O for Polygons ---- */

/* Load polygons from a file of A_POLY format */

Polygons _mw_load_polygons_a_poly(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  Polygons poly;
  Polygon newp,oldp;
  Point_curve newpc,oldpc;
  int px,py,i,nc;
  char channel[20];

  fp = _mw_open_data_ascii_file(fname);
  if (fp == NULL) return(NULL);

  if (_mw_fascii_search_string(fp,"def Polygon\n") == EOF)
    {
      mwerror(ERROR, 0,
	      "No Polygon description found in the file \"%s\"",fname);
      fclose(fp);
      return(NULL);
    }

  poly = mw_new_polygons();
  if (poly == NULL) return(poly);
  oldp = newp = NULL;

  do 
    {
      if ((_mw_fascii_get_field(fp,fname,"nb_channels:","%d",&nc) != 1)
	  || (nc < 0))
	{
	  mw_delete_polygons(poly);
	  fclose(fp);
	  return(NULL);
	}
      
      newp = mw_change_polygon(NULL,nc);
      if (newp == NULL)
	{
	  mw_delete_polygons(poly);
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
	      mw_delete_polygons(poly);
	      fclose(fp);
	      return(NULL);
	    }
	}

      while (fscanf(fp,"%d,%d\n",&px,&py) ==  2)
	{
	  newpc = mw_new_point_curve();
	  if (newpc == NULL)
	    {
	      mw_delete_polygons(poly);
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
    } while (_mw_fascii_search_string(fp,"def Polygon\n") != EOF);
      
  fclose(fp);
  return(poly);
}


/* Load polygons from file of different types */

Polygons _mw_load_polygons(fname,Type)

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
  
  if (strcmp(Type,"A_POLY") == 0)
    return(_mw_load_polygons_a_poly(fname));

  if (strcmp(Type,"MW2_CURVES") == 0)
    {
      curves = (Curves) _mw_load_curves_mw2_curves(fname);
      polys = (Polygons) mw_curves_to_polygons(curves);
      mw_delete_curves(curves);
      return(polys);
    }

  if (strcmp(Type,"MW2_FCURVES") == 0)
    {
      fcurves =(Fcurves) _mw_load_fcurves_mw2_fcurves(fname);
      polys = (Polygons) mw_fcurves_to_polygons(fcurves);
      mw_delete_fcurves(fcurves);
      return(polys);
    }

  if (strcmp(Type,"A_FPOLY") == 0)
    {
      fpolys = (Fpolygons) _mw_load_fpolygons_a_fpoly(fname);
      polys = (Polygons) mw_fpolygons_to_polygons(fpolys);
      mw_delete_fpolygons(fpolys);      
      return(polys);
    }

   if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      ll = (Morpho_line) _mw_load_ml_mw2_ml(fname);
      curve = (Curve) mw_morpho_line_to_curve(ll);
      curves = (Curves) mw_curve_to_curves(curve);
      polys = (Polygons) mw_curves_to_polygons(curves);
      mw_delete_curves(curves);
      mw_delete_curve(curve);
      mw_delete_morpho_line(ll);
      return(polys);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) _mw_load_mimage_mw2_mimage(fname);
      curves = (Curves) mw_mimage_to_curves(mimage);
      polys = (Polygons) mw_curves_to_polygons(curves);
      mw_delete_curves(curves);
      mw_delete_mimage(mimage);
      return(polys);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}  

/* Write file in A_POLY format */  

short _mw_create_polygons_a_poly(fname,poly)

char  *fname;                        /* file name */
Polygons poly;

{
  FILE *fp;
  Polygon pl;
  Point_curve pc;
  int i,n;

  if (poly == NULL)
    mwerror(INTERNAL,1,"Cannot create file: Polygons structure is NULL\n");

  if (poly->first == NULL)
    mwerror(INTERNAL,1,
	      "Cannot create file: No polygon in the Polygons structure\n");

  fp =_mw_create_data_ascii_file(fname);
  if (fp == NULL) return(-1);

  for (pl=poly->first, n=1; pl; pl=pl->next, n++)
    {
      if (pl->first == NULL)
	mwerror(INTERNAL,1,"Polygon #%d has no point curve\n",n);
      fprintf(fp,"%%\n");
      fprintf(fp,"%%----- Polygon #%d -----\n",n);
      fprintf(fp,"def Polygon\n");
      fprintf(fp,"nb_channels: %d\n",pl->nb_channels);
      for (i=0;i<pl->nb_channels;i++)
	fprintf(fp,"channel #%d: %f\n",i+1,pl->channel[i]);
      
      for (pc=pl->first; pc; pc=pc->next)
	fprintf(fp,"%d,%d\n",pc->x,pc->y);
    }
      
  fclose(fp);
  return(0);
}
   
/* Write file in different formats */

short _mw_create_polygons(fname,polys,Type)

char  *fname;                        /* file name */
Polygons polys;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Fpolygons fpolys;
  Fcurves fcurves;
  Curves curves;
  Curve curve;
  Morpho_line ll;
  Mimage mimage;

  if (strcmp(Type,"A_POLY") == 0)
    return(_mw_create_polygons_a_poly(fname,polys));

  if (strcmp(Type,"MW2_CURVES") == 0)
    {
      curves = (Curves) mw_polygons_to_curves(polys);
      ret = _mw_create_curves_mw2_curves(fname,curves);
      mw_delete_curves(curves);
      return(ret);
    }

  if (strcmp(Type,"MW2_FCURVES") == 0)
    {
      fcurves = (Fcurves) mw_polygons_to_fcurves(polys);
      ret = _mw_create_fcurves_mw2_fcurves(fname,fcurves);
      mw_delete_fcurves(fcurves);
      return(ret);
    }

  if (strcmp(Type,"A_FPOLY") == 0)
    {
      fpolys = (Fpolygons) mw_polygons_to_fpolygons(polys);
      ret = _mw_create_fpolygons_a_fpoly(fname,fpolys);
      mw_delete_fpolygons(fpolys);
      return(ret);
    }

 if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      curves = (Curves) mw_polygons_to_curves(polys);
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
      curves = (Curves) mw_polygons_to_curves(polys);
      mimage = (Mimage) mw_curves_to_mimage(curves);
      ret = _mw_create_mimage_mw2_mimage(fname,mimage);
      mw_delete_mimage(mimage);
      mw_delete_curves(curves);
      return(ret);
    }

 mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}





