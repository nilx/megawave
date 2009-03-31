/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {ml_reconstruct};
 version = {"3.7"};
 author = {"Georges Koepfler"};
 function = {"Reconstruct image from morpho_lines of mimage"};
 usage = {
  'i'->v_flag        "use the maxvalue of the morpho_lines (default minvalue)",
  m_image->m_image           "input mimage",
  image_out<-ml_reconstruct  "reconstructed fimage"
};
*/
/*----------------------------------------------------------------------
 v3.7: upgrade for new kernel (L.Moisan)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "mw.h"
#include "mw-modules.h"

struct fill_segment {
    int y, xl, xr, dy;
};
/* Represents    filled segment [(xl,y),(xr,y)]        */
/* Parent    -filled-   segment [(xl,y-dy),(xr,y-dy)]  */
/* Child -to be filled- segment [(xl,y+dy),(xr,y+dy)]  */

#define FALSE 0
#define TRUE  1

#define POINT_OK(P,Y,X)  (((P)->x>=0)&&((P)->x<=X)&&((P)->y>=0)&&((P)->y<=Y))
#define BAD_POINT(P,Y,X) (!POINT_OK(P,Y,X))
#define INC_OK(D1,D2)    (((abs(D1)==1)&&(D2==0))||((D1==0)&&(abs(D2)==1)))
#define BAD_INC(D1,D2)   (!INC_OK(D1,D2))

#define MAX(A,B)         (((A) > (B)) ?  (A) : (B))

#define N_EQUAL(A,B)     (((A)<(B))||((A)>(B)))
#define EQUAL(A,B)       (!N_EQUAL(A,B))
#define GET_VALUE(A)     ((v_flag==NULL)? (A)->minvalue : (A)->maxvalue)

/* we even allow  dumbbells */
#define FLIP_FLOP(A)     ((((A)==TRUE)?FALSE:TRUE))

/* push segment on stack ST */
#define ST_PUSH(Y,XL,XR,DY) \
     if((st_ptr<(ST+size_ST))&&((Y)+(DY)>=0)&&((Y)+(DY)<NL)) \
               {st_ptr->y=(Y);   st_ptr->dy=(DY); \
                st_ptr->xl=(XL); st_ptr->xr=(XR); \
                st_ptr++;}

/* pop segment off stack ST */
#define ST_POP(Y,XL,XR,DY) \
     { st_ptr--; \
       Y=st_ptr->y+(DY=st_ptr->dy); \
       XL=st_ptr->xl; XR=st_ptr->xr;}

/* test vertical connectedness */
#define TEST_v(Y,X,DY) \
         (  (  (((DY)== 1)&&((Y)<NL)&&(H[(Y)-1][(X)]==FALSE)) \
             ||(((DY)==-1)&&((Y)>=0)&&(H[(Y)  ][(X)]==FALSE)) ) \
          &&(N_EQUAL(im[(Y)][(X)],current_level)) \
          &&(EQUAL(im[(Y)][(X)],p_level))  )

/* test horizontal connectedness */
#define TEST_h(Y,X,DX) \
         (  (  (((DX)== 1)&&((X)<NC)&&(V[(Y)][(X)-1]==FALSE)) \
             ||(((DX)==-1)&&((X)>=0)&&(V[(Y)][(X)  ]==FALSE)) ) \
          &&(N_EQUAL(im[(Y)][(X)],current_level)) \
          &&(EQUAL(im[(Y)][(X)],p_level))  )

static void llcheck(Mimage mimage)
{
    Point_curve point;
    Morpho_line ll;
    int NC, NL;

    NC = mimage->ncol;
    NL = mimage->nrow;
    for (ll = mimage->first_ml; ll; ll = ll->next)
    {
        point = ll->first_point;
        while (point != NULL)
        {
            if (BAD_POINT(point, NL, NC))
            {
                mwdebug
                    ("Morpho Line number %d :\n"
                     "   point->x=%d (NC=%d) \t point->y=%d (NL=%d)\n",
                     ll->num, point->x, NC, point->y, NL);
                mwerror(WARNING, 0, "[llcheck] Point out of image.\n");
            }
            point = point->next;
        }
    }
}

