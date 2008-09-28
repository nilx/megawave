/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   xmw_com.h
   
   Vers. 0.0
   (C) 1994 Jacques Froment
   - List the MegaWave2 memory conversion possibilities according to the
   function _mw_convert_struct().
   - Functions declaration in xmw_com.c

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef xmw_com_flg
#define xmw_com_flg

#include <stdio.h>

/*====== External references =====*/

extern int mwrunmode;  /* defined in mw.c */

/*====== List of allowed memory conversion =====*/

/* Order of the tab declaration follows the list in megawave2.io */

static char *conv_img[] =
{
  "rim",
  NULL
};

static char *conv_cimage[] = 
{
  "fimage",
  "ccimage",
  "rim",
  NULL
  };

static char *conv_rim[] =
{
  "cimage",
  "img",
  NULL
};


static char *conv_fimage[] = 
{
  "cimage", 
  NULL
};

static char *conv_cmovie[] = 
{
  NULL
};

static char *conv_fmovie[] = 
{
  NULL
};


static char *conv_polygon[] = 
{
  "fpolygon",
  "curve",
  "fcurve",
  NULL
};

static char *conv_polygons[] = 
{
  "fpolygons",
  "curves",
  "fcurves",
  NULL
};

static char *conv_fpolygon[] = 
{
  "polygon",
  "fcurve",
  "curve",
  NULL
};

static char *conv_fpolygons[] = 
{
  "polygons",
  "fcurves",
  "curves",
  NULL
};

static char *conv_curve[] = 
{
  "polygon",
  "fcurve",
  "fpolygon",
  "curves",
  "morpho_line",
  NULL
};

static char *conv_fcurve[] = 
{
  "fpolygon",
  "curve",
  "polygon",
  "fcurves",
  "fmorpho_line",
  NULL
};

static char *conv_curves[] = 
{
  "polygons",
  "fcurves",
  "fpolygons",
  "curve",
  "mimage",
  NULL
};

static char *conv_fcurves[] = 
{
  "fpolygons",
  "curves",
  "polygons",
  "fcurve",
  "mimage",
  NULL
};

static char *conv_fsignal[] = 
{
  NULL
};

static char *conv_wtrans1d[] = 
{
  NULL
};

static char *conv_wtrans2d[] = 
{
  NULL
};

static char *conv_vchain_wmax[] = 
{
  NULL
};

static char *conv_vchains_wmax[] = 
{
  NULL
};

static char *conv_ccimage[] = 
{
  "cfimage",
  "cimage",
  NULL
  };

static char *conv_cfimage[] = 
{
  "ccimage",
  NULL
};

static char *conv_ccmovie[] = 
{
  NULL
};

static char *conv_cfmovie[] = 
{
  NULL
};

static char *conv_morpho_line[] = 
{
  "curve",
  "fmorpho_line",
  "mimage",
  NULL
};

static char *conv_fmorpho_line[] = 
{
  "fcurve",
  "morpho_line",
  "mimage",
  NULL
};

static char *conv_morpho_set[] = 
{
  "morpho_sets",
  NULL
};

static char *conv_morpho_sets[] = 
{
  "morpho_sets",
  "mimage",
  NULL
};

static char *conv_mimage[] = 
{
  "morpho_line",
  "fmorpho_line",
  "curves",
  "fcurves",
  "morpho_sets",
  NULL
};

/*===== functions declaration in xmw_com.c =====*/

#ifdef __STDC__

short _mw_load_anytype(char *,char [], char [], void **, char *);
short _mw_save_anytype(char *, char [], char [], char [], void *, char *);
char ** _mw_list_of_file_type(char *);
void * _mw_convert_struct(void *, char *, char *);
char ** _mw_list_of_convert_struct(char *);
void _mw_delete_anytype(void *, char *);
int _mw_get_info_anytype(void *, char *, int *, int *, char *, char *, 
	void **, void **, void **, int *);
void _mw_put_info_anytype(void *, char *, int, int, char *, char *, void *,
	void *, void *, int);
short _mw_display_anytype(Wframe *, void *, char *, int, int, int, int);

/* External functions (in XMW library) */
extern _mw_display_GrayImage8bits(unsigned long, unsigned char *, int, int, int, 
				int, int);
extern _mw_display_ColorImage24bits(unsigned long, unsigned char *, unsigned char *,
		unsigned char *, int, int, int, int, int);
		

#else

short _mw_load_anytype();
short _mw_save_anytype();
char ** _mw_list_of_file_type();
void * _mw_convert_struct();
char ** _mw_list_of_convert_struct();
void _mw_delete_anytype();
int _mw_get_info_anytype();
void _mw_put_info_anytype();
short _mw_display_anytype();

/* External functions (in XMW library) */
extern _mw_display_GrayImage8bits();
extern _mw_display_ColorImage24bits();
	
#endif

#endif

