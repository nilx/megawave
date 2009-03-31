/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {flprintasc};
   version = {"1.1"};
   author = {"Lionel Moisan"};
   function = {"Print the content of a Flists"};
   usage = {
    'n':n->n  "print only list #n",
    'v'->v    "verbose mode",
    in->in    "input Flists"
   };
*/
/*----------------------------------------------------------------------
 v1.1: added -n and -v options (L.Moisan)
 ----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw3.h"
#include "mw3-modules.h"

void flprintasc(Flists in, char *v, int *n)
{
    Flist l;
    int i, j, k;

    for (i = 0; i < in->size; i++)
        if (!n || i + 1 == *n)
        {
            l = in->list[i];
            if (v)
                printf("\tFlist #%d (dim %d, size %d): \n", i + 1, l->dim,
                       l->size);
            for (j = 0; j < l->size; j++)
            {
                if (v)
                    printf("element #%d : ", j + 1);
                for (k = 0; k < l->dim; k++)
                    printf("%g ", l->values[j * l->dim + k]);
                printf("\n");
            }
            printf("\n");
        }
}
