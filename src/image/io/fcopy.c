/*----------------------------- MegaWave Module -----------------------------*/
/* mwcommand
  name = {fcopy};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Copy of a float image. Use it as format converter"};
  usage = {
    A->Input  "Input (could be a fimage)",
    ...<-Output  "Output (copy of the input)"
    };
*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

void fcopy(Input,Output)

Fimage Input,*Output;

{
  if (*Output == NULL) mwerror(USAGE,1,"At least one Output requested\n");
  *Output = Input; 
}



