/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {view_demo};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Demo of a user's lib call to the view modules: View images"};
  usage = {
  'x':[pos_x=50]->x0
      "X coordinate for the upper-left corner of the Window",
  'y':[pos_y=50]->y0
      "Y coordinate for the upper-left corner of the Window",
  'z':[zoom=1.0]->zoom
      "Zoom factor (float value)",
  cimage->cimage
        "Input cimage",
  ccimage->ccimage
        "Input ccimage"
  };
*/
/*--- MegaWave - Copyright (C) 1994 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>

/* Include always the MegaWave2 include file */
#include "mw.h"

/* Include the window since we use windows facility */
#include "window.h"


view_demo(cimage,ccimage,x0,y0,zoom)

int *x0,*y0;
float *zoom;
Cimage cimage;
Ccimage ccimage;

{
 Wframe *ImageWindow;
 int no_refresh=1;

 /* FIRST DISPLAY in the window #1 */

 printf("Call to cview and display in the window #1...\n");
 ImageWindow = (Wframe *)
   mw_get_window((Wframe *) NULL,cimage->ncol,cimage->nrow,*x0,*y0,
		 cimage->name);
 cview(cimage,x0,y0,zoom,&no_refresh,ImageWindow);
 sleep(2);

 /* SECOND DISPLAY in another window #2 */

 printf("Call to cview and display in the window #2...\n");
 *x0 += 500;
 cview(cimage,x0,y0,zoom,&no_refresh,NULL);
 sleep(2);

 /* THIRD DISPLAY in the first window #1 */

 printf("Call to ccview and display in the window #1...\n");
 *x0 -= 500;
 ImageWindow = (Wframe *)
   mw_get_window(ImageWindow,ccimage->ncol,ccimage->nrow,*x0,*y0,
		 ccimage->name);
 ccview(ccimage,x0,y0,zoom,(int *) NULL,ImageWindow);

}







