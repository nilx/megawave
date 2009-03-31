/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {flreadasc};
   version = {"1.1"};
   author = {"Lionel Moisan"};
   function = {"Read a Flists in ascii format"};
   usage = {
        dim->dim           "dimension of each Flist",
        output<-flreadasc  "output Flists"
   };
*/

#include <stdlib.h>
#include <stdio.h>
#include "mw3.h"
#include "mw3-modules.h"

Flists flreadasc(int dim)
{
    Flists ls;
    Flist l;
    float x;
    char c;
    int i, end;
    int retval;

    ls = mw_change_flists(NULL, 10, 0);

    printf
        ("Enter the values of each Flist, separated by <space> or <enter>\n");
    printf("End the current Flist by 'e' and the whole Flists with 'q'\n");

    do
    {
        l = mw_change_flist(NULL, 10, 0, dim);
        if (ls->size == ls->max_size)
            mw_enlarge_flists(ls);
        ls->list[ls->size++] = l;
        i = 0;
        do
        {
            end = !scanf("%f", &x);
            if (!end)
            {
                if (i == l->max_size)
                    mw_enlarge_flist(l);
                l->values[i++] = x;
            }
        }
        while (!end);
        l->size = (i + dim - 1) / dim;
        retval = scanf("%c", &c);
        end = (c == 'q' || c == 'Q');
    }
    while (!end);

    printf("%d flist%s entered\n", ls->size, ls->size > 1 ? "s" : "");
    printf("size%s : ", ls->size > 1 ? "s" : "");
    for (i = 0; i < ls->size; i++)
        printf("%d ", ls->list[i]->size);
    printf("\n");

    return ls;
}
