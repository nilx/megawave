/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   xmw_com.c
   
   Vers. 1.1
   (C) 1994-2000 Jacques Froment
   Communication with XMegaWave

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifdef XMWP

#include <stdio.h>
#include <string.h>

#include "mw.h"
#include "Wdevice.h"
#include "xmw_com.h"

/*===== Modules functions, to display. =====*/

/* See also the file shell/xmw2_header.c */
extern splot();
extern cmview();
extern ccmview();

/*===== Input/Output files =====*/

/* A generic function to load a file in a MegaWave2 structure.   
   This function should be automatically generated from the megawave2.io file.
   Order follows the list in megawave2.io.
*/

short _mw_load_anytype(fname,ftype,comment,mwstruct,mtype)

char *fname; 	/* File to load (input) */
char ftype[]; 	/* Global file type (input and output) */
char comment[]; /* Global comment (input and output) */
void **mwstruct;/* Pointer to the adress where the memory structure has been 
		   put (output) */
char *mtype;    /* Memory type (computed from the file type of <fname>) which
                   gives the C type of <mwstruct> to XMegaWave (input). */
                   
{
 
 if (strcmp(mtype,"?") == 0)
   mwerror(FATAL,1,"File %s is not a MegaWave2 file\n",fname);

 if (strcmp(mtype,"cimage") == 0) 
   return(_mwload_cimage(fname, ftype, comment, (Cimage *) mwstruct));
 
 if (strcmp(mtype,"fimage") == 0)
   return(_mwload_fimage(fname, ftype, comment, (Fimage *) mwstruct));
   
 if (strcmp(mtype,"cmovie") == 0) 
   return(_mwload_cmovie(fname, ftype, comment, (Cmovie *) mwstruct));

 if (strcmp(mtype,"fmovie") == 0) 
   return(_mwload_fmovie(fname, ftype, comment, (Fmovie *) mwstruct));

 if (strcmp(mtype,"polygon") == 0) 
   return(_mwload_polygon(fname, ftype, comment, (Polygon *) mwstruct));
   
 if (strcmp(mtype,"polygons") == 0) 
   return(_mwload_polygons(fname, ftype, comment, (Polygons *) mwstruct));
   
 if (strcmp(mtype,"fpolygon") == 0) 
   return(_mwload_fpolygon(fname, ftype, comment, (Fpolygon *) mwstruct));

 if (strcmp(mtype,"fpolygons") == 0) 
   return(_mwload_fpolygons(fname, ftype, comment, (Fpolygons *) mwstruct));
   
 if (strcmp(mtype,"curve") == 0) 
   return(_mwload_curve(fname, ftype, comment, (Curve *) mwstruct));

 if (strcmp(mtype,"fcurve") == 0) 
   return(_mwload_fcurve(fname, ftype, comment, (Fcurve *) mwstruct));
 
 if (strcmp(mtype,"curves") == 0) 
   return(_mwload_curves(fname, ftype, comment, (Curves *) mwstruct));

 if (strcmp(mtype,"fcurves") == 0) 
   return(_mwload_fcurves(fname, ftype, comment, (Fcurves *) mwstruct));

 if (strcmp(mtype,"fsignal") == 0) 
   return(_mwload_fsignal(fname, ftype, comment, (Fsignal *) mwstruct));
   
 if (strcmp(mtype,"wtrans1d") == 0) 
   return(_mwload_wtrans1d(fname, ftype, comment, (Wtrans1d *) mwstruct));
   
 if (strcmp(mtype,"wtrans2d") == 0) 
   return(_mwload_wtrans2d(fname, ftype, comment, (Wtrans2d *) mwstruct));
   
 if (strcmp(mtype,"vchain_wmax") == 0) 
   return(_mwload_vchain_wmax(fname, ftype, comment, (Vchain_wmax *) mwstruct));
 
 if (strcmp(mtype,"vchains_wmax") == 0) 
   return(_mwload_vchains_wmax(fname, ftype, comment, (Vchains_wmax *) mwstruct));
 
 if (strcmp(mtype,"ccimage") == 0) 
   return(_mwload_ccimage(fname, ftype, comment, (Ccimage *) mwstruct));
   
 if (strcmp(mtype,"cfimage") == 0) 
   return(_mwload_cfimage(fname, ftype, comment, (Cfimage *) mwstruct));
 
 if (strcmp(mtype,"ccmovie") == 0) 
   return(_mwload_ccmovie(fname, ftype, comment, (Ccmovie *) mwstruct));

 if (strcmp(mtype,"cfmovie") == 0) 
   return(_mwload_cfmovie(fname, ftype, comment, (Cfmovie *) mwstruct));

 if (strcmp(mtype,"morpho_line") == 0) 
   return(_mwload_morpho_line(fname, ftype, comment, (Morpho_line *) mwstruct));

 if (strcmp(mtype,"fmorpho_line") == 0) 
   return(_mwload_fmorpho_line(fname, ftype, comment, (Fmorpho_line *) mwstruct));

 if (strcmp(mtype,"morpho_set") == 0) 
   return(_mwload_morpho_set(fname, ftype, comment, (Morpho_set *) mwstruct));

 if (strcmp(mtype,"morpho_sets") == 0) 
   return(_mwload_morpho_sets(fname, ftype, comment, (Morpho_sets *) mwstruct));

 if (strcmp(mtype,"mimage") == 0) 
   return(_mwload_mimage(fname, ftype, comment, (Mimage *) mwstruct));

  mwerror(INTERNAL,1,
     "[_mw_load_anytype]: Don't know how to load %s file format\n",mtype);
 return(-1);   
}


