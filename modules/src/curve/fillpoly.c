/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {fillpoly};
  version = {"1.2"};
  author = {"Jean-Pierre D'Ales"};
  function = {"Fill a Polygon given by its vertices and generate a Cimage"};
  usage = {
  'x':[size_x=256]->dx  "size in x (nb of columns) of the output Cimage",
  'y':[size_y=256]->dy  "size in y (nb of rows) of the output Cimage",
  Polygon->poly         "Input Polygon",
  Cimage<-bitmap        "bitmapped Cimage of the filled polygon"
  };
*/
/*----------------------------------------------------------------------
 v1.1: ending delete_polygon() call removed until bug is fixed (L.Moisan)
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"

#define NCHANNEL_POLYG 1   /* Nuber of channels in polygons */
#define BG_SYMB 255        /* Symbol for background in bitmap */
#define FILL_SYMB 254      /* Symbol for filling of contour */

#define min(A,B)     (((A)>(B)) ? (B) : (A))
#define max(A,B)     (((A)>(B)) ? (A) : (B))
#define iabs(A)      (((A)>=(0)) ? (A) : -(A))


static void FILL_CONTOUR(Cimage bitmap, Polygon contour)

                                     /* Output bitmap image */
                                     /* Contour of polygon */

{
  Point_curve     current_point;      /* Pointer to the current curve point */
  unsigned char  *ptri1, *ptri2;      /* Pointers to the image values */
  int             r, c;		      /* indices of row and column in 
				       * bitmap for the current point */
  int             nrow, ncol;	      /* Number of rows and columns bitmap */
  char            dir;		      /* Direction of the curve 
				       * at the current point*/
  int             npoints;	      /* Total number of edge points 
				       * in morpho line */

  nrow = bitmap->nrow;
  ncol = bitmap->ncol;

  /*--- First pass ---*/

  current_point = contour->first;
  c = current_point->x;
  r = current_point->y;
  ptri1 = bitmap->gray;
  ptri1 += r * ncol + c;
  npoints = 1;

  current_point = current_point->next;
  if (current_point != NULL) 
  {
    if (current_point->x - c == 1) 
      dir = 1;
    else if (current_point->x - c == - 1) 
      dir = 3;
    else if (current_point->y - r == 1) 
      dir = 2;
    else if (current_point->y - r != - 1)
      mwerror(FATAL, 2, "Something wrong in filling morpho set (erase borders) :\nsuccessive points in morpho lines are not neighbours!\n r1 = %d, c1 = %d, r2 = %d, c2 = %d, line = %d, point = %d\n", r, c, current_point->y, current_point->x, npoints);
    else
      dir = 0;
  }
 
  while (current_point != NULL) {
    npoints++;
    if (current_point->x - c == 1) {

      /*--- Contour goes right ---*/

      if (*ptri1 == 0) {
	r--;
	if (r >= 0) 
	  ptri2 = ptri1 - ncol;
	while ((r >= 0) && (*ptri2 >= FILL_SYMB)) {
	  *ptri2 = FILL_SYMB;
	  r--;
	  if (r >= 0) 
	    ptri2 -= ncol;
	}

	if (dir == 0) {
	  c = current_point->previous->x;
	  r = current_point->previous->y;
	  c--;
	  if (c >= 0) 
	    ptri2 = ptri1 - 1;
	  while ((c >= 0) && (*ptri2 >= FILL_SYMB)) {
	    *ptri2 = FILL_SYMB;
	    c--;
	    if (c >= 0) 
	      ptri2--;
	  }
	}
      }      

      dir = 1;
      ptri1++;

    } else
      if (current_point->x - c == - 1) {

	/*--- Contour goes left ---*/

	if (*ptri1 == 0) {
	  r++;
	  if (r < nrow) 
	    ptri2 = ptri1 + ncol;
	  while ((r < nrow) && (*ptri2 >= FILL_SYMB)) {
	    *ptri2 = FILL_SYMB;
	    r++;
	    if (r < nrow) 
	      ptri2 += ncol;
	  }

	  if (dir == 2) {
	    c = current_point->previous->x;
	    r = current_point->previous->y;
	    c++;
	    if (c < ncol) 
	      ptri2 = ptri1 + 1;
	    while ((c < ncol) && (*ptri2 >= FILL_SYMB)) {
	      c++;
	      *ptri2 = FILL_SYMB;
	      if (c < ncol) 
		ptri2++;
	    }
	  }
	}

	dir = 3;
	ptri1--;

      } else
	if (current_point->y - r == 1) {
	  
	  /*--- Contour goes down ---*/

	  if (*ptri1 == 0) {
	    c++;
	    if (c < ncol) 
	      ptri2 = ptri1 + 1;
	    while ((c < ncol) && (*ptri2 >= FILL_SYMB)) {
	      c++;
	      *ptri2 = FILL_SYMB;
	      if (c < ncol) 
		ptri2++;
	    }

	    if (dir == 1) {
	      c = current_point->previous->x;
	      r = current_point->previous->y;
	      r--;
	      if (r >= 0) 
		ptri2 = ptri1 - ncol;
	      while ((r >= 0) && (*ptri2 >= FILL_SYMB)) {
		*ptri2 = FILL_SYMB;
		r--;
		if (r >= 0) 
		  ptri2 -= ncol;
	      }
	    }	  
	  }
	  dir = 2;
	  ptri1 += ncol;

	} else
	  if (current_point->y - r != - 1)
	    mwerror(FATAL, 2, "Something wrong in filling morpho set :\nsuccessive points in morpho lines are not neighbours!\n r1 = %d, c1 = %d, r2 = %d, c2 = %d, point = %d\n", r, c, current_point->y, current_point->x, npoints);
	  else
	    {

	      /*--- Contour goes up ---*/

	      if (*ptri1 == 0) {
		c--;
		if (c >= 0) 
		  ptri2 = ptri1 - 1;
		while ((c >= 0) && (*ptri2 >= FILL_SYMB)) {
		  *ptri2 = FILL_SYMB;
		  c--;
		  if (c >= 0) 
		    ptri2--;
		}

		if (dir == 3) {
		  c = current_point->previous->x;
		  r = current_point->previous->y;
		  r++;
		  if (r < nrow) 
		    ptri2 = ptri1 + ncol;
		  while ((r < nrow) && (*ptri2 >= FILL_SYMB)) {
		    *ptri2 = FILL_SYMB;
		    r++;
		    if (r < nrow) 
		      ptri2 += ncol;
		  }
		}
	      }

	      dir = 0;
	      ptri1 -= ncol;
	    }
  
    c = current_point->x;
    r = current_point->y;
    current_point = current_point->next;
  }


  /*--- Second pass ---*/

  current_point = contour->first;
  c = current_point->x;
  r = current_point->y;
  ptri1 = bitmap->gray;
  ptri1 += r * ncol + c;
  npoints = 1;

  current_point = current_point->next;
  if (current_point != NULL) 
  {
    if (current_point->x - c == 1) 
      dir = 1;
    else if (current_point->x - c == - 1) 
      dir = 3;
    else if (current_point->y - r == 1) 
      dir = 2;
    else if (current_point->y - r != - 1)
      mwerror(FATAL, 2, "Something wrong in filling morpho set (erase borders) :\nsuccessive points in morpho lines are not neighbours!\n r1 = %d, c1 = %d, r2 = %d, c2 = %d, line = %d, point = %d\n", r, c, current_point->y, current_point->x, npoints);
    else
      dir = 0;
  }

  while (current_point != NULL) {
    npoints++;
    if (current_point->x - c == 1) {
      r--;
      if (r >= 0) 
	ptri2 = ptri1 - ncol;
      while ((r >= 0) && (*ptri2 <= FILL_SYMB)) {
	*ptri2 = 0;
	r--;
	if (r >= 0) 
	  ptri2 -= ncol;
      }

      if (dir == 0) {
	c = current_point->previous->x;
	r = current_point->previous->y;
	c--;
	if (c >= 0) 
	  ptri2 = ptri1 - 1;
	while ((c >= 0) && (*ptri2 <= FILL_SYMB)) {
	  *ptri2 = 0;
	  c--;
	  if (c >= 0) 
	    ptri2--;
	}
      }
      
      dir = 1;
      ptri1++;

    } else
      if (current_point->x - c == - 1) {
	r++;
	if (r < nrow) 
	  ptri2 = ptri1 + ncol;
	while ((r < nrow) && (*ptri2 <= FILL_SYMB)) {
	  *ptri2 = 0;
	  r++;
	  if (r < nrow) 
	    ptri2 += ncol;
	}

	if (dir == 2) {
	  c = current_point->previous->x;
	  r = current_point->previous->y;
	  c++;
	  if (c < ncol) 
	    ptri2 = ptri1 + 1;
	  while ((c < ncol) && (*ptri2 <= FILL_SYMB)) {
	    c++;
	    *ptri2 = 0;
	    if (c < ncol) 
	      ptri2++;
	  }
	}
	
	dir = 3;
	ptri1--;

      } else
	if (current_point->y - r == 1) {
	  c++;
	  if (c < ncol) 
	    ptri2 = ptri1 + 1;
	  while ((c < ncol) && (*ptri2 <= FILL_SYMB)) {
	    c++;
	    *ptri2 = 0;
	    if (c < ncol) 
	      ptri2++;
	  }

	  if (dir == 1) {
	    c = current_point->previous->x;
	    r = current_point->previous->y;
	    r--;
	    if (r >= 0) 
	      ptri2 = ptri1 - ncol;
	    while ((r >= 0) && (*ptri2 <= FILL_SYMB)) {
	      *ptri2 = 0;
	      r--;
	      if (r >= 0) 
		ptri2 -= ncol;
	    }
	  }	  

	  dir = 2;
	  ptri1 += ncol;

	} else
	  if (current_point->y - r != - 1)
	      mwerror(FATAL, 2, "Something wrong in filling morpho set :\nsuccessive points in morpho lines are not neighbours!\n r1 = %d, c1 = %d, r2 = %d, c2 = %d, point = %d\n", r, c, current_point->y, current_point->x, npoints);
	    else
	      {
		c--;
		if (c >= 0) 
		  ptri2 = ptri1 - 1;
		while ((c >= 0) && (*ptri2 <= FILL_SYMB)) {
		  *ptri2 = 0;
		  c--;
		  if (c >= 0) 
		    ptri2--;
		}

		if (dir == 3) {
		  c = current_point->previous->x;
		  r = current_point->previous->y;
		  r++;
		  if (r < nrow) 
		    ptri2 = ptri1 + ncol;
		  while ((r < nrow) && (*ptri2 <= FILL_SYMB)) {
		    *ptri2 = 0;
		    r++;
		    if (r < nrow) 
		      ptri2 += ncol;
		  }
		}
	    
		dir = 0;
		ptri1 -= ncol;
	      }
  
    c = current_point->x;
    r = current_point->y;
    current_point = current_point->next;
  }

}


