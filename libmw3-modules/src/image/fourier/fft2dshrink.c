/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {fft2dshrink};
 version = {"1.1"};
 author = {"Lionel Moisan"};
 function = {"Shrink a Fimage to speed up FFT"};
 usage = {
 'v'->v        "verbose mode",
 'p':[p=1.]->p "maximum percentage of shrink allowed on each axis",
 in->in        "input Fimage",
 out<-out      "output shrinked Fimage"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw3.h"
#include "mw3-modules.h"

#define SIZE_PRIME 460

static int primes[SIZE_PRIME] =
    { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67,
    71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
    151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257,
    263, 269, 271, 277, 281,
    283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379,
    383, 389, 397, 401, 409,
    419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499,
    503, 509, 521, 523, 541,
    547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631,
    641, 643, 647, 653, 659,
    661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761,
    769, 773, 787, 797, 809,
    811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907,
    911, 919, 929, 937, 941,
    947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031,
    1033,
    1039, 1049, 1051, 1061,
    1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151,
    1153, 1163, 1171, 1181,
    1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277,
    1279, 1283, 1289, 1291,
    1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399,
    1409, 1423, 1427, 1429,
    1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493,
    1499, 1511, 1523, 1531,
    1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609,
    1613, 1619, 1621, 1627,
    1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733,
    1741, 1747, 1753, 1759,
    1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871,
    1873, 1877, 1879, 1889,
    1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997,
    1999, 2003, 2011, 2017,
    2027, 2029, 2039, 2053, 2063, 2069, 2081, 2083, 2087, 2089, 2099, 2111,
    2113, 2129, 2131, 2137,
    2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243,
    2251, 2267, 2269, 2273,
    2281, 2287, 2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357,
    2371, 2377, 2381, 2383,
    2389, 2393, 2399, 2411, 2417, 2423, 2437, 2441, 2447, 2459, 2467, 2473,
    2477, 2503, 2521, 2531,
    2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617, 2621, 2633,
    2647, 2657, 2659, 2663,
    2671, 2677, 2683, 2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729,
    2731, 2741, 2749, 2753,
    2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851,
    2857, 2861, 2879, 2887,
    2897, 2903, 2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999,
    3001, 3011, 3019, 3023,
    3037, 3041, 3049, 3061, 3067, 3079, 3083, 3089, 3109, 3119, 3121, 3137,
    3163, 3167, 3169, 3181,
    3187, 3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257
};

/* Compute the complexity per element of the FFT of a signal with size n
   that is the sum of the "prime factors of n minus 1" */

static int comp(int n)
{
    int c, i;

    for (i = c = 0; n > 1 && i < SIZE_PRIME;)
        if (n % primes[i] == 0)
        {
            c += primes[i] - 1;
            n /= primes[i];
        }
        else
            i++;
    c += n - 1;

    return (c);
}

/*------------------------------ MAIN MODULE ------------------------------*/

Fimage fft2dshrink(Fimage in, Fimage out, float *p, char *v)
{
    int nx, ny, x, y, dx, dy, bestx = 0, besty = 0, c =
        0, bestc, initc, finalc;

    if (*p < 0. || *p > 100.)
        mwerror(FATAL, 1, "p should be between 0 and 100\n");

    nx = in->ncol;
    ny = in->nrow;

    /* compute optimal size (x) */
    bestc = 2 * nx;
    for (x = (int) ((1. - 0.01 * (*p)) * (float) nx); x <= nx; x++)
    {
        c = comp(x);
        if (c <= bestc)
        {
            bestx = x;
            bestc = c;
        }
    }
    initc = c;
    finalc = bestc;

    /* compute optimal size (y) */
    if (nx == ny)
        besty = bestx;
    else
    {
        bestc = 2 * ny;
        for (y = (int) ((1. - 0.01 * (*p)) * (float) ny); y <= ny; y++)
        {
            c = comp(y);
            if (c <= bestc)
            {
                besty = y;
                bestc = c;
            }
        }
    }
    initc += c;
    finalc += bestc;

    /* print result if requested */
    if (v)
    {
        printf("Initial image is %d x %d (FFT is %d op/pixel)\n", nx, ny,
               initc);
        printf(" ... shrinked to %d x %d (FFT is %d op/pixel)\n", bestx,
               besty, finalc);
        printf("  -> expected speed ratio = %f\n",
               (float) initc / (float) finalc);
    }

    /* copy center part of input image */
    out = mw_change_fimage(out, besty, bestx);
    if (!out)
        mwerror(FATAL, 1, "Not enough memory.");
    dx = (nx - bestx) / 2;
    dy = (ny - besty) / 2;
    for (x = 0; x < bestx; x++)
        for (y = 0; y < besty; y++)
            out->gray[y * bestx + x] = in->gray[(y + dy) * nx + x + dx];

    return (out);
}