/* A generic function to save a file put in a MegaWave2 structure.
   This function should be automatically generated from the megawave2.io file.
   Order follows the list in megawave2.io.
*/

short _mw_save_anytype(fname,ftype,ftype_force,comment,mwstruct,mtype)

char *fname;		/* File to save (input) */
char ftype[];		/* Global file type (input) */
char ftype_force[]; 	/* Overwrite global file type (input) */
char comment[];		/* Global comment (input) */
void *mwstruct;		/* Memory structure */
char *mtype;		/* Memory type */

{

 if (strcmp(mtype,"cimage") == 0)
   return(_mwsave_cimage(fname, ftype, ftype_force, comment, (Cimage) mwstruct));
   
 if (strcmp(mtype,"fimage") == 0)
   return(_mwsave_fimage(fname, ftype, ftype_force, comment, (Fimage) mwstruct));

 if (strcmp(mtype,"cmovie") == 0)
   return(_mwsave_cmovie(fname, ftype, ftype_force, comment, (Cmovie) mwstruct));

 if (strcmp(mtype,"fmovie") == 0)
   return(_mwsave_fmovie(fname, ftype, ftype_force, comment, (Fmovie) mwstruct));

 if (strcmp(mtype,"polygon") == 0)
   return(_mwsave_polygon(fname, ftype, ftype_force, comment, (Polygon) mwstruct));

 if (strcmp(mtype,"polygons") == 0)
   return(_mwsave_polygons(fname, ftype, ftype_force, comment, (Polygons) mwstruct));
  
 if (strcmp(mtype,"fpolygon") == 0)
   return(_mwsave_fpolygon(fname, ftype, ftype_force, comment, (Fpolygon) mwstruct));

 if (strcmp(mtype,"fpolygons") == 0)
   return(_mwsave_fpolygons(fname, ftype, ftype_force, comment, (Fpolygons) mwstruct));

 if (strcmp(mtype,"curve") == 0)
   return(_mwsave_curve(fname, ftype, ftype_force, comment, (Curve) mwstruct));

 if (strcmp(mtype,"fcurve") == 0)
   return(_mwsave_fcurve(fname, ftype, ftype_force, comment, (Fcurve) mwstruct));

 if (strcmp(mtype,"curves") == 0)
   return(_mwsave_curves(fname, ftype, ftype_force, comment, (Curves) mwstruct));

 if (strcmp(mtype,"fcurves") == 0)
   return(_mwsave_fcurves(fname, ftype, ftype_force, comment, (Fcurves) mwstruct));

 if (strcmp(mtype,"fsignal") == 0)
   return(_mwsave_fsignal(fname, ftype, ftype_force, comment, (Fsignal) mwstruct));

 if (strcmp(mtype,"wtrans1d") == 0)
   return(_mwsave_wtrans1d(fname, ftype, ftype_force, comment, (Wtrans1d) mwstruct));

 if (strcmp(mtype,"wtrans2d") == 0)
   return(_mwsave_wtrans2d(fname, ftype, ftype_force, comment, (Wtrans2d) mwstruct));

 if (strcmp(mtype,"vchain_wmax") == 0)
   return(_mwsave_vchain_wmax(fname, ftype, ftype_force, comment, (Vchain_wmax) mwstruct));

 if (strcmp(mtype,"vchains_wmax") == 0)
   return(_mwsave_vchains_wmax(fname, ftype, ftype_force, comment, (Vchains_wmax) mwstruct));

 if (strcmp(mtype,"ccimage") == 0)
   return(_mwsave_ccimage(fname, ftype, ftype_force, comment, (Ccimage) mwstruct));

 if (strcmp(mtype,"cfimage") == 0)
   return(_mwsave_cfimage(fname, ftype, ftype_force, comment, (Cfimage) mwstruct));

 if (strcmp(mtype,"ccmovie") == 0)
   return(_mwsave_ccmovie(fname, ftype, ftype_force, comment, (Ccmovie) mwstruct));

 if (strcmp(mtype,"cfmovie") == 0)
   return(_mwsave_cfmovie(fname, ftype, ftype_force, comment, (Cfmovie) mwstruct));

 if (strcmp(mtype,"morpho_line") == 0)
   return(_mwsave_morpho_line(fname, ftype, ftype_force, comment, (Morpho_line) mwstruct));

 if (strcmp(mtype,"fmorpho_line") == 0)
   return(_mwsave_fmorpho_line(fname, ftype, ftype_force, comment, (Fmorpho_line) mwstruct));

 if (strcmp(mtype,"morpho_set") == 0)
   return(_mwsave_morpho_set(fname, ftype, ftype_force, comment, (Morpho_set) mwstruct));

 if (strcmp(mtype,"morpho_sets") == 0)
   return(_mwsave_morpho_sets(fname, ftype, ftype_force, comment, (Morpho_sets) mwstruct));

 if (strcmp(mtype,"mimage") == 0)
   return(_mwsave_mimage(fname, ftype, ftype_force, comment, (Mimage) mwstruct));

 mwerror(INTERNAL,1,
     "[_mw_save_anytype]: Don't know how to save %s file format\n",mtype);
 return(-1);   

}

/* Return the list of all available file formats associated with a memory
   format. The list is given by the static variables defined in file_type.h 
*/
   
char ** _mw_list_of_file_type(mtype)

char *mtype;

