/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {kplotasc};
   version = {"1.0"};
   author = {"Jacques Froment"};
   function = {"Print the geometry of a curve"};
   usage = {
      in->cv    "input Curve"
   };
*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"

#define MAXX 100
#define MAXY 300

void kplotasc(Curve cv)
{
    Point_curve point;
    int minx, maxx, miny, maxy, x, y, bx, by, n;
    char Tab[MAXX][MAXY];       /* Tab[x][y] = 'x' if there is a point */
    int intersect;

    minx = miny = 32700;
    maxx = maxy = -32700;
    intersect = 0;
    for (point = cv->first; point; point = point->next)
    {
        if (point->x < minx)
            minx = point->x;
        if (point->y < miny)
            miny = point->y;
        if (point->x > maxx)
            maxx = point->x;
        if (point->y > maxy)
            maxy = point->y;
    }

    bx = maxx - minx + 1;
    if (bx > MAXX)
        mwerror(FATAL, 1,
                "This curve cannot be printed : too many points (%d) "
                "in X direction\n", bx);

    by = maxy - miny + 1;
    if (by > MAXY)
        mwerror(FATAL, 1,
                "This curve cannot be printed : too many points (%d) "
                "in Y direction\n", by);

    for (x = 0; x < bx; x++)
        for (y = 0; y < by; y++)
            Tab[x][y] = ' ';
    for (point = cv->first, n = 0; point; point = point->next, n++)
    {
        if (n == 10)
            n = 0;

        if (point == cv->first)
        {
            printf("First point * in (%d,%d)\n", point->x, point->y);
            Tab[point->x - minx][point->y - miny] = '*';
        }
        else if (point->next == NULL)
        {
            printf("Last point # (%d) in (%d,%d)\n", n, point->x, point->y);
            Tab[point->x - minx][point->y - miny] = '#';
        }
        else if (Tab[point->x - minx][point->y - miny] != ' ')
        {
            Tab[point->x - minx][point->y - miny] = 'x';
            intersect++;
        }
        else
            Tab[point->x - minx][point->y - miny] = '0' + n;
    }

    if (intersect > 0)
        printf
            ("This curve intersects itself %d times (point(s) marked by 'x')\n",
             intersect);
    for (y = 0; y < by; y++)
    {
        for (x = 0; x < bx; x++)
            printf("%c", Tab[x][y]);
        printf("\n");
    }
}
