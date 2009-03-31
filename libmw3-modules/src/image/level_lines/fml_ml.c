/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fml_ml};
 version = {"1.5"};
 author = {"Georges Koepfler"};
 function = {"Transform fmorpho_lines into morpho_lines
              by a (+0.5,+0.5) translation"};
 usage = {
    fmorpho_line_in->flline   "input fmorpho_line",
    morpho_line_out<-fml_ml   "output morpho_line"
};
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "mw3.h"
#include "mw3-modules.h"

Morpho_line fml_ml(Fmorpho_line flline)
{
    Morpho_line lline = NULL, old_lline, first_lline = NULL;
    Point_fcurve fpoint;
    Point_curve point;
    Point_type type, ftype;
    int nb_points, nb_types;

    if (flline == NULL)
        mwerror(FATAL, 1, "No float level lines in input.");

    old_lline = NULL;
    while (flline != NULL)
    {
        for (fpoint = flline->first_point, nb_points = 0; fpoint != NULL;
             fpoint = fpoint->next)
            nb_points++;
        for (ftype = flline->first_type, nb_types = 0; ftype != NULL;
             ftype = ftype->next)
            nb_types++;
        if (nb_points != nb_types)
            mwerror(FATAL, 1, "Points / types mismatch");
        point = (Point_curve) malloc(nb_points * sizeof(struct point_curve));
        type = (Point_type) malloc(nb_types * sizeof(struct point_type));
        if (!(point && type))
            mwerror(FATAL, 1, "Not enough memory.");
        lline = mw_change_morpho_line(NULL);
        lline->minvalue = flline->minvalue;
        lline->maxvalue = flline->maxvalue;
        lline->open = flline->open;
        lline->first_point = point;
        lline->first_type = type;
        lline->previous = old_lline;
        lline->next = NULL;
        for (fpoint = flline->first_point, ftype = flline->first_type;
             fpoint != NULL; fpoint = fpoint->next, ftype = ftype->next)
        {
            point->x = (int) (fpoint->x + .5);
            point->y = (int) (fpoint->y + .5);
            point->previous = (fpoint->previous == NULL) ? NULL : point - 1;
            point->next = (fpoint->next == NULL) ? NULL : point + 1;
            point++;
            type->type = ftype->type;
            type->previous = (ftype->previous == NULL) ? NULL : type - 1;
            type->next = (ftype->next == NULL) ? NULL : type + 1;
            type++;
        }
        if (old_lline == NULL)
            first_lline = lline;
        else
            old_lline->next = lline;
        old_lline = lline;
        flline = flline->next;
    }

    return (first_lline);
}
