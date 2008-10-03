/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {demohead3};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Demo of MegaWave2 header - #3: variable and notused arguments -"};
  usage = {
    A->Input            "Input (could be a cimage)",
    ...<-Output         "Output (copy of the input)",
    notused -> Win      "Window (internal use only)" 
          };
*/
/*--- MegaWave2 - Copyright (C)1994 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

/* Include the window since we use windows facility */
#include "window.h" 

void demohead3(Input,Output,Win)

Cimage Input;
Cimage *Output;  /* Here we define *Output since the function changes the  */
                 /* pointer value (we set bellow *Output = Input)          */

char *Win;       /* Should be "Wframe *Win" for a MegaWave2 window         */
                 /* BUG: We cannot use other type than scalar or MegaWave2 */
                 /* Don't forget to cast before the function call          */
{
  if (Win != NULL)
    {
      printf("Library call: passing Window ptr\n");
      /*
	 ...
      */
    }
  else printf("Command call: no Window ptr\n");

  if (*Output == NULL) printf("No output requested !\n");
  else 
    *Output = Input;  
}