{
  if (strcmp(mtype,"cimage") == 0)
  	return(cimage_native_ftypes);
  if (strcmp(mtype,"fimage") == 0)
  	return(fimage_native_ftypes);
  if (strcmp(mtype,"fsignal") == 0)
  	return(fsignal_native_ftypes);
  if (strcmp(mtype,"ccimage") == 0)
  	return(ccimage_native_ftypes);
  if (strcmp(mtype,"cfimage") == 0)
  	return(cfimage_native_ftypes);
  if (strcmp(mtype,"curve") == 0)
  	return(curve_native_ftypes);
  if (strcmp(mtype,"curves") == 0)
  	return(curves_native_ftypes);
  if (strcmp(mtype,"fcurve") == 0)
  	return(fcurve_native_ftypes);
  if (strcmp(mtype,"fcurves") == 0)
  	return(fcurves_native_ftypes);
  if (strcmp(mtype,"polygon") == 0)
  	return(polygon_native_ftypes);
  if (strcmp(mtype,"fpolygon") == 0)
  	return(fpolygon_native_ftypes);
  if (strcmp(mtype,"polygons") == 0)
  	return(polygons_native_ftypes);
  if (strcmp(mtype,"fpolygons") == 0)
  	return(fpolygons_native_ftypes);
  if (strcmp(mtype,"morpho_line") == 0)
  	return(morpho_line_native_ftypes);
  if (strcmp(mtype,"fmorpho_line") == 0)
  	return(fmorpho_line_native_ftypes);
  if (strcmp(mtype,"mimage") == 0)
  	return(mimage_native_ftypes);
  if (strcmp(mtype,"morpho_set") == 0)
  	return(morpho_set_native_ftypes);
  if (strcmp(mtype,"morpho_sets") == 0)
  	return(morpho_sets_native_ftypes);
  	
  return(NULL);
}
  

/*====== Structure Conversion =====*/

/* A generic function to convert MegaWave2 structure, using the functions defined
   in convert_struct.c. Please modify the lists defined in xmw_com.h according to 
   each change in this function. Order follows the list in xmw_com.h.
*/

void * _mw_convert_struct(mwstruct,mtype_in,mtype_out)

void * mwstruct; /* Any type of MegaWave2 structure */
char *mtype_in;  /* Type of the input <mwstruct> */
char *mtype_out; /* Type of the output structure */

