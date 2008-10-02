/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   wpanel.h
   
   Vers. 1.1
   (C) 2001 Lionel Moisan
   Panel display facilities and buttons

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*----------------------------------------------------------------------
 v1.1: non reentrant call and functions declaration added (J. Froment)
----------------------------------------------------------------------*/

#ifndef wpanel_flg
#define wpanel_flg

#define Wp_max_buttons 100  /* maximum number of buttons */
#define WP_STRSIZE 1000  /* maximum string size */

/* colors 64 grey levels + 5x5x5 */

#define WP_BLACK    0
#define WP_GREY    40
#define WP_WHITE   63
#define WP_RED    164
#define WP_BLUE    68
#define WP_GREEN   84


/* wp types */

#define WP_NULL    0
#define WP_TOGGLE  1
#define WP_INT     2
#define WP_FLOAT   3


typedef struct wp_toggle {
  char *text;          /* text to display */
  int color;           /* active color */
  short nbuttons;      /* number of buttons */
  short button;        /* current active button */
  char **button_text;  /* text for each button */
  int x,y      ;       /* position on window (upleft corner) */
  int (*proc)();       /* function to call when value changes (may be NULL) */
} *Wp_toggle;

typedef struct wp_int {
  char *text;          /* text to display */
  char *format;        /* format for int display (eg "%d") */
  int value;           /* value */
  int strsize;         /* internal use (initialize to 0) */
  int scale;           /* length of scale bar (0 means no bar) */
  int firstscale;      /* value of bar left border */
  int lastscale;       /* value of bar right border */
  int divscale;        /* number of bar scale divisions */
  int color;           /* text color */
  short nbuttons;      /* number of buttons */
  char **button_text;  /* text for each button */
  int *button_inc;     /* increment for each button */
  int x,y      ;       /* position on window (upleft corner) */
  int (*proc)();       /* function to call when value changes (may be NULL) */
} *Wp_int;

typedef struct wp_float {
  char *text;          /* text to display */
  char *format;        /* format for int display (eg "%d") */
  float value;         /* value */
  int strsize;         /* internal use (initialize to 0) */
  int color;           /* text color */
  short nbuttons;      /* number of buttons */
  char **button_text;  /* text for each button */
  float *button_inc;   /* increment for each button */
  int x,y      ;       /* position on window (upleft corner) */
  int (*proc)();       /* function to call when value changes (may be NULL) */
} *Wp_float;

typedef struct wpanel {
  Wframe *window;        /* attached window */
  char state;            /* -1 means that window should be closed */
  int nx,ny;             /* size of bitmaps (initial window size) */
  char *type;            /* bitmap (associated wp type) */
  void **action;         /* bitmap (pointer to wp structure) */
  short *button;         /* bitmap (associated button number) */
} *Wpanel;

#ifdef __STDC__
int Wp_DrawButton(Wframe *, int, int, char *, int);
void Wp_DrawScale(Wframe *, int, int, int, int, int, int);
Wpanel Wp_Init(Wframe *);
void Wp_SetButton(int, Wpanel, void *);
int Wp_handle(Wpanel, int, int, int);
int Wp_notify(Wframe *, void *);
void Wp_main_loop(Wpanel);
#else
int Wp_DrawButton();
void Wp_DrawScale();
Wpanel Wp_Init();
void Wp_SetButton();
int Wp_handle();
int Wp_notify();
void Wp_main_loop();
#endif


#endif
