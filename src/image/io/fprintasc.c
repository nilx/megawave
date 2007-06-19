/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fprintasc};
 version = {"1.0"};
 author = {"Jacques Froment, Lionel Moisan"};
 function = {"Print a part of a fimage in ascii format"};
 usage = {
    'v'->verbose "display line and column positions",
    input->u     "input cimage",
    {
      x1->x1     "first column",
      y1->y1     "first line",
      x2->x2     "last column",
      y2->y2     "last line"
    }
};
*/

#include <stdio.h>
#include "mw.h"
 
void fprintasc(u,x1,y1,x2,y2,verbose)
     Fimage u;
     int *x1,*y1,*x2,*y2;
     char *verbose;
{
  int x,y,nx,ny;
  int vx1,vy1,vx2,vy2;
  
  nx = u->ncol; 
  ny = u->nrow;
  vx1 = x1?*x1:0;
  vx2 = x2?*x2:nx-1;
  vy1 = y1?*y1:0;
  vy2 = y2?*y2:ny-1;
  
  if (verbose)	    
  {
    printf(" ");
    for (x=vx1;x<=vx2;x++)
      if (x<10) printf(" %11d  ",x); else printf("%13d ",x);
    printf("\n      ");
    for (x=vx1;x<=vx2;x++)
      printf("      |       ");
    }
  for (y=vy1;y<=vy2;y++) 
    {
      if (verbose) printf("\n%3d - ",y);
      for (x=vx1;x<=vx2;x++)
	printf("%+6e ",u->gray[y*nx+x]);
    }
  printf("\n");
  if (verbose) printf("\n");
}