static void
draw_HV(Morpho_line lline_ptr, unsigned char **V, unsigned char **H, int NL,
        int NC)
{
    Point_curve point;
    int l, c, dl, dc;

    point = lline_ptr->first_point;
    if (BAD_POINT(point, NL, NC))
    {
        mwdebug
            ("Morpho Line number = %d : "
             "\t First point->x=%d (NC=%d) \t point->y=%d (NL=%d)\n",
             lline_ptr->num, point->x, NC, point->y, NL);
        mwerror(FATAL, 1, "Point out of image.\n");
    }
    l = point->y;
    c = point->x;
    while (point->next != NULL)
    {
        point = point->next;
        if (BAD_POINT(point, NL, NC))
        {
            mwdebug
                ("Morpho Line number = %d : "
                 "\t point->x=%d (NC=%d) \t point->y=%d (NL=%d)\n",
                 lline_ptr->num, point->x, NC, point->y, NL);
            mwerror(FATAL, 1, "Point out of image.\n");
        }
        dl = point->y - l;
        dc = point->x - c;
        if (BAD_INC(dl, dc))
        {
            mwdebug("Morpho Line number = %d : \t (%d,%d) \t (%d,%d)\n",
                    lline_ptr->num, point->previous->x, point->previous->y,
                    point->x, point->y);
            mwerror(FATAL, 1, "Points are not 4-adjacent.\n");
        }
        if (dl == 1)
            V[l][c - 1] = FLIP_FLOP(V[l][c - 1]);
        else if (dl == -1)
            V[l - 1][c - 1] = FLIP_FLOP(V[l - 1][c - 1]);
        else if (dc == 1)
            H[l - 1][c] = FLIP_FLOP(H[l - 1][c]);
        else                    /* dc==-1 */
            H[l - 1][c - 1] = FLIP_FLOP(H[l - 1][c - 1]);
        l += dl;
        c += dc;
    }
    if (lline_ptr->open == 0)
    {                           /* closed polygon */
        dl = lline_ptr->first_point->y - l;
        dc = lline_ptr->first_point->x - c;
        if (BAD_INC(dl, dc))
        {
            mwdebug("Morpho Line number = %d : \t (%d,%d) \t (%d,%d)\n",
                    lline_ptr->num, lline_ptr->first_point->x,
                    lline_ptr->first_point->y, point->x, point->y);
            mwerror(FATAL, 1, "Points are not 4-adjacent.\n");
        }
        if (dl == 1)
            V[l][c - 1] = FLIP_FLOP(V[l][c - 1]);
        else if (dl == -1)
            V[l - 1][c - 1] = FLIP_FLOP(V[l - 1][c - 1]);
        else if (dc == 1)
            H[l - 1][c] = FLIP_FLOP(H[l - 1][c]);
        else                    /* dc==-1 */
            H[l - 1][c - 1] = FLIP_FLOP(H[l - 1][c - 1]);
    }
}

/* "fill" fills (x,y) and all its 4-connected neighbours with color */
/* current_level in image im. The region is delimited by V and H.   */
static void
fill(int y, int x, unsigned char **V, unsigned char **H,
     struct fill_segment *ST, int size_ST, float current_level, float **im,
     int NL, int NC, float p_level)
{
    struct fill_segment *st_ptr;
    int left, right, x1, x2, dy;

    if ((EQUAL(im[y][x], current_level)) || (x < 0) || (x >= NC) || (y < 0)
        || (y >= NL))
        return;

    im[y][x] = current_level;
    /* now get all horizontal neighbours of initial pixel */
    x1 = x - 1;
    while (TEST_h(y, x1, -1))
    {
        im[y][x1] = current_level;
        x1--;
    }
    x1++;
    x2 = x + 1;
    while (TEST_h(y, x2, 1))
    {
        im[y][x2] = current_level;
        x2++;
    }
    x2--;

    /* initialize stack */
    st_ptr = ST;
    ST_PUSH(y, x1, x2, 1) ST_PUSH(y, x1, x2, -1) while (st_ptr > ST)
    {
        ST_POP(y, x1, x2, dy) if (TEST_v(y, x1, dy))
        {
            im[y][x1] = current_level;
            x = x1 - 1;
            while (TEST_h(y, x, -1))
            {
                im[y][x] = current_level;
                x--;
            }
            left = x + 1;
            if (left < x1)
                ST_PUSH(y, left, x1 - 1, -dy) x = x1;
        }
        else
        {
            x = x1 + 1;
            while ((x <= x2) && (H[y - MAX(0, dy)][x] == TRUE))
                x++;            /* (y,x) not connected to pixel (y-dy,x) */
            left = x;
        }
        while (x <= x2)
        {
            im[y][x] = current_level;
            x++;
            while (TEST_h(y, x, 1))
            {
                im[y][x] = current_level;
                x++;
            }
            right = x - 1;
            if (right > x2)
                ST_PUSH(y, x2 + 1, right, -dy)
                    ST_PUSH(y, left, right, dy) if (x < x2)
                {
                    x++;
                    while ((x <= x2) && (H[y - MAX(0, dy)][x] == TRUE))
                        x++;
                    left = x;
                }
                else            /* we have to quit */
                    x++;
        }
    }
}

