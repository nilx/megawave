/*----------------------------- MegaWave Module -----------------------------*/
/* mwcommand
  name = {genxmw2};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Generate the XMegaWave2 body main program from a hierarchy of modules"};
  usage = {
    A->Input  "Ascii file of MegaWave2 modules (input)",
    B<-Output "XMegaWave2 body C main program (output)"
    };
*/
/*--- MegaWave2 - Copyright (C)1994 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"
#include "module.h"

void genxmw2(Input,Output)

Modules Input,*Output;

{
  *Output = Input; 
}



