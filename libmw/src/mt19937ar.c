/**
 * @file mt19927ar.c
 *
 * @author Makoto Matsumoto (1997 - 2002)
 * @author Takuji Nishimura (1997 - 2002)
 * @author Nicolas Limare (2009)
 *
 * Mersenne Twister pseudo-RNG code. Original code by Takuji Nishimura
 * and Makoto Matsumoto, adapted to the megawave environment
 *
 */

/*
 * Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. The names of its contributors may not be used to endorse or promote
 *      products derived from this software without specific prior written
 *      permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>

#include "mt19937ar.h"

/* Period parameters */
#define MT_N 624
#define MT_M 397
#define MT_MATRIX_A 0x9908b0dfUL        /* constant vector a */
#define MT_UPPER_MASK 0x80000000UL      /* most significant w-r bits */
#define MT_LOWER_MASK 0x7fffffffUL      /* least significant r bits */

static unsigned long mt[MT_N];  /* the array for the state vector  */
static int mti = MT_N + 1;      /* mti==MT_N+1 means mt[MT_N]
                                 * is not initialized */

/* initializes mt[MT_N] with a seed */
static void init_genrand(unsigned long s)
{
    mt[0] = s & 0xffffffffUL;
    for (mti = 1; mti < MT_N; mti++)
    {
        mt[mti] = (1812433253UL * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
static void init_by_array(unsigned long init_key[], int key_length)
{
    int i, j, k;
    init_genrand(19650218UL);
    i = 1;
    j = 0;
    k = (MT_N > key_length ? MT_N : key_length);
    for (; k; k--)
    {
        /* non linear */
        mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 30))
                          * 1664525UL)) + init_key[j] + j;
        /* for WORDSIZE > 32 machines */
        mt[i] &= 0xffffffffUL;
        i++;
        j++;
        if (i >= MT_N)
        {
            mt[0] = mt[MT_N - 1];
            i = 1;
        }
        if (j >= key_length)
            j = 0;
    }
    for (k = MT_N - 1; k; k--)
    {
        /* non linear */
        mt[i] =
            (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 30)) * 1566083941UL)) - i;
        /* for WORDSIZE > 32 machines */
        mt[i] &= 0xffffffffUL;
        i++;
        if (i >= MT_N)
        {
            mt[0] = mt[MT_N - 1];
            i = 1;
        }
    }

    mt[0] = 0x80000000UL;       /* MSB is 1; assuring non-zero initial array */
}

/* generates a random number on [0,0xffffffff]-interval */
static unsigned long genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2] = { 0x0UL, MT_MATRIX_A };
    /* mag01[x] = x * MT_MATRIX_A  for x=0,1 */

    if (mti >= MT_N)
    {                           /* generate MT_N words at one time */
        int kk;

        if (mti == MT_N + 1)    /* if init_genrand() has not been called, */
            init_genrand(5489UL);       /* a default initial seed is used */

        for (kk = 0; kk < MT_N - MT_M; kk++)
        {
            y = (mt[kk] & MT_UPPER_MASK) | (mt[kk + 1] & MT_LOWER_MASK);
            mt[kk] = mt[kk + MT_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (; kk < MT_N - 1; kk++)
        {
            y = (mt[kk] & MT_UPPER_MASK) | (mt[kk + 1] & MT_LOWER_MASK);
            mt[kk] = mt[kk + (MT_M - MT_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[MT_N - 1] & MT_UPPER_MASK) | (mt[0] & MT_LOWER_MASK);
        mt[MT_N - 1] = mt[MT_M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }

    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

#ifdef _MT_UNUSED
#error "This part of the could should no be seen by the compiler"
/* generates a random number on [0,0x7fffffff]-interval */
static long genrand_int31(void)
{
    return (long) (genrand_int32() >> 1);
}

/* generates a random number on [0,1]-real-interval */
static double genrand_real1(void)
{
    return genrand_int32() * (1.0 / 4294967295.0);
    /* divided by 2^32-1 */
}

/* generates a random number on [0,1)-real-interval */
static double genrand_real2(void)
{
    return genrand_int32() * (1.0 / 4294967296.0);
    /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
static double genrand_real3(void)
{
    return (((double) genrand_int32()) + 0.5) * (1.0 / 4294967296.0);
    /* divided by 2^32 */
}
#endif                          /* _MT_UNUSED */

/* generates a random number on [0,1) with 53-bit resolution*/
static double genrand_res53(void)
{
    unsigned long a = genrand_int32() >> 5, b = genrand_int32() >> 6;
    return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0);
}

/*
 * public wrappers
 */

/**
 * @brief initialize the random seed from one unsigned long intinteger
 *
 * @param s random seed
 */
void mw_srand_mt(unsigned long s)
{
    init_genrand(s);
    return;
}

/**
 * @brief initialize the random seed from an array of unsigned long
 * integers
 *
 * @param init_key random seeds array
 * @param key_length random seeds array length
 */
void mw_srand_mt_array(unsigned long init_key[], int key_length)
{
    init_by_array(init_key, key_length);
    return;
}

/**
 * @brief generate an integer random number on
 * [0,0xffffffff]-interval, with 32-bit randomness.
 *
 * This random number is an unsigned long. You can use it to get:
 * - a 31-bit (signed) long random number
 *   (long)(mw_rand32_mt()>>1
 * - a double-precision 32-bit floating-point random bumber on [0,1],
 *   mw_rand32_mt()*(1.0/4294967295.0)
 * - a double-precision 32-bit floating-point random bumber on [0,1),
 *   mw_rand32_mt()*(1.0/4294967296.0)
 * - a double-precision 32-bit floating-point random bumber on (0,1),
 *   (((double)mw_rand32_mt()) + 0.5)*(1.0/4294967296.0)
 *
 * @return a pseudo-random integer number with 32-bit randomness
 */
unsigned long mw_rand32_mt(void)
{
    return genrand_int32();
}

/**
 * @brief generate a floating-point random number on [0,1), with
 * 53-bit randomness.
 *
 * As IEE754 standard specifies that double precision floating-point
 * numbers have a 53 bits mantissa, this is the recommmended method.
 *
 * @return a pseudo-random floaint-point number with 53-bit randomness
 */
double mw_drand53_mt(void)
{
    return genrand_res53();
}

#undef MT_N
#undef MT_M
#undef MT_MATRIX_A
#undef MT_UPPER_MASK
#undef MT_LOWER_MASK