static void
fill_level(unsigned char **V, unsigned char **H, struct fill_segment *ST,
           int size_ST, Morpho_line lline, char *v_flag, float **im, int NL,
           int NC, float p_level)
{
    float current_level;
    Point_curve p, q;
    int l, c, dl, dc;

    if (!lline)
        return;
    current_level = GET_VALUE(lline);

    while ((lline != NULL) && EQUAL(GET_VALUE(lline), current_level))
    {
        p = lline->first_point;
        if (!p)
            return;
        q = p->next;
        if (!q)
            return;
        dl = q->y - p->y;
        dc = q->x - p->x;
        if (BAD_INC(dl, dc))
        {
            mwdebug("Morpho Line number = %d : \t (%d,%d) \t (%d,%d)\n",
                    lline->num, p->x, p->y, q->x, q->y);
            mwerror(FATAL, 1, "Points are not 4-adjacent.\n");
        }
        if (dl == 0)
        {
            if (dc == 1)
            {
                c = p->x;
                l = p->y - 1;
            }
            else if (dc == -1)
            {
                c = p->x - 1;
                l = p->y;
            }
        }
        else if (dc == 0)
        {
            if (dl == 1)
            {
                c = p->x;
                l = p->y;
            }
            else if (dl == -1)
            {
                c = p->x - 1;
                l = p->y - 1;
            }
        }
        fill(l, c, V, H, ST, size_ST, current_level, im, NL, NC, p_level);
        lline = lline->next;
    }
}

Fimage ml_reconstruct(char *v_flag, Mimage m_image)
{
    unsigned char **V, **H, *cptr, inc_val;
    int NC, NL, i, size_ST;
    float **im, current_level, p_level;
    Fimage image_out = NULL;
    Morpho_line lline_ptr = NULL, lline2;
    struct fill_segment *ST;

    if (m_image == NULL)
    {
        mwerror(WARNING, 1, "Mimage is NULL.");
        return (image_out);
    }
    if (m_image->minvalue >= m_image->maxvalue)
        mwerror(WARNING, 1,
                "Bad level set representation (minvalue>=maxvalue).\n");
    NC = m_image->ncol;
    NL = m_image->nrow;
    if ((NC <= 0) || (NL <= 0))
        mwerror(WARNING, 1, "Check Mimage data.");

    if (m_image->first_ml == NULL)
        mwerror(USAGE, 1, "No morpho_lines in mimage.");
    else
        lline_ptr = m_image->first_ml;

    /* memory for vertical boundaries */
    V = (unsigned char **) malloc(NL * sizeof(unsigned char *));
    V[0] = (unsigned char *) malloc(NL * (NC - 1) * sizeof(unsigned char));
    for (i = 1; i < NL; i++)
        V[i] = V[i - 1] + (NC - 1);

    /* memory for horizontal boundaries */
    H = (unsigned char **) malloc((NL - 1) * sizeof(unsigned char *));
    H[0] = (unsigned char *) malloc((NL - 1) * NC * sizeof(unsigned char));
    for (i = 1; i < NL - 1; i++)
        H[i] = H[i - 1] + NC;

    /* memory for stack for region filling */
    size_ST = NL * NC;          /* definitely be to much */
    ST = (struct fill_segment *) malloc(size_ST *
                                        sizeof(struct fill_segment));

    if ((H[0] == NULL) || (V[0] == NULL) || (ST == NULL))
    {
        free((void *) H[0]);
        free((void *) V[0]);
        free((void *) ST);
        mwerror(FATAL, 1, "Not enough memory.\n");
    }

    if ((image_out = mw_change_fimage(NULL, NL, NC)) == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");
    current_level = GET_VALUE(m_image);
    mw_clear_fimage(image_out, current_level);

    /* get easy access to image_out->gray */
    im = (float **) malloc(NL * sizeof(float *));
    im[0] = image_out->gray;
    for (i = 1; i < NL; i++)
        im[i] = im[i - 1] + NC;

    if (mwdbg == 1)
    {
        mwdebug("Checking mimage in ml_reconstruct (%d level lines)...\n",
                mw_num_morpho_line(m_image->first_ml));
        llcheck(m_image);
        mwdebug("End of checking mimage\n");
    }

    inc_val = TRUE;
    do
    {
        if (inc_val && (((!v_flag) && (current_level > lline_ptr->minvalue))
                        || ((v_flag)
                            && (current_level < lline_ptr->maxvalue))))
            inc_val = FALSE;
        p_level = current_level;
        current_level = GET_VALUE(lline_ptr);
        /* re-initialize V[][],H[][] for each level */
        cptr = V[0] + NL * (NC - 1);
        while (cptr-- > *V)
            *cptr = FALSE;
        cptr = H[0] + (NL - 1) * NC;
        while (cptr-- > *H)
            *cptr = FALSE;
        lline2 = lline_ptr;
        while ((lline_ptr != NULL)
               && EQUAL(GET_VALUE(lline_ptr), current_level))
        {
            draw_HV(lline_ptr, V, H, NL, NC);
            lline_ptr = lline_ptr->next;
        }
        fill_level(V, H, ST, size_ST, lline2, v_flag, im, NL, NC, p_level);
    }
    while (lline_ptr != NULL);
    if (!inc_val)
    {
        if (!v_flag)
            mwerror(WARNING, 1, "Values of level lines not increasing.");
        else
            mwerror(WARNING, 1, "Values of level lines not decreasing.");
    }
    free((void *) V[0]);
    free((void *) H[0]);
    free((void *) V);
    free((void *) H);
    free((void *) ST);
    free((void *) im);
    return (image_out);
}