{

  if (strcmp(mtype_in,"cimage") == 0)
    {
      if (strcmp(mtype_out,"fimage") == 0)
      	return( (Fimage) mw_cimage_to_fimage( (Cimage) mwstruct));
      if (strcmp(mtype_out,"ccimage") == 0)
      	return( (Ccimage) mw_cimage_to_ccimage( (Cimage) mwstruct));
     }
    
  if (strcmp(mtype_in,"fimage") == 0)
    {
      if (strcmp(mtype_out,"cimage") == 0)
      	return( (Cimage) mw_fimage_to_cimage( (Fimage) mwstruct));
     }

  if (strcmp(mtype_in,"polygon") == 0)
    {
      if (strcmp(mtype_out,"fpolygon") == 0)
      	return( (Fpolygon) mw_polygon_to_fpolygon( (Polygon) mwstruct));
      if (strcmp(mtype_out,"curve") == 0)
      	return( (Curve) mw_polygon_to_curve( (Polygon) mwstruct));
      if (strcmp(mtype_out,"fcurve") == 0)
      	return( (Fcurve) mw_polygon_to_fcurve( (Polygon) mwstruct));
     }

  if (strcmp(mtype_in,"polygons") == 0)
    {
      if (strcmp(mtype_out,"fpolygons") == 0)
      	return( (Fpolygons) mw_polygons_to_fpolygons( (Polygons) mwstruct));
      if (strcmp(mtype_out,"curves") == 0)
      	return( (Curves) mw_polygons_to_curves( (Polygons) mwstruct));
      if (strcmp(mtype_out,"fcurves") == 0)
      	return( (Fcurves) mw_polygons_to_fcurves( (Polygons) mwstruct));
     }

  if (strcmp(mtype_in,"fpolygon") == 0)
    {
      if (strcmp(mtype_out,"polygon") == 0)
      	return( (Polygon) mw_fpolygon_to_polygon( (Fpolygon) mwstruct));
      if (strcmp(mtype_out,"fcurve") == 0)
      	return( (Fcurve) mw_fpolygon_to_fcurve( (Fpolygon) mwstruct));
      if (strcmp(mtype_out,"curve") == 0)
      	return( (Curve) mw_fpolygon_to_curve( (Fpolygon) mwstruct));
     }

  if (strcmp(mtype_in,"fpolygons") == 0)
    {
      if (strcmp(mtype_out,"polygons") == 0)
      	return( (Polygons) mw_fpolygons_to_polygons( (Fpolygons) mwstruct));
      if (strcmp(mtype_out,"fcurves") == 0)
      	return( (Fcurves) mw_fpolygons_to_fcurves( (Fpolygons) mwstruct));
      if (strcmp(mtype_out,"curves") == 0)
      	return( (Curves) mw_fpolygons_to_curves( (Fpolygons) mwstruct));
     }

   if (strcmp(mtype_in,"curve") == 0)
    {
      if (strcmp(mtype_out,"polygon") == 0)
      	return( (Polygon) mw_curve_to_polygon( (Curve) mwstruct));
      if (strcmp(mtype_out,"fcurve") == 0)
      	return( (Fcurve) mw_curve_to_polygon( (Curve) mwstruct));
      if (strcmp(mtype_out,"fpolygon") == 0)
      	return( (Fpolygon) mw_curve_to_fpolygon( (Curve) mwstruct));
      if (strcmp(mtype_out,"curves") == 0)
      	return( (Curves) mw_curve_to_curves( (Curve) mwstruct));
      if (strcmp(mtype_out,"morpho_line") == 0)
      	return( (Morpho_line) mw_curve_to_morpho_line( (Curve) mwstruct));
     }

   if (strcmp(mtype_in,"fcurve") == 0)
    {
      if (strcmp(mtype_out,"fpolygon") == 0)
      	return( (Fpolygon) mw_fcurve_to_fpolygon( (Fcurve) mwstruct));
      if (strcmp(mtype_out,"curve") == 0)
      	return( (Curve) mw_fcurve_to_curve( (Fcurve) mwstruct));
      if (strcmp(mtype_out,"polygon") == 0)
      	return( (Polygon) mw_fcurve_to_polygon( (Fcurve) mwstruct));
      if (strcmp(mtype_out,"fcurves") == 0)
      	return( (Fcurves) mw_fcurve_to_fcurves( (Fcurve) mwstruct));
      if (strcmp(mtype_out,"fmorpho_line") == 0)
      	return( (Fmorpho_line) mw_fcurve_to_fmorpho_line( (Fcurve) mwstruct));
     }

   if (strcmp(mtype_in,"curves") == 0)
    {
      if (strcmp(mtype_out,"polygons") == 0)
      	return( (Polygons) mw_curves_to_polygons( (Curves) mwstruct));
      if (strcmp(mtype_out,"fcurves") == 0)
      	return( (Fcurves) mw_curves_to_polygons( (Curves) mwstruct));
      if (strcmp(mtype_out,"fpolygons") == 0)
      	return( (Fpolygons) mw_curves_to_fpolygons( (Curves) mwstruct));
      if (strcmp(mtype_out,"curve") == 0)
      	return( (Curve) mw_curves_to_curve( (Curves) mwstruct));
      if (strcmp(mtype_out,"mimage") == 0)
      	return( (Mimage) mw_curves_to_mimage( (Curves) mwstruct));
     }

   if (strcmp(mtype_in,"fcurves") == 0)
    {
      if (strcmp(mtype_out,"fpolygons") == 0)
      	return( (Fpolygons) mw_fcurves_to_fpolygons( (Fcurves) mwstruct));
      if (strcmp(mtype_out,"curves") == 0)
      	return( (Curves) mw_fcurves_to_curves( (Fcurves) mwstruct));
      if (strcmp(mtype_out,"polygons") == 0)
      	return( (Polygons) mw_fcurves_to_polygons( (Fcurves) mwstruct));
      if (strcmp(mtype_out,"fcurve") == 0)
      	return( (Fcurve) mw_fcurves_to_fcurve( (Fcurves) mwstruct));
      if (strcmp(mtype_out,"mimage") == 0)
      	return( (Mimage) mw_fcurves_to_mimage( (Fcurves) mwstruct));
     }


   if (strcmp(mtype_in,"ccimage") == 0)
    {
      if (strcmp(mtype_out,"cfimage") == 0)
      	return( (Cfimage) mw_ccimage_to_cfimage( (Ccimage) mwstruct));
      if (strcmp(mtype_out,"cimage") == 0)
      	return( (Cimage) mw_ccimage_to_cimage( (Ccimage) mwstruct));
     }

  if (strcmp(mtype_in,"cfimage") == 0)
    {
      if (strcmp(mtype_out,"ccimage") == 0)
      	return( (Ccimage) mw_cfimage_to_ccimage( (Cfimage) mwstruct));
     }

  if (strcmp(mtype_in,"morpho_line") == 0)
    {
      if (strcmp(mtype_out,"curve") == 0)
      	return( (Curve) mw_morpho_line_to_curve( (Morpho_line) mwstruct));
      if (strcmp(mtype_out,"fmorpho_line") == 0)
      	return( (Fmorpho_line) mw_morpho_line_to_fmorpho_line((Morpho_line) mwstruct));
      if (strcmp(mtype_out,"mimage") == 0)
      	return( (Mimage) mw_morpho_line_to_mimage( (Morpho_line) mwstruct));
     }


  if (strcmp(mtype_in,"fmorpho_line") == 0)
    {
      if (strcmp(mtype_out,"fcurve") == 0)
      	return( (Fcurve) mw_fmorpho_line_to_fcurve( (Fmorpho_line) mwstruct));
      if (strcmp(mtype_out,"morpho_line") == 0)
      	return( (Morpho_line) mw_fmorpho_line_to_morpho_line((Fmorpho_line) mwstruct));
      if (strcmp(mtype_out,"mimage") == 0)
      	return( (Mimage) mw_fmorpho_line_to_mimage( (Fmorpho_line) mwstruct));
     }
     
  if (strcmp(mtype_in,"morpho_set") == 0)
    {
      if (strcmp(mtype_out,"morpho_sets") == 0)
      	return( (Morpho_sets) mw_morpho_set_to_morpho_sets( (Morpho_set) mwstruct));
     }

  if (strcmp(mtype_in,"morpho_sets") == 0)
    {
      if (strcmp(mtype_out,"morpho_set") == 0)
      	return( (Morpho_set) mw_morpho_sets_to_morpho_set( (Morpho_sets) mwstruct));
      if (strcmp(mtype_out,"mimage") == 0)
      	return( (Mimage) mw_morpho_sets_to_mimage( (Morpho_sets) mwstruct));
     }

  if (strcmp(mtype_in,"mimage") == 0)
    {
      if (strcmp(mtype_out,"morpho_line") == 0)
      	return( (Morpho_line) mw_mimage_to_morpho_line( (Mimage) mwstruct));
      if (strcmp(mtype_out,"fmorpho_line") == 0)
      	return( (Fmorpho_line) mw_mimage_to_fmorpho_line( (Mimage) mwstruct));
      if (strcmp(mtype_out,"curves") == 0)
      	return( (Curves) mw_mimage_to_curves( (Mimage) mwstruct));
      if (strcmp(mtype_out,"fcurves") == 0)
      	return( (Fcurves) mw_mimage_to_fcurves( (Mimage) mwstruct));
      if (strcmp(mtype_out,"morpho_sets") == 0)
      	return( (Morpho_sets) mw_mimage_to_morpho_sets( (Mimage) mwstruct));
     }
  return(NULL);

  /*  
    mwerror(INTERNAL,1,
     "(_mw_convert_anytype): Don't know how to convert %s memory type to %s\n",
    mtype_in,mtype_out);
  */

}


