/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {fkprintfig};
  version = {"1.4"};
  author = {"Lionel Moisan"};
  function = {"Convert fcurves to fig 3.2 polygons (on stdout)"};
  usage = {
   'd':[d=2]->d [1,3]   "display mode: 1=points, 2=lines, 3=both",
   'e'->e_flag          "to mark extremal points",
   's'->s_flag          "to symmetrize y coordinate (y -> -y)",
   'm':m->m             "to set magnification factor (default: auto + shift)",
   'r':[r=0.2]->r       "relative size of displayed points",
   in->in               "input Fcurves"
  };
*/
/*----------------------------------------------------------------------
 v1.4 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for fkbox() */

/* fig 3.2 header string */
#define FIG_HEADER \
        "#FIG 3.2\nPortrait\nCenter\nMetric\nA4\n100.00\nSingle\n-2\n1200 2\n"

/* fig 3.2 closed polygon (depth D) with N points (coordinates follow) */
#define FIG_CLOSED_POLYGON(D,N) \
        "2 3 0 1 0 7 %d 0 -1 0.000 0 0 -1 0 0 %d\n",D,N      

/* fig 3.2 nonclosed polygon (depth d) with n points (coordinates follow) */
#define FIG_NONCLOSED_POLYGON(D,N) \
	"2 1 0 1 0 7 %d 0 -1 0.000 0 0 -1 0 0 %d\n",D,N

/* fig 3.2 black-filled disc (depth D) with center (X,Y) and radius R */
#define FIG_DISC(D,X,Y,R) \
        "1 3 0 1 0 7 %d 0 10 0.000 1 0.0000 %d %d %d %d %d %d %d %d\n",\
        D,X,Y,R,R,X,Y,X+R,Y

/* size of desired bounding box */
#define BB_SIZE 1.0e5


/*---------- MAIN MODULE ----------*/

void fkprintfig(Fcurves in, int *d, char *e_flag, char *s_flag, float *m, float *r)
{
  Fcurve c;
  Point_fcurve p,last;
  float shift_x,shift_y,zoom_x,zoom_y,dx,dy;
  int nx,ny,n,radius;

  /*----- compute shift and zoom -----*/

  if (m) {
    /* manually set magnification factor -> no translation */
    zoom_x = *m;
    shift_x = shift_y = 0.0;
  } else {
    /* automatic zoom */
    if (s_flag) {
      fkbox(in,&shift_x,&dy,&dx,&shift_y,NULL,NULL);
      dy = shift_y-dy;
    } else {
      fkbox(in,&shift_x,&shift_y,&dx,&dy,NULL,NULL);
      dy -= shift_y;
    }
    dx -= shift_x;
    if (dx>0 || dy>0) {
      zoom_x = BB_SIZE/(dx>dy?dx:dy);
      if (zoom_x>1.5) zoom_x = (float)floor((double)zoom_x);
      else if (zoom_x>0.5) zoom_x = 1.0;
    } else zoom_x = 1.0;
  }
  mwdebug("Magnification factor is %f\n",zoom_x);

  /* compute zoom on y and radius of points */
  zoom_y = (s_flag?-1.0:1.0)*zoom_x;
  radius = ceil((double)(zoom_x*(*r)));


  /*----- Print XFIG header (default parameters) -----*/

  printf(FIG_HEADER);


  /*----- MAIN LOOP -----*/

  for (c=in->first;c;c=c->next) {

    /* compute number of points */
    for (n=0,p=c->first;p;p=p->next) last=p,n++;
    
    if (*d>=2) { 

      /* LINES */

      /* compute number of points */
      for (n=0,p=c->first;p;p=p->next) last=p,n++;
      
      if (last->x==c->first->x && last->y==c->first->y) 
	printf(FIG_CLOSED_POLYGON(100,n));
      else printf(FIG_NONCLOSED_POLYGON(100,n));
      for (p=c->first;p;p=p->next) {
	nx = floor(zoom_x * (p->x - shift_x) + .5);
	ny = floor(zoom_y * (p->y - shift_y) + .5);
	printf("%d %d ", nx, ny);
      }
      printf("\n");
    }
     
    if (*d==1 || *d==3) {

      /* POINTS */
      for (p=c->first;p;p=p->next) {
	nx = floor(zoom_x * (p->x - shift_x) + .5);
	ny = floor(zoom_y * (p->y - shift_y) + .5);
	printf(FIG_DISC(101, nx, ny, radius));
      }

    } 

    if (e_flag) {

      /* EXTREMAL POINTS */
      p=c->first;
      nx = floor(zoom_x * (p->x - shift_x) + .5);
      ny = floor(zoom_y * (p->y - shift_y) + .5);
      printf(FIG_DISC(102, nx, ny, radius*4));

      while (p->next) p=p->next;
      nx = floor(zoom_x * (p->x - shift_x) + .5);
      ny = floor(zoom_y * (p->y - shift_y) + .5);
      printf(FIG_DISC(102, nx, ny, radius*4));

    }
    
  }
}
