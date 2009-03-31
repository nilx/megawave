/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fcopy};
 version = {"1.0"};
 author = {"Jacques Froment"};
 function = {"Copy of a float image. Use it as format converter"};
 usage = {
    A->Input     "Input (could be a fimage)",
    ...<-Output  "Output (copy of the input)"
};
*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"

void fcopy(Fimage Input, Fimage * Output)
{
    if (*Output == NULL)
        mwerror(USAGE, 1, "At least one Output requested\n");
    *Output = Input;
}