/* Return the list of all available memory formats associated with a memory
   format. The list is given by the static variables defined in xmw_com.h.
   The list should be maintained to reflect the capacity of _mw_convert_struct().
   Order follows the list in xmw_com.h.
*/

char ** _mw_list_of_convert_struct(mtype)

char *mtype;

{
  if (strcmp(mtype,"cimage") == 0)
  	return(conv_cimage);
  if (strcmp(mtype,"fimage") == 0)
  	return(conv_fimage);
  if (strcmp(mtype,"polygon") == 0)
  	return(conv_polygon);
  if (strcmp(mtype,"polygons") == 0)
  	return(conv_polygons);
  if (strcmp(mtype,"fpolygon") == 0)
  	return(conv_fpolygon);
  if (strcmp(mtype,"fpolygons") == 0)
  	return(conv_fpolygons);
  if (strcmp(mtype,"curve") == 0)
  	return(conv_curve);
  if (strcmp(mtype,"fcurve") == 0)
  	return(conv_fcurve);
  if (strcmp(mtype,"curves") == 0)
  	return(conv_curves);
  if (strcmp(mtype,"fcurves") == 0)
  	return(conv_fcurves);
  if (strcmp(mtype,"ccimage") == 0)
  	return(conv_ccimage);
  if (strcmp(mtype,"cfimage") == 0)
  	return(conv_cfimage);
  if (strcmp(mtype,"morpho_line") == 0)
  	return(conv_morpho_line);
  if (strcmp(mtype,"fmorpho_line") == 0)
  	return(conv_fmorpho_line);
  if (strcmp(mtype,"morpho_set") == 0)
  	return(conv_morpho_set);
  if (strcmp(mtype,"morpho_sets") == 0)
  	return(conv_morpho_sets);
  if (strcmp(mtype,"mimage") == 0)
  	return(conv_mimage);

  return(NULL);
}

/* A generic function to delete any MegaWave2 structure.
   Order follows the list in megawave2.io.
*/

void _mw_delete_anytype(mwstruct,mtype)

void * mwstruct; /* Any type of MegaWave2 structure */
char *mtype;     /* Type of the input <mwstruct> */

{
  if (strcmp(mtype,"cimage") == 0)
    {
      mw_delete_cimage((Cimage) mwstruct);
      return;
    }

  if (strcmp(mtype,"fimage") == 0)
    {
      mw_delete_fimage((Fimage) mwstruct);
      return;
    }  
  if (strcmp(mtype,"cmovie") == 0)
    {
      mw_delete_cmovie((Cmovie) mwstruct);
      return;
    }
  if (strcmp(mtype,"fmovie") == 0)
    {
      mw_delete_fmovie((Fmovie) mwstruct);
      return;
    }
  if (strcmp(mtype,"polygon") == 0)
    {
      mw_delete_polygon((Polygon) mwstruct);
      return;
    }
  if (strcmp(mtype,"polygons") == 0)
    {
      mw_delete_polygons((Polygons) mwstruct);
      return;
    }
  if (strcmp(mtype,"fpolygon") == 0)
    {
      mw_delete_fpolygon((Fpolygon) mwstruct);
      return;
    }
  if (strcmp(mtype,"fpolygons") == 0)
    {
      mw_delete_fpolygons((Fpolygons) mwstruct);
      return;
    }
  if (strcmp(mtype,"curve") == 0)
    {
      mw_delete_curve((Curve) mwstruct);
      return;
    }
  if (strcmp(mtype,"fcurve") == 0)
    {
      mw_delete_fcurve((Fcurve) mwstruct);
      return;
    }
  if (strcmp(mtype,"curves") == 0)
    {
      mw_delete_curves((Curves) mwstruct);
      return;
    }
  if (strcmp(mtype,"fcurves") == 0)
    {
      mw_delete_fcurves((Fcurves) mwstruct);
      return;
    }
  if (strcmp(mtype,"fsignal") == 0)
    {
      mw_delete_fsignal((Fsignal) mwstruct);
      return;
    }
  if (strcmp(mtype,"wtrans1d") == 0)
    {
      mw_delete_wtrans1d((Wtrans1d) mwstruct);
      return;
    }
  if (strcmp(mtype,"wtrans2d") == 0)
    {
      mw_delete_wtrans2d((Wtrans2d) mwstruct);
      return;
    }
  if (strcmp(mtype,"vchain_wmax") == 0)
    {
      mw_delete_vchain_wmax((Vchain_wmax) mwstruct);
      return;
    }
  if (strcmp(mtype,"vchains_wmax") == 0)
    {
      mw_delete_vchains_wmax((Vchains_wmax) mwstruct);
      return;
    }
  if (strcmp(mtype,"ccimage") == 0)
    {
      mw_delete_ccimage((Ccimage) mwstruct);
      return;
    }
  if (strcmp(mtype,"cfimage") == 0)
    {
      mw_delete_cfimage((Cfimage) mwstruct);
      return;
    }
  if (strcmp(mtype,"ccmovie") == 0)
    {
      mw_delete_ccmovie((Ccmovie) mwstruct);
      return;
    }
  if (strcmp(mtype,"cfmovie") == 0)
    {
      mw_delete_cfmovie((Cfmovie) mwstruct);
      return;
    }  
  if (strcmp(mtype,"morpho_line") == 0)
    {
      mw_delete_morpho_line((Morpho_line) mwstruct);
      return;
    }
  if (strcmp(mtype,"fmorpho_line") == 0)
    {
      mw_delete_fmorpho_line((Fmorpho_line) mwstruct);
      return;
    }
  if (strcmp(mtype,"morpho_set") == 0)
    {
      mw_delete_morpho_set((Morpho_set) mwstruct);
      return;
    }
  if (strcmp(mtype,"morpho_sets") == 0)
    {
      mw_delete_morpho_sets((Morpho_sets) mwstruct);
      return;
    }
  if (strcmp(mtype,"mimage") == 0)
    {
      mw_delete_mimage((Mimage) mwstruct);
      return;
    }
  
  mwerror(INTERNAL,1,
   "[_mw_delete_anytype]: Don't know how to delete memory type %s\n",mtype);

}

