/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fshrink2};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"Shrink a Fimage to dimensions power of two"};
   usage = {
           in->in       "input Fimage",
           out<-out     "shrinked Fimage"
   };
   */
/*-- MegaWave - Copyright (C) 1994 Jacques Froment. All Rights Reserved. --*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

/* NB : calling this module with out=in is nonsense */

void fshrink2(in,out)
Fimage in,out;
{
    int n,p,nn,pp,tmp,i,j,iofs,jofs;

    n = in->nrow;
    p = in->ncol;

    /* Compute new image size */
    nn = 1; tmp = n>>1;
    while (tmp) {tmp>>=1; nn<<=1;}
    pp = 1; tmp = p>>1;
    while (tmp) {tmp>>=1; pp<<=1;}

    /* copy center part of input image */
    out = mw_change_fimage(out,nn,pp);
    if (!out) mwerror(FATAL,1,"Not enough memory.");
    iofs = (n-nn)>>1;
    jofs = (p-pp)>>1;
    for (i=0;i<nn;i++) for (j=0;j<pp;j++) 
      out->gray[i*pp+j] = in->gray[(i+iofs)*p+j+jofs];
}



