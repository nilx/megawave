/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   native_ftype.h
   
   Vers. 1.07
   Author : Jacques Froment

   List the external (file) types sorted in decreasing priority order,
   associated to a memory format. 
   "Native" external types are listed only (that is, types for which no
   conversion is needed). External types accessible using conversion are
   now automatically managed by the system.

   Since V.1.06, a new list has been added, which gives a short description
   for each external (file) types.

   Those lists should be updated each time a new file format is added.

   Versions history
   1.07 (JF, march 2006) : - wpack2d added; 
                           - description of implemented file types corrected
                          
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
  "cimage","IMG","PM_C","GIF","TIFF","PGMA","PGMR","BMP",
#ifdef JPEG
  "JFIF",
#endif
  "PS","EPSF","INR","MTI","BIN",NULL,
  "fimage","RIM","PM_F",NULL,
  "ccimage","PMC_C","TIFFC","BMPC","PPM",
#ifdef JPEG
"JFIFC",
#endif
NULL,
  "cfimage","PMC_F",NULL,
  "fsignal","A_FSIGNAL","WAVE_PCM",NULL,
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
  "wpack2d","A_WPACK2D",NULL,
  NULL,
};


/*
  A short description for each implemented file types.
*/

static char *mw_ftypes_description[] = 
{
  "IMG", "Char image format defined by the defunct PCVision software and used by MegaWave1 [1x8-bits plane]",
  "PM_C", "PM char image format [1x8-bits plane]",
  "GIF", "GIF87 char image format (Graphics Interchange Format) [1x8-bits plane]",
  "TIFF", "Tag char image format [1x8-bits plane]",
  "PGMA", "PGM char image format (portable graymap), ascii version [1x8-bits plane]",
  "PGMR", "PGM char image format (portable graymap), rawbits version [1x8-bits plane]",
  "BMP", "Microsoft Windows char image format (BitMaP) [1x8-bits plane]",
#ifdef JPEG
  "JFIF", "JPEG/JFIF char image format from the Independent JPEG Group, monochrome version [1x8-bits plane]",
#endif
  "PS", "PostScript (level 1) char image format, for output objects only [1x8-bits plane]",
  "EPSF", "Encapsulated PostScript (level 1) char image format, for output objects only [1x8-bits plane]",
  "INR", "Old char image format defined by the software Inrimage (from INRIA) [1x8-bits plane]",
  "MTI", "Old char image format defined by the software MultImage (from 2AI). Quite exotic now [1x8-bits plane]",
  "BIN", "Universal binary char image format. No header (squared images only) [1x8-bits plane]",

  "RIM", "Original float image format defined by MegaWave1 [1x32-bits plane]",
  "PM_F", "PM float image format [1x32-bits plane]",

  "PMC_C", "PM color image format [3x8-bits planes]",
  "TIFFC", "Tag color image format [3x8-bits planes]",
  "BMPC", "Microsoft Windows color image format (BitMaP) [3x8-bits planes]",
  "PPM", "Portable pixmap color image format, rawbits version [3x8-bits planes]",
#ifdef JPEG
"JFIFC", "JPEG/JFIFC color image format from the Independent JPEG Group [3x8-bits planes]",
#endif
  "PMC_F", "PM color float image format [3x32-bits planes]",
  "A_FSIGNAL", "MegaWave2 Data Ascii format for Fsignal internal type",
  "WAVE_PCM", "Microsoft's RIFF WAVE sound file format with PCM encoding",
  "MW2_CURVE", "MegaWave2 binary format for Curve internal type",
  "MW2_CURVES","MegaWave2 binary format for Curves internal type",
  "MW2_FCURVE", "MegaWave2 binary format for Fcurve internal type",
  "MW2_FCURVES", "MegaWave2 binary format for Fcurves internal type",
  "MW2_DCURVE", "MegaWave2 binary format for Dcurve internal type",
  "MW2_DCURVES","MegaWave2 binary format for Dcurves internal type",
  "A_POLY","MegaWave2 Data Ascii format for Polygon and Polygons internal types",
  "A_FPOLY","MegaWave2 Data Ascii format for Fpolygon and Fpolygons internal types",
  "MW2_MORPHO_LINE","MegaWave2 binary format for Morpho_line internal type",
  "MW2_FMORPHO_LINE","MegaWave2 binary format for Fmorpho_line internal type",
  "MW2_MORPHO_SET","MegaWave2 binary format for Morpho_set internal type",
  "MW2_MORPHO_SETS","MegaWave2 binary Ascii format for Morpho_sets internal type",
  "MW2_MIMAGE","MegaWave2 Data binary format for Mimage internal type",
  "MW2_CMORPHO_LINE","MegaWave2 binary format for Cmorpho_line internal type",
  "MW2_CFMORPHO_LINE","MegaWave2 binary format for Cfmorpho_line internal type",
  "MW2_CMORPHO_SET","MegaWave2 binay format for Cmorpho_set internal type",
  "MW2_CMORPHO_SETS","MegaWave2 binary format for Cmorpho_sets internal type",
  "MW2_CMIMAGE","MegaWave2 binary format for Cmimage internal type",
  "MW2_SHAPE","MegaWave2 binary format for Shape internal type",
  "MW2_SHAPES","MegaWave2 binary format for Shapes internal type",
  "MW2_FLIST","MegaWave2 binary format for Flist internal type",
  "MW2_FLISTS","MegaWave2 binary format for Flists internal type",
  "MW2_DLIST","MegaWave2 binary format for Dlist internal type",
  "MW2_DLISTS","MegaWave2 binary format for Dlists internal type",
  "A_WPACK2D", "MegaWave2 Data Ascii format for Wpack2d internal type (with pointers to signal and image files)",
  NULL,
};


#endif