/* Get information from any MegaWave2 structure. Return the "ptype".
   See _mw_display_anytype() to know the display procedures available.
*/

int _mw_get_info_anytype(mwstruct,mtype,width,height,sname,comment,R,G,B,timer)

void * mwstruct; /* Any type of MegaWave2 structure */
char *mtype;     /* Type of the input <mwstruct> */
int *width;      /* width (ncol) in case of an image */
int *height;     /* height (nrow) in case of an image */
char *sname;      /* name of the loaded structure */
char *comment;   /* comment associated to the structure */
void **R,**G,**B;/* pointers to data */
int *timer;     /* if 1, When displayed the structure needs timer interrupt */

{ 
  if (strcmp(mtype,"cimage") == 0)
  {
    *width = ((Cimage) mwstruct)-> ncol;
    *height = ((Cimage) mwstruct)-> nrow;
    strcpy(sname,((Cimage) mwstruct)->name);
    strcpy(comment,((Cimage) mwstruct)->cmt);
    *R = ((Cimage) mwstruct)->gray;
    *G = *B = NULL;
    *timer=0;
    return(1);
   }
   
  if (strcmp(mtype,"fimage") == 0)
  {
    *width = ((Fimage) mwstruct)-> ncol;
    *height = ((Fimage) mwstruct)-> nrow;
    strcpy(sname,((Fimage) mwstruct)->name);
    strcpy(comment,((Fimage) mwstruct)->cmt);
    *R = ((Fimage) mwstruct)->gray;
    *G = *B = NULL;
    *timer=0;
    return(2);
   }
   
  if (strcmp(mtype,"ccimage") == 0)
  {
    *width = ((Ccimage) mwstruct)-> ncol;
    *height = ((Ccimage) mwstruct)-> nrow;
    strcpy(sname,((Ccimage) mwstruct)->name);
    strcpy(comment,((Ccimage) mwstruct)->cmt);
    *R = ((Ccimage) mwstruct)->red;
    *G = ((Ccimage) mwstruct)->green;
    *B = ((Ccimage) mwstruct)->blue;
    *timer=0;
    return(3);
   }
   
  if (strcmp(mtype,"cfimage") == 0)
  {
    *width = ((Cfimage) mwstruct)-> ncol;
    *height = ((Cfimage) mwstruct)-> nrow;
    strcpy(sname,((Cfimage) mwstruct)->name);
    strcpy(comment,((Cfimage) mwstruct)->cmt);
    *R = ((Cfimage) mwstruct)->red;
    *G = ((Cfimage) mwstruct)->green;
    *B = ((Cfimage) mwstruct)->blue;
    *timer=0;
    return(4);
   }
 
  /* Not an image */
  *R = *G = *B = NULL;

  /* Put here types for which a display procedure is written but
     which are not images. Return the size the window has to be.
  */
  if (strcmp(mtype,"fsignal")==0)
    {
      *width = 500;   /* Default value of splot set in _mw_display_anytype() */
      *height = 200;
      *timer=0;
      return(0);
    }

  /* Movies */
  if ((strcmp(mtype,"cmovie")==0) && (((Cmovie) mwstruct)->first))
    {
      *width = (((Cmovie) mwstruct)->first)->ncol;
      *height = (((Cmovie) mwstruct)->first)->nrow;
      *timer=1000;
      return(0);
    }
  if ((strcmp(mtype,"ccmovie")==0) && (((Ccmovie) mwstruct)->first))
    {
      *width = (((Ccmovie) mwstruct)->first)->ncol;
      *height = (((Ccmovie) mwstruct)->first)->nrow;
      *timer=1000;
      return(0);
    }
  if ((strcmp(mtype,"fmovie")==0) && (((Fmovie) mwstruct)->first))
    {
      /*  No display implemented now 
       *width = (((Fmovie) mwstruct)->first)->ncol;
       *height = (((Fmovie) mwstruct)->first)->nrow;
       *timer=1000;
       return(0);
      */

    }
  if ((strcmp(mtype,"cfmovie")==0) && (((Cfmovie) mwstruct)->first))
    {
      /*  No display implemented now 
       *width = (((Cfmovie) mwstruct)->first)->ncol;
       *height = (((Cfmovie) mwstruct)->first)->nrow;
       *timer=1000;
       return(0);
      */
    }


  /* No display procedure available */
  *width = *height = -1;
  *timer=0;
  return(-1);
}

