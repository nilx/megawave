/*-------------------------- MegaWave2  Module ------------------------------*/
/* mwcommand
name     = {csample};
author   = {"Jacques Froment, Regis Monneau"};
function = {"sampling of a cimage"};
version  = {"1.0"};
usage    = {
             in->in                   "input image",
             out<-out                 "sampled image",
             reduction->reduction     "factor of sampling"    
           };
*/


#include <stdio.h>
#include "mw.h"

#define _(a,i,j)  ((a)->gray[(i)*(a)->ncol+(j)] )


csample(in ,out,reduction)
Cimage out,in;
int reduction;

{
  register int i,j;
  int nr;
  int nc;
  int nr1;
  int nc1;

  if (reduction < 2) mwerror(USAGE,1,"reduction must be greater than 1 !\n");

  nr = in->nrow;
  nc = in->ncol;
  nr1 = (nr - (nr % reduction))/reduction;
  nc1 = (nc - (nc % reduction))/reduction; 

  mwdebug("Input size: nr = %d \t nc = %d\n", nr,nc);
  mwdebug("Output size: nr1 = %d \t nc1 = %d\n", nr1,nc1);

  out = mw_change_cimage(out, nr1, nc1);
  if (out == NULL) mwerror(FATAL,1,"not enough memory.\n");

  for (i=0 ; i<  nr1; i++)
     for (j=0 ; j<  nc1; j++) 
         _(out,i,j) = _(in,i*reduction ,j*reduction );

}

