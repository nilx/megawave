/*----------------------------- MegaWave Module -----------------------------*/
/* mwcommand
  name = {ccopy};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Copy of a char image. Use it as format converter"};
  usage = {
    A->Input  "Input (could be a cimage)",
    ...<-Output  "Output (copy of the input)"
    };
*/
/*--- MegaWave2 - Copyright (C)1994 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

void ccopy(Input,Output)

Cimage Input,*Output;

{
  if (*Output == NULL) mwerror(USAGE,1,"At least one Output requested\n");
  *Output = Input; 
}



