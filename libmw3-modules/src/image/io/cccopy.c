/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {cccopy};
 version = {"1.0"};
 author = {"Jacques Froment"};
 function = {"Copy of a color char image. Use it as format converter"};
 usage = {
    A->Input     "Input (could be a ccimage)",
    ...<-Output  "Output (copy of the input)"
};
*/

#include <stdio.h>
#include "mw3.h"
#include "mw3-modules.h"

void cccopy(Ccimage Input, Ccimage * Output)
{
    if (*Output == NULL)
        mwerror(USAGE, 1, "At least one Output requested\n");
    *Output = Input;
}