/* Put information to any MegaWave2 structure
*/

void _mw_put_info_anytype(mwstruct,mtype,width,height,sname,comment,R,G,B,ptype)

void * mwstruct; /* Any type of MegaWave2 structure */
char *mtype;     /* Type of the input <mwstruct> */
int width;      /* width (ncol) in case of an image */
int height;     /* height (nrow) in case of an image */
char *sname;      /* name of the loaded structure */
char *comment;   /* comment associated to the structure */
void *R,*G,*B ; /* pointer to data */
int ptype;      /* type of the data */

{ 
  if ((strcmp(mtype,"cimage") == 0)&&(ptype == 1))
  {
    strncpy(((Cimage) mwstruct)->name,sname,mw_namesize);
    strncpy(((Cimage) mwstruct)->cmt,comment,mw_cmtsize);
 
    if ((R != ((Cimage) mwstruct)->gray)||(((Cimage) mwstruct)-> ncol != width) ||
    	(((Cimage) mwstruct)-> nrow != height))
    {
      ((Cimage) mwstruct)-> ncol = width;
      ((Cimage) mwstruct)-> nrow = height;
      free(((Cimage) mwstruct)->gray);
      ((Cimage) mwstruct)->allocsize = width*height*sizeof(unsigned char);
      ((Cimage) mwstruct)->gray = R;
    }
    return;
   }
   
  if ((strcmp(mtype,"fimage") == 0)&&(ptype == 2))
  {
    strncpy(((Fimage) mwstruct)->name,sname,mw_namesize);
    strncpy(((Fimage) mwstruct)->cmt,comment,mw_cmtsize);
 
    if ((R != ((Fimage) mwstruct)->gray)||(((Fimage) mwstruct)-> ncol != width) ||
    	(((Fimage) mwstruct)-> nrow != height))
    {
      ((Fimage) mwstruct)-> ncol = width;
      ((Fimage) mwstruct)-> nrow = height;
      free(((Fimage) mwstruct)->gray);
      ((Fimage) mwstruct)->allocsize = width*height*sizeof(float);
      ((Fimage) mwstruct)->gray = R;
    }
    return;
   }

  if ((strcmp(mtype,"ccimage") == 0)&&(ptype == 3))
  {
    strncpy(((Ccimage) mwstruct)->name,sname,mw_namesize);
    strncpy(((Ccimage) mwstruct)->cmt,comment,mw_cmtsize);
 
    if ((R != ((Ccimage) mwstruct)->red)||
	(G != ((Ccimage) mwstruct)->green)||
	(B != ((Ccimage) mwstruct)->blue)||
	(((Ccimage) mwstruct)-> ncol != width) ||
    	(((Ccimage) mwstruct)-> nrow != height))
      {
	((Ccimage) mwstruct)-> ncol = width;
	((Ccimage) mwstruct)-> nrow = height;
	free(((Ccimage) mwstruct)->red);
	free(((Ccimage) mwstruct)->green);
	free(((Ccimage) mwstruct)->blue);
	((Ccimage) mwstruct)->allocsize = width*height*sizeof(unsigned char);
	((Ccimage) mwstruct)->red = R;
	((Ccimage) mwstruct)->green = G;
	((Ccimage) mwstruct)->blue = B;
      }
    return;
  }

  if ((strcmp(mtype,"cfimage") == 0)&&(ptype == 4))
  {
    strncpy(((Cfimage) mwstruct)->name,sname,mw_namesize);
    strncpy(((Cfimage) mwstruct)->cmt,comment,mw_cmtsize);
 
    if ((R != ((Cfimage) mwstruct)->red)||
	(G != ((Cfimage) mwstruct)->green)||
	(B != ((Cfimage) mwstruct)->blue)||
	(((Cfimage) mwstruct)-> ncol != width) ||
    	(((Cfimage) mwstruct)-> nrow != height))
      {
	((Cfimage) mwstruct)-> ncol = width;
	((Cfimage) mwstruct)-> nrow = height;
	free(((Cfimage) mwstruct)->red);
	free(((Cfimage) mwstruct)->green);
	free(((Cfimage) mwstruct)->blue);
	((Cfimage) mwstruct)->allocsize = width*height*sizeof(float);
	((Cfimage) mwstruct)->red = R;
	((Cfimage) mwstruct)->green = G;
	((Cfimage) mwstruct)->blue = B;
      }
    return;
  }
  
  mwerror(INTERNAL,1,
   "Cannot put information of type #%d in MegaWave2 memory of type %s\n",
   	ptype,mtype);
}

/*====== Structure copy =====*/

/* A generic function to copy MegaWave2 structure. 
*/

short _mw_copy_anytype(struct1,struct2,mtype)

void *struct1;   /* Any type of MegaWave2 structure (input) */
void **struct2;  /* Output structure */
char *mtype;  /* Type of the input <struct> */

