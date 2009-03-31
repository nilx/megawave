/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {fkreadasc};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"Read a Fcurves in ascii format"};
   usage = {
        output<-fkreadasc  "output Fcurves"
   };
*/

#include <stdlib.h>
#include <stdio.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for fkprintasc() */

Fcurves fkreadasc(void)
{
    Fcurves cvs;
    Fcurve newcv, cv_previous, *cv_next;
    Point_fcurve newp, p_previous = NULL, *p_next = NULL;
    float x, y;
    int nump, numcv, end;
    char c;
    int retval;

    printf("Type the coordinates of the points, ");
    printf("separated by <space> or <enter>\n");
    printf("End the current curve by 'e' and the whole Fcurves with 'q'\n");

    cvs = mw_new_fcurves();
    cv_next = &(cvs->first);
    cv_previous = NULL;
    numcv = 0;

    do
    {
        nump = 0;
        do
        {
            end = !scanf("%f", &x);
            if (!end)
            {
                if (nump == 0)
                {
                    numcv++;
                    newcv = mw_new_fcurve();
                    newcv->previous = cv_previous;
                    *cv_next = cv_previous = newcv;
                    cv_next = &(newcv->next);
                    p_next = &(newcv->first);
                    p_previous = NULL;
                }
                nump++;
                newp = mw_new_point_fcurve();
                newp->x = x;
                newp->previous = p_previous;
                *p_next = p_previous = newp;
                p_next = &(newp->next);
                y = 0.0;
                retval = scanf("%f", &y);
                newp->y = y;
            }
        }
        while (!end);
        retval = scanf("%c", &c);
        end = (nump == 0 || c == 'q' || c == 'Q');
        if (!end)
            *p_next = NULL;
    }
    while (!end);

    if (numcv == 0)
        mwerror(FATAL, 1, "Empty Fcurves ! ");

    *cv_next = NULL;
    printf("%d curve%s entered\n", numcv, numcv > 1 ? "s" : "");

    fkprintasc(cvs);

    return cvs;
}