static void
CHANGE_ORIENTATION_CONTOUR(Polygon contour)

                                     /* Contour of polygon */

{
  Point_curve     point_begin, point_end;  /* Pointers to the beginning and  
				      * the end of the curve */
int             r, c;                /* indices of row and column 
				      * for the current point */

  point_begin = contour->first;
  point_end = contour->first;
  while (point_end->next) 
    point_end = point_end->next;
    
  while ((point_begin != point_end) && (point_begin->next != point_end)) {
    r = point_begin->y;
    c = point_begin->x;
    point_begin->y = point_end->y;
    point_begin->x = point_end->x;
    point_end->y = r;
    point_end->x = c;
    point_begin = point_begin->next;
    point_end = point_end->previous;
  }

}



static char
GET_ORIENTATION_CONTOUR(Cimage bitmap, Polygon contour)

  /*--- Return 0 if inner side is on he left, else return 1 ---*/

                                     /* Output bitmap image */
                                     /* Contour of polygon */

{
  Point_curve     current_point;      /* Pointer to the current curve point */
  int             r, c;		      /* indices of row and column in 
				       * bitmap for the current point */
  int             nrow, ncol;	      /* Number of rows and columns bitmap */
  char            dir;		      /* Direction of the curve 
				       * at the current point*/
  char            test_found;         /* Flag to indicate if orientation
				       * has been found or not */

  test_found = FALSE;
  nrow = bitmap->nrow;
  ncol = bitmap->ncol;
  current_point = contour->first->next;
  while ((current_point) && (test_found == FALSE)) {
    c = current_point->x;
    r = current_point->y;
    if (bitmap->gray[r * ncol + c] == 0)  {
      r = 0;
      while ((r < current_point->y) && (bitmap->gray[r * ncol + c] == BG_SYMB))
	r++;
      if (r == current_point->y) 
      {
	if (current_point->next) 
	{
	  if (((current_point->previous->x >= c) && (current_point->next->x < c)) || ((current_point->previous->x > c) && (current_point->next->x <= c)))
	  {
	    dir = 0;
	    test_found = TRUE;
	  }
	  else if (((current_point->previous->x <= c) && (current_point->next->x > c)) || ((current_point->previous->x < c) && (current_point->next->x >= c)))
	  {
	    dir = 1;
	    test_found = TRUE;
	  }
	}
      }

      if (test_found == FALSE) {
	c = current_point->x;
	r = current_point->y;
	c = 0;
	while ((c < current_point->x) && (bitmap->gray[r * ncol + c] == BG_SYMB))
	  c++;
	if (c == current_point->x) 
	{
	  if (current_point->next)
	  { 
	    if (((current_point->previous->y >= r) && (current_point->next->y < r)) || ((current_point->previous->y > r) && (current_point->next->y <= r)))
	    {
	      dir = 1;
	      test_found = TRUE;
	    }
	    else if (((current_point->previous->y <= r) && (current_point->next->y > r)) || ((current_point->previous->y < r) && (current_point->next->y >= r)))
	    {
	      dir = 0;
	      test_found = TRUE;
	    }
	  }
	}
      }
    }

    current_point = current_point->next;
  }
  
  if (test_found == FALSE)
    mwerror(WARNING, 0, "Could not find orientation!\n");
  
  return(dir);
}




