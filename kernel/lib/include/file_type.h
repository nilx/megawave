/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   file_type.h
   
   Vers. 1.10
   (C) 1993-99 Jacques Froment
   - List the external (file) types sorted in decreasing priority order,
   associated to a memory format (note that memory conversion may be 
   implemented. So there is no formal concordance between memory and 
   file format). A " " in the list indicates that the types which follow
   are conversion type (not to be read by _mw_type_exists).
   This list should be updated each time a new file format or a new memory
   conversion procedure is added.
   - Functions declaration in file_type.c

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef file_type_flg
#define file_type_flg

#include <stdio.h>

/*====== external (file) types sorted in decreasing priority order =====*/

static char *cimage_types[] = 
{
  "IMG",
  "PM_C",
  "GIF",
  "TIFF",
  "PGMA",
  "PGMR",
  "PS",
  "EPSF",
  "INR",
  "MTI",
  "BIN",
  " ",
  "RIM",
  "PM_F",
  "TIFFC",
  "PMC_C",
  "PMC_F",
  NULL
  };


static char *fimage_types[] = 
{
  "RIM",
  "PM_F",
  " ",
  "PMC_F",
  "PMC_C",
  "TIFFC",
  "IMG",
  "PM_C",
  "GIF",
  "TIFF",
  "PGMA",
  "PGMR",
  "PS",
  "EPSF",
  "INR",
  "MTI",
  "BIN",
  NULL
  };

static char *ccimage_types[] = 
{
  "PMC_C",
  "TIFFC",
  " ",
  "PMC_F",
  "IMG",
  "PM_C",
  "GIF",
  "TIFF",
  "PGMA",
  "PGMR",
  "PS",
  "EPSF",
  "INR",
  "MTI",
  "BIN",
  "RIM",
  "PM_F",
  NULL
  };

static char *cfimage_types[] = 
{
  "PMC_F",
  " ",
  "PMC_C",
  "TIFFC",
  "RIM",
  "PM_F",
  "IMG",
  "PM_C",
  "GIF",
  "TIFF",
  "PGMA",
  "PGMR",
  "PS",
  "EPSF",
  "INR",
  "MTI",
  "BIN",
  NULL
  };

static char *fsignal_types[] = 
{
  "A_FSIGNAL",
  NULL
  };


static char *curve_types[] = 
{
  "MW2_CURVE",
  " ",
  "MW2_FCURVE",
  "MW2_MORPHO_LINE",
  "MW2_FMORPHO_LINE",
  "MW2_MIMAGE",
  "A_POLY",
  "A_FPOLY",
  "MW2_CURVES",
  "MW2_FCURVES",
   NULL
  };


static char *curves_types[] = 
{
  "MW2_CURVES",
  " ",
  "MW2_FCURVES",
  "MW2_MORPHO_LINE",
  "MW2_FMORPHO_LINE",
  "MW2_MIMAGE",
  "A_POLY",
  "A_FPOLY",
  "MW2_CURVE",
  "MW2_FCURVE",
   NULL
  };

static char *fcurve_types[] = 
{
  "MW2_FCURVE",
  " ",
  "MW2_CURVE",
  "A_POLY",
  "A_FPOLY",
  "MW2_FCURVES",
  "MW2_CURVES",
  "MW2_MORPHO_LINE",
  "MW2_FMORPHO_LINE",
  "MW2_MIMAGE",
   NULL
  };

static char *fcurves_types[] = 
{
  "MW2_FCURVES",
  " ",
  "MW2_CURVES",
  "A_POLY",
  "A_FPOLY",
  "MW2_FCURVE",
  "MW2_CURVE",
  "MW2_MORPHO_LINE",
  "MW2_FMORPHO_LINE",
  "MW2_MIMAGE",
   NULL
  };

static char *polygon_types[] = 
{
  "A_POLY",
  " ",
  "MW2_CURVE",
  "MW2_FCURVE",
  "A_FPOLY",
  "MW2_MORPHO_LINE",
  "MW2_MIMAGE",
   NULL
  };


static char *polygons_types[] = 
{
  "A_POLY",
  " ",
  "MW2_CURVES",
  "MW2_FCURVES",
  "A_FPOLY",
  "MW2_MORPHO_LINE",
  "MW2_MIMAGE",
   NULL
  };