{

  if (strcmp(mtype,"cimage") == 0)
    {
      *struct2=mw_change_cimage(NULL,((Cimage)struct1)->nrow,
				((Cimage)struct1)->ncol);
      if (*struct2 == NULL)
	mwerror(FATAL,1,"Not enough memory !\n");
      mw_copy_cimage((Cimage) struct1, (Cimage) *struct2);
      return(0);
    }

  if (strcmp(mtype,"fimage") == 0)
    {
      *struct2=mw_change_fimage(NULL,((Fimage)struct1)->nrow,
				((Fimage)struct1)->ncol);
      if (*struct2 == NULL)
	mwerror(FATAL,1,"Not enough memory !\n");
      mw_copy_fimage((Fimage) struct1, (Fimage) *struct2);
      return(0);
    }

  if (strcmp(mtype,"ccimage") == 0)
    {
      *struct2=mw_change_ccimage(NULL,((Ccimage)struct1)->nrow,
				((Ccimage)struct1)->ncol);
      if (*struct2 == NULL)
	mwerror(FATAL,1,"Not enough memory !\n");
      mw_copy_ccimage((Ccimage) struct1, (Ccimage) *struct2);
      return(0);
    }

  if (strcmp(mtype,"cfimage") == 0)
    {
      *struct2=mw_change_cfimage(NULL,((Cfimage)struct1)->nrow,
				((Cfimage)struct1)->ncol);
      if (*struct2 == NULL)
	mwerror(FATAL,1,"Not enough memory !\n");
      mw_copy_cfimage((Cfimage) struct1, (Cfimage) *struct2);
      return(0);
    }


  mwerror(INTERNAL,1,
   "[_mw_copy_anytype] Don't know how to copy structure of type %s !\n",mtype);
  return(-1);

}

/*===== Structure Display =====*/

/* A generic function to display in a window the content of a structure.

   WARNING: 
     - calls are genereted to XMW library. So you must have the XMW
       library in order to link this program.
     - update _mw_get_info_anytype() according to this function.   
*/

#ifdef __STDC__
short _mw_display_anytype(Wframe *window, void * mwstruct, char * mtype,
	int x, int y, int width, int height)
#else
short _mw_display_anytype(window,mwstruct,mtype,x,y,width,height)

Wframe *window;  /* the window where to display the structure */
void * mwstruct; /* Any type of MegaWave2 structure */
char *mtype;     /* Type of the input <mwstruct> */
int x,y;         /* Upper-left coordinate of the rectangle to display */  
int width;       /* width/height of the rectangle to display */
int height;     
#endif

{
  unsigned char *cptr=NULL;
  int size;

  int X=500; /* for splot */
  int Y=200;
  int loop=1; /* for cmovie */
  float zoom=1.0;

  if (strcmp(mtype,"cimage") == 0)
  {
    if (((x+width-1) > ((Cimage) mwstruct)->ncol) ||
    	((y+height-1) > ((Cimage) mwstruct)->nrow))
  	  mwerror(INTERNAL,1,"[_mw_display_anytype]: Rectangle out of cimage !\n");
  	  
    _mw_display_GrayImage8bits(window->win,((Cimage) mwstruct)->gray,
    	((Cimage) mwstruct)->ncol, x,y,width,height);  
    return(0);   
  }
  
  if (strcmp(mtype,"fimage") == 0)
  {
    if (((x+width-1) > ((Fimage) mwstruct)->ncol) ||
    	((y+height-1) > ((Fimage) mwstruct)->nrow))
  	  mwerror(INTERNAL,1,"[_mw_display_anytype]: Rectangle out of fimage !\n");

    /* Should be changed to alloc only the rectangle we want to display */
    size = ((Fimage) mwstruct)->ncol*((Fimage) mwstruct)->ncol;  
    if (!(cptr = (unsigned char *) malloc(size*sizeof(unsigned char))))
      {
	mwerror(ERROR, 0,"Not enough memory to display Fimage\n");
        return(2); /* NOT ENOUGH MEMORY */
      }
    _mw_float_to_uchar(((Fimage) mwstruct)->gray,cptr,size,"Gray");
    _mw_display_GrayImage8bits(window->win,cptr,((Fimage) mwstruct)->ncol, x,y,
    				width,height);  
    free(cptr);
    return(0);   
  }
  
  if (strcmp(mtype,"ccimage") == 0)
  {
    if (((x+width-1) > ((Ccimage) mwstruct)->ncol) ||
    	((y+height-1) > ((Ccimage) mwstruct)->nrow))
  	  mwerror(INTERNAL,1,"[_mw_display_anytype]: Rectangle out of ccimage !\n");
  	  
    _mw_display_ColorImage24bits(window->win,((Ccimage) mwstruct)->red,
    ((Ccimage) mwstruct)->green,((Ccimage) mwstruct)->blue,
    ((Ccimage) mwstruct)->ncol, x,y,width,height);     
    return(0);
  }

  if (strcmp(mtype,"fsignal") == 0)
  {
    mwrunmode=3;
    splot((Fsignal) mwstruct, &x, &y, &X, &Y, (int *) NULL, (char *) window);
    mwrunmode=2;
    return(0);   
  }  

  if (strcmp(mtype,"cmovie") == 0)
  {
    mwrunmode=3;
    cmview((Cmovie) mwstruct, &x, &y, &zoom, &loop, (char *) window);
    mwrunmode=2;
    return(0);   
  }  

  if (strcmp(mtype,"ccmovie") == 0)
  {
    mwrunmode=3;
    ccmview((Ccmovie) mwstruct, &x, &y, &zoom, &loop, (char *) window);
    mwrunmode=2;
    return(0);   
  }  

  mwerror(ERROR, 0,"No procedure to display objects of type %s\n",mtype);  
  return(1);
}

#endif /* XMWP */