static int
GET_NPOINTS_CONTOUR(Polygon poly)

                                 /* Input polygon */

{
  int              n;
  int              x, y;
  int              dx, dy;
  Point_curve      ptr_point;	    /* Pointer to the current point 
				     * in current polygon */

  n = 1;
  ptr_point = poly->first;
  x = ptr_point->x;
  y = ptr_point->y;
  ptr_point = ptr_point->next;
  while (ptr_point) {
    dx = iabs(x - ptr_point->x);
    dy = iabs(y - ptr_point->y);
    n += dx;
    n += dy;
    x = ptr_point->x;
    y = ptr_point->y;
    ptr_point = ptr_point->next;
  }
  ptr_point = poly->first;
  dx = iabs(x - ptr_point->x);
  dy = iabs(y - ptr_point->y);
  n += dx;
  n += dy;

  return(n);
}



static void
DRAW_CONTOUR(Cimage bitmap, Polygon contour)

                                 /* bitmap image */
                                 /* Contour of polygon */

{
  Point_curve      ptr_point;	    /* Pointer to the current point 
				     * in contour */
  int              ncol;
  unsigned char   *ptrb;

  ncol = bitmap->ncol;
  ptr_point = contour->first;

  while (ptr_point) {

    if ((ptr_point->x < 0) || (ptr_point->x >= bitmap->ncol) || (ptr_point->y < 0) || (ptr_point->y >= bitmap->nrow))
	  mwerror(FATAL, 2, "Vertex of polygon outside of image!\nr = %d, c = %d, nrow = %d, ncol = %d.\n", ptr_point->y, ptr_point->x, bitmap->nrow, bitmap->ncol);

    ptrb = bitmap->gray + (ptr_point->y * ncol + ptr_point->x);
    if (*ptrb == BG_SYMB) {
      *ptrb = 0;

      /*--- Check if point is the extremity of a segment ---*/
            /*--- or the vertex of a sharp corner ---*/ 

      if (ptr_point->next && ptr_point->previous)
	if ((ptr_point->next->x == ptr_point->previous->x) && (ptr_point->next->y == ptr_point->previous->y))
	  *ptrb = 1;
    } else

      /*--- Contour has already passed through this point ---*/ 

      if (ptr_point->next)
	(*ptrb)++;
      else
	if ((contour->first->next->x == ptr_point->previous->x) && (contour->first->next->y == ptr_point->previous->y)) 
	  (*ptrb)++;
	
    ptr_point = ptr_point->next;
  }

}


