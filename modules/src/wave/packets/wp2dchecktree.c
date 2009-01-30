/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {wp2dchecktree};
version = {"1.0"};
author = {"Francois Malgouyres"};
function = {"Check if an image can be considered as a quad-tree for a 2D-wavelet packet decomposition"};
usage = { 
   'd'->display_on_flag     "print the result", 
   Cimage->tree             "Cimage describing (or not) a quad-tree"
};
*/
/*----------------------------------------------------------------------
 v1.0: adaptation from checkTree v0.1 (fpack) (JF)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"

int wp2dchecktree(Cimage tree, char *display_on_flag)
{
  int level;

  level=mw_checktree_wpack2d(tree);

  if(display_on_flag)
    printf("This image has a tree structure. \n Its maximum decomposition level is %d\n",level);
  
  return(level); 
}
