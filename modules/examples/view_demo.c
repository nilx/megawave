/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {view_demo};
  version = {"1.1"};
  author = {"Jacques Froment"};
  function = {"Demo of a user's lib call to the view modules: View images"};
  usage = {
  'x':[pos_x=50]->x0    "X coordinate for the upper-left corner of the window",
  'y':[pos_y=50]->y0    "Y coordinate for the upper-left corner of twe Window",
  'z':[zoom=1.0]->zoom  "Zoom factor (float value)",
  cimage->cimage        "Input cimage",
  ccimage->ccimage      "Input ccimage"
  };
*/
/*----------------------------------------------------------------------
 v1.1: return void (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"

/* Include the window since we use windows facility */
#include "window.h"


void view_demo(cimage,ccimage,x0,y0,zoom)

     int *x0,*y0;
     float *zoom;
     Cimage cimage;
     Ccimage ccimage;

{
 Wframe *ImageWindow;
 int order=0,no_refresh=1;

 /* FIRST DISPLAY in the window #1 */

 printf("Call to cview and display in the window #1...\n");
 ImageWindow = (Wframe *)
   mw_get_window((Wframe *) NULL,cimage->ncol,cimage->nrow,*x0,*y0,
		 cimage->name);
 cview(cimage,x0,y0,zoom,&order,&no_refresh,ImageWindow);
 sleep(2);

 /* SECOND DISPLAY in another window #2 */

 printf("Call to cview and display in the window #2...\n");
 *x0 += 500;
 cview(cimage,x0,y0,zoom,&order,&no_refresh,NULL);
 sleep(2);

 /* THIRD DISPLAY in the first window #1 */

 printf("Call to ccview and display in the window #1...\n");
 *x0 -= 500;
 ImageWindow = (Wframe *)
   mw_get_window(ImageWindow,ccimage->ncol,ccimage->nrow,*x0,*y0,
		 ccimage->name);
 ccview(ccimage,x0,y0,zoom,&order,(int *) NULL,ImageWindow);

}