static void
ADD_SEGMENT(int x1, int y1, int x2, int y2, Point_curve *cont_point)

                                 /* Coordinate of first extremity */
                                 /* Coordinate of second extremity */
                                 /* Pointer to the current point in contour */
                                 /* Table of points for memory 
				  * allocation */ 
	     
{
  Point_curve      ptr_point;	    /* Pointer to the current point 
				     * in contour */
  int              x, y;
  int              dx, dy;
  double           m, c;
  double           z;

  dx = iabs(x2 - x1);
  dy = iabs(y2 - y1);
  ptr_point = *cont_point;

  if ((dx > 0) || (dy > 0))
  {
    if (dx >= dy)
    {
      m = (double) (y2 - y1) / (x2 - x1);
      c = (double) y1 - m * (double) x1;
      if (x1 < x2)
      {
	for (x = x1 + 1; x <= x2; x++)
	{
	  z = m * x + c;
	  y = floor(z + .5);
	  if (y != ptr_point->y)
	  {
	    ptr_point->next = ptr_point + 1;
	    ptr_point++;
	    ptr_point->previous = ptr_point - 1;
	    ptr_point->x = x;
	    ptr_point->y = ptr_point->previous->y;
	  }
	  ptr_point->next = ptr_point + 1;
	  ptr_point++;
	  ptr_point->previous = ptr_point - 1;
	  ptr_point->x = x;
	  ptr_point->y = y;
	}
      }
      else
      {
	for (x = x1 - 1; x >= x2; x--)
	{
	  z = m * x + c;
	  y = floor(z + .5);
	  if (y != ptr_point->y)
	  {
	    ptr_point->next = ptr_point + 1;
	    ptr_point++;
	    ptr_point->previous = ptr_point - 1;
	    ptr_point->x = x;
	    ptr_point->y = ptr_point->previous->y;
	  }
	  ptr_point->next = ptr_point + 1;
	  ptr_point++;
	  ptr_point->previous = ptr_point - 1;
	  ptr_point->x = x;
	  ptr_point->y = y;
	}
      }
    }
    else
    {
      m = (double) (x2 - x1) / (y2 - y1);
      c = (double) x1 - m * (double) y1;
      if (y1 < y2)
      {
	for (y = y1 + 1; y <= y2; y++)
	{
	  z = m * y + c;
	  x = floor(z + .5);
	  if (x != ptr_point->x)
	  {
	    ptr_point->next = ptr_point + 1;
	    ptr_point++;
	    ptr_point->previous = ptr_point - 1;
	    ptr_point->x = ptr_point->previous->x;
	    ptr_point->y = y;
	  }
	  ptr_point->next = ptr_point + 1;
	  ptr_point++;
	  ptr_point->previous = ptr_point - 1;
	  ptr_point->x = x;
	  ptr_point->y = y;
	}
      }
      else
      {
	for (y = y1 - 1; y >= y2; y--)
	{
	  z = m * y + c;
	  x = floor(z + .5);
	  if (x != ptr_point->x)
	  {
	    ptr_point->next = ptr_point + 1;
	    ptr_point++;
	    ptr_point->previous = ptr_point - 1;
	    ptr_point->x = ptr_point->previous->x;
	    ptr_point->y = y;
	  }
	  ptr_point->next = ptr_point + 1;
	  ptr_point++;
	  ptr_point->previous = ptr_point - 1;
	  ptr_point->x = x;
	  ptr_point->y = y;
	}
      }
    }
  }
  ptr_point->next = NULL;
  *cont_point = ptr_point;
}




