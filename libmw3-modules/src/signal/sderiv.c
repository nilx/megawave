/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {sderiv};
  version = {"0.1"};
  author = {"Jacques Froment"};
  function = {"Compute the discrete derivative of a fsignal (basic example)"};
  usage = {
     fsignal_in->A   "input fsignal",
     fsignal_out<-B  "output fsignal"
  };
*/

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"

void sderiv(Fsignal A, Fsignal B)
{
    int i, size;

    size = A->size;
    B = mw_change_fsignal(B, size);
    if (B == NULL)
        mwerror(FATAL, 1, "Not enough memory.");

    mw_copy_fsignal_header(A, B);

    for (i = 1; i < size; i++)
        B->values[i] = (float) (A->values[i] - A->values[i - 1]);
    B->values[0] = 0.0;

}
