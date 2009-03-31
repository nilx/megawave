/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fvalues};
 version = {"4.2"};
 author = {"Georges Koepfler"};
 function = {"Get and sort all the pixel values of a fimage"};
 usage = {
   'i'->i_flag
        "decreasing values (default is increasing)",
   'm':multiplicity<-multiplicity
        "output the multiplicity of the values (fsignal)",
   'r':rank<-image_rank
        "output rank of each pixel value in ordered list (fimage)",
   image_in->image_in
        "input fimage",
   values<-fvalues
        "output list of sorted values (fsignal)"
};
*/
/*----------------------------------------------------------------------
 v4.1: new -r option (G.Koepfler)
 v4.2: bug corrected in case of -m and input image of size (1,1) (JF)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define NDEBUG
/* comment this line out/in to en/disable assert() */
#include <assert.h>
#include "mw.h"
#include "mw-modules.h"

#ifndef u_long
#define u_long unsigned long int
#endif
#define h_value(A)      (*(heap+(u_long)(A)))
#define h_up(A)         ((u_long)((u_long)(A)-1)>>1)
#define h_left(A)       ((u_long)((u_long)(A)<<1)+1)
#define h_right(A)      ((u_long)((u_long)(A)+1)<<1)

Fsignal fvalues(char *i_flag, Fsignal multiplicity, Fimage image_rank,
                Fimage image_in)
{
    Fsignal values;
    float *val;
    float *rank = NULL;         /* should be an (u_long) array */
    u_long *heap, nb_values, size_max, i, l, h0, h1, h2, tmp;

    size_max = image_in->ncol * image_in->nrow;
    if (size_max <= 0)
        mwerror(FATAL, 1, "image_in too small");

    if (!(heap = (u_long *) malloc(size_max * sizeof(u_long))))
        mwerror(FATAL, 1, "\n Not enough memory for initialisation!!\n");

    if (image_rank)
    {
        if (!
            (image_rank =
             mw_change_fimage(image_rank, image_in->nrow, image_in->ncol)))
            mwerror(FATAL, 1, "\n Not enough memory for rank!!\n");
        rank = image_rank->gray;
    }

    i = 0;
    val = image_in->gray;
    do
    {
        h1 = i;
        while ((h1 > (u_long) 0) && (val[i] < val[h_value(h2 = h_up(h1))]))
        {
            h_value(h1) = h_value(h2);
            h1 = h2;
        }
        h_value(h1) = i++;
    }
    while (i < size_max);
    assert(i == size_max);
    assert(i > (u_long) 0);
    if (i == 1)
        nb_values = 1;
    else
    {
        nb_values = 0;
        do
        {
            tmp = h_value(--i);
            h_value(i) = h_value(0);
            if ((nb_values == 0) || (val[h_value(i)] > val[h_value(i + 1)]))
                nb_values++;
            h0 = 0;
            while ((h1 = h_left(h0)) < i)
            {
                if (((h2 = h_right(h0)) < i)
                    && (val[h_value(h2)] < val[h_value(h1)]))
                    h1 = h2;
                if (val[h_value(h1)] > val[tmp])
                    break;
                h_value(h0) = h_value(h1);
                h0 = h1;
            }
            h_value(h0) = tmp;
        }
        while (i > 1);
        assert(i == 1);
        if (val[h_value(0)] > val[h_value(1)])
            nb_values++;
    }
    assert(nb_values > (u_long) 0);

    if (!(values = mw_change_fsignal(NULL, nb_values)))
        mwerror(FATAL, 1, "\n Not enough memory for values!!\n");
    i = 0;
    if (i_flag)
    {
        values->values[0] = val[h_value(i)];
        for (l = 1; l < nb_values; l++)
        {
            do
            {
                if (rank)
                    rank[h_value(i)] = l - 1;
                i++;
            }
            while (!(val[h_value(i - 1)] > val[h_value(i)]));
            values->values[l] = val[h_value(i)];
            if (rank)
                rank[h_value(i)] = l;
        }
        if (rank)
            do
            {
                rank[h_value(i)] = nb_values - 1;
                i++;
            }
            while (i < size_max);
        assert(i <= size_max);
    }
    else
    {
        values->values[nb_values - 1] = val[h_value(i)];
        for (l = nb_values - 1; l > (u_long) 0; l--)
        {
            do
            {
                if (rank)
                    rank[h_value(i)] = l;
                i++;
            }
            while (!(val[h_value(i - 1)] > val[h_value(i)]));
            values->values[l - 1] = val[h_value(i)];
            if (rank)
                rank[h_value(i)] = l - 1;
        }
        if (rank)
            do
            {
                rank[h_value(i)] = 0;
                i++;
            }
            while (i < size_max);
        assert(i <= size_max);
    }
    if (multiplicity)
    {
        if (!(multiplicity = mw_change_fsignal(multiplicity, nb_values)))
            mwerror(FATAL, 1, "\n Not enough memory for multiplicity!!\n");
        i = size_max - 1;
        h0 = 1;
        while ((i != 0) && (!(val[h_value(i)] < val[h_value(i - 1)])))
        {
            i--;
            h0++;
        }
        if (i != 0)
            i--;
        l = (i_flag) ? nb_values - 1 : 0;
        multiplicity->values[l] = h0;
        while (i != 0)
        {
            h0 = 1;
            while ((i != 0) && (!(val[h_value(i)] < val[h_value(i - 1)])))
            {
                i--;
                h0++;
            }
            if (i != 0)
                i--;
            if (i_flag)
                multiplicity->values[--l] = h0;
            else
                multiplicity->values[++l] = h0;
        }
        assert(i == 0);
        if ((size_max == 1) || (val[h_value(i)] > val[h_value(i + 1)]))
        {
            if (i_flag)
                multiplicity->values[--l] = 1;
            else
                multiplicity->values[++l] = 1;
        }
        assert((i_flag && l == 0) || (!i_flag && l == nb_values - 1));
#ifndef NDEBUG
        for (l = 0, tmp = 0; l < nb_values; l++)
            size_max -= multiplicity->values[l];
        assert(size_max == 0);
#endif
    }
    free((void *) heap);
    return (values);
}