static void
MAKE_CONTOUR(Cimage bitmap, Polygon poly, Polygon contour, Point_curve *p)

                                 /* bitmap image */
                                 /* Input polygnal */
                                 /* Contour of polygon */
                                 /* Table of points for memory 
				  * allocation */ 

{
  Point_curve   cont_point;      /* Pointer to the current point in contour */
  Point_curve   poly_point;      /* Pointer to the current point in polygon */
  int           x1, y1;          /* Coordinate of first extremity */
  int           x2, y2;          /* Coordinate of second extremity */

  contour->first = *p;
  cont_point = contour->first;
  cont_point->previous = NULL;
  poly_point = poly->first;
  if ((poly_point->x < 0) || (poly_point->x >= bitmap->ncol) || (poly_point->y < 0) || (poly_point->y >= bitmap->nrow))
	  mwerror(FATAL, 2, "Vertex of polygon outside of image!\nr = %d, c = %d, nrow = %d, ncol = %d.\n", poly_point->y, poly_point->x, bitmap->nrow, bitmap->ncol);
  x1 = poly_point->x;
  y1 = poly_point->y;
  poly_point = poly_point->next;
  cont_point->x = x1;
  cont_point->y = y1;
    
  while (poly_point) {

    if ((poly_point->x < 0) || (poly_point->x >= bitmap->ncol) || (poly_point->y < 0) || (poly_point->y >= bitmap->nrow))
      mwerror(FATAL, 2, "Vertex of polygon outside of image!\nr = %d, c = %d, nrow = %d, ncol = %d.\n", poly_point->y, poly_point->x, bitmap->nrow, bitmap->ncol);
    
    /*--- Draw one segment ---*/

    x2 = poly_point->x;
    y2 = poly_point->y;

    ADD_SEGMENT(x1, y1, x2, y2, &cont_point);

    x1 = poly_point->x;
    y1 = poly_point->y;
    poly_point = poly_point->next;
  }

  /*--- Draw last segment ---*/

  x2 = poly->first->x;
  y2 = poly->first->y;
  
  ADD_SEGMENT(x1, y1, x2, y2, &cont_point);

  DRAW_CONTOUR(bitmap, contour);
}



