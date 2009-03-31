/*--------------------------- MegaWave2 Module -----------------------------*/
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

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"

void ccopy(Cimage Input, Cimage * Output)
{
    if (*Output == NULL)
        mwerror(USAGE, 1, "At least one Output requested\n");
    *Output = Input;
}