static char *fpolygon_types[] = 
{
  "A_FPOLY",
  " ",
  "MW2_CURVE",
  "MW2_FCURVE",
  "A_POLY",
  "MW2_MORPHO_LINE",
  "MW2_MIMAGE",
   NULL
  };



static char *fpolygons_types[] = 
{
  "A_FPOLY",
  " ",
  "MW2_CURVES",
  "MW2_FCURVES",
  "A_POLY",
  "MW2_MORPHO_LINE",
  "MW2_MIMAGE",
   NULL
  };

static char *morpho_line_types[] = 
{
  "MW2_MORPHO_LINE",
  " ",
  "MW2_FMORPHO_LINE",
  "MW2_MIMAGE",
  "MW2_CMORPHO_LINE",
  "MW2_CFMORPHO_LINE",
  "MW2_CMIMAGE",
  "MW2_CURVE",
   NULL
  };

static char *fmorpho_line_types[] = 
{
  "MW2_FMORPHO_LINE",
  " ",
  "MW2_MIMAGE",
  "MW2_FCURVE",
   NULL
  };

static char *morpho_set_types[] = 
{
  "MW2_MORPHO_SET",
  " ",
  "MW2_MORPHO_SETS",
  "MW2_MIMAGE",
   NULL
  };

static char *morpho_sets_types[] = 
{
  "MW2_MORPHO_SETS",
  " ",
  "MW2_MORPHO_SET",
  "MW2_MIMAGE",
   NULL
  };

static char *mimage_types[] = 
{
  "MW2_MIMAGE",
  " ",
  "MW2_MORPHO_LINE",
  "MW2_FMORPHO_LINE",
  "MW2_MORPHO_SET",
  "MW2_MORPHO_SETS",
  "MW2_CMIMAGE",
  "MW2_CMORPHO_LINE",
  "MW2_CURVES",
   NULL
  };


static char *cmorpho_line_types[] = 
{
  "MW2_CMORPHO_LINE",
  " ",
  "MW2_CFMORPHO_LINE",
  "MW2_CMIMAGE",
  "MW2_MORPHO_LINE",
  "MW2_FMORPHO_LINE",
  "MW2_MIMAGE",
  "MW2_CURVE",
   NULL
  };

static char *cfmorpho_line_types[] = 
{
  "MW2_CFMORPHO_LINE",
  " ",
  "MW2_CMIMAGE",
  "MW2_FCURVE",  
   NULL
  };


static char *cmorpho_set_types[] = 
{
  "MW2_CMORPHO_SET",
  " ",
  "MW2_CMORPHO_SETS",
  "MW2_CMIMAGE",
   NULL
  };

static char *cmorpho_sets_types[] = 
{
  "MW2_CMORPHO_SETS",
  " ",
  "MW2_CMORPHO_SET",
  "MW2_CMIMAGE",
   NULL
  };

static char *cmimage_types[] = 
{
  "MW2_CMIMAGE",
  " ",
  "MW2_CMORPHO_LINE",
  "MW2_CFMORPHO_LINE",
  "MW2_CMORPHO_SET",
  "MW2_CMORPHO_SETS",
  "MW_CURVES",
   NULL
  };

static char *shape_types[] = 
{
  "MW2_SHAPE",
   NULL
  };

static char *shapes_types[] = 
{
  "MW2_SHAPES",
   NULL
  };


/*===== functions declaration in file_type.c =====*/

#ifdef __STDC__

void _mw_lower_type(char []);
short _mw_range_type(char [],char *[]);
short _mw_type_exists(char [], char *[]);
short _mw_convert_type_exists(char [], char *[]);
void _mw_make_type(char [], char [], char *[]);
void _mw_print_convert_type(char *[], char *);
void _mw_choose_type(char [], char *, char *[], char *);
void  _mw_make_comment(char [], char []);
int _mw_get_binary_file_type(char *, char *, char *);
int _mw_get_ascii_file_type(char *, char *, char *);
int _mw_get_file_type(char *, char *, char *);
char ** _mw_list_of_file_type(char *mtype);

#else

void _mw_lower_type();
short _mw_range_type();
short _mw_type_exists();
short _mw_convert_type_exists();
void _mw_make_type();
void _mw_print_convert_type();
void _mw_choose_type();
void  _mw_make_comment();
int _mw_get_binary_file_type();
int _mw_get_ascii_file_type();
int _mw_get_file_type();
char ** _mw_list_of_file_type();

#endif

#endif