void 
fillpoly(int *dx, int *dy, Polygon poly, Cimage bitmap)

                                     /* Size of output bitmap */
                                     /* Input polygon */
                                     /* Output bitmap */

{
  register unsigned char  *ptra;     /* Pointer to significance map */
  long             x;                /* Buffer index for current point 
				      * in bitmap */
  long             size;             /* Size of bitmap */
  Polygon          contour;          /* Contour of current polygon */
  Point_curve      alloc_point;      /* Table of points for memory 
				      * allocation */ 
  int              npoints_cont;     /* Maximum number of points in a 
				      * polygon's contour */
  char             dir;              /* orientation of contour */

  /*--- Memory allocation and initialization for bitmap ---*/

  bitmap = mw_change_cimage(bitmap, *dy, *dx);
  if (bitmap == NULL) 
    mwerror(FATAL,1,"Not enough memory.\n");
  size = bitmap->nrow * bitmap->ncol;
  for (x = 0, ptra = bitmap->gray; x < size; x++, ptra++) 
    *ptra = BG_SYMB;      

  /*--- Memory allocation for contours ---*/

  contour = mw_change_polygon(NULL, NCHANNEL_POLYG);
  if (contour == NULL)
    mwerror(FATAL, 1, "Allocation for selected area contour refused!\n");

  /*--- Memory allocation for contour points ---*/

  npoints_cont = GET_NPOINTS_CONTOUR(poly);
  alloc_point = (Point_curve) malloc(npoints_cont * sizeof(struct point_curve));
  if (alloc_point == NULL)
    mwerror(FATAL, 1, "Not enough memory for polygon's contour points!\n");

  /*--- Compute contour and draw it on bitmap ---*/ 

  MAKE_CONTOUR(bitmap, poly, contour, &alloc_point);

  /*--- Find contour's orientation and change it (if necessary) ---*/
            /*--- so that inner side is on the left ---*/ 

  dir = GET_ORIENTATION_CONTOUR(bitmap, contour);
  if (dir == 1)
    CHANGE_ORIENTATION_CONTOUR(contour);

  /*--- Fill contour in bitmap ---*/

  FILL_CONTOUR(bitmap, contour);

  /* free() crashes on a valid point : 
     there may be an allocation problem in the module */

  /* mw_delete_polygon(contour); */

}








