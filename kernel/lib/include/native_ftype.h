/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   native_ftype.h
   
   Vers. 1.01
   (C) 2000-2001 Jacques Froment

   List the external (file) types sorted in decreasing priority order,
   associated to a memory format. 
   "Native" external types are listed only (that is, types for which no
   conversion is needed). External types accessible using conversion are
   now automatically managed by the system.
   This list should be updated each time a new file format is added.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef native_ftype_flg
#define native_ftype_flg

/*
  Native external (file) types sorted in decreasing priority order and
  associated to an internal (memory) type, which must be listed in first 
  position on each line.
  You may change the order if you want to change default priorities.
*/

static char *mw_native_ftypes[] = 
{
  "cimage","IMG","PM_C","GIF","TIFF","PGMA","PGMR","PS","EPSF","INR","MTI","BIN",NULL,
  "fimage","RIM","PM_F",NULL,
  "ccimage","PMC_C","TIFFC",NULL,
  "cfimage","PMC_F",NULL,
  "fsignal","A_FSIGNAL",NULL,
  "curve","MW2_CURVE",NULL,
  "curves","MW2_CURVES",NULL,
  "fcurve","MW2_FCURVE",NULL,
  "fcurves","MW2_FCURVES",NULL,
  "dcurve","MW2_DCURVE",NULL,
  "dcurves","MW2_DCURVES",NULL,
  "polygon","A_POLY",NULL,
  "polygons","A_POLY",NULL,
  "fpolygon","A_FPOLY",NULL,
  "fpolygons","A_FPOLY",NULL,
  "morpho_line","MW2_MORPHO_LINE",NULL,
  "fmorpho_line","MW2_FMORPHO_LINE",NULL,
  "morpho_set","MW2_MORPHO_SET",NULL,
  "morpho_sets","MW2_MORPHO_SETS",NULL,
  "mimage","MW2_MIMAGE",NULL,
  "cmorpho_line","MW2_CMORPHO_LINE",NULL,
  "cfmorpho_line","MW2_CFMORPHO_LINE",NULL,
  "cmorpho_set","MW2_CMORPHO_SET",NULL,
  "cmorpho_sets","MW2_CMORPHO_SETS",NULL,
  "cmimage","MW2_CMIMAGE",NULL,
  "shape","MW2_SHAPE",NULL,
  "shapes","MW2_SHAPES",NULL,
  "flist","MW2_FLIST",NULL,
  "flists","MW2_FLISTS",NULL,
  "dlist","MW2_DLIST",NULL,
  "dlists","MW2_DLISTS",NULL,
  NULL,
};


#endif

