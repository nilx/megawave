/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  W_X11R4_var.c    Window Device for X11 Release 4,5,6 - Global Variables -
                   This file regroups all miscellaneous WX* commands     
                   (That is, which should not be used as this by the     
                   main functions, but which are called in W_X11R4.c)    
  Vers. 3.1
  (C) 1991-2001 Jacques Froment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~  This file is part of the MegaWave2 Wdevice library ~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Since Wdevice V3, all global variables must begin with _W_ */

Display 	*_W_Display=NULL;	/* -- Which display          */
int		_W_Screen;	/* -- Which screen on the display    */
int		_W_Depth;	/* -- Number of color planes         */
unsigned long	_W_BlackPixel;	/* -- System "Black" color           */
unsigned long	_W_WhitePixel;	/* -- System "White" color           */
Colormap	_W_Colormap;	/* -- default System color map       */
Cursor		_W_Cursor;	/* -- Application program's cursor   */
GC              _W_GC;          /* -- Graphic content to be used     */
Visual          *_W_Visual;     /* -- Visual color structure         */
XFontStruct     *_W_Font;       /* -- Font loaded                    */ 

int _W_KeyBuffer;                /* -- The Key pressed */
int _W_XErrorOccured = 0;        /* -- Used by WIsAnActiveWindow */

/* Color parameters, located into the Wframe structure in Wdevice V2 : */

unsigned char _W_Red[256],_W_Green[256],_W_Blue[256];  /* User ColorMap */
XColor  _W_RGB[256];           /* ColorMap (X11 format) */
int _W_NumCols=0;             /* Number of colors to allocate */
unsigned char _W_special_color; /* The special color (used as a mark) */
                     
                 /* Internal use only */

/* This array is defined in window.c for MegaWave2 */
/* To use the Wdevice library outside MegaWave2, remove the "extern" below */
extern unsigned long _W_cols[256];   /* Maps bitmap pixel values to X pixel vals */

int        _W_nfcols=0;          /* number of colors to free */
unsigned long _W_freecols[256]; /* list of pixel values to free */







