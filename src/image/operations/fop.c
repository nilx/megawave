/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fop};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"Perform an elementary operation between 2 Fimages"};
   usage = {
            'A':A->A       "take Fimage A as left term",
            'a':a->a       "take constant a as left term",
            'p' -> plus    "the A + B (plus) operator",
	    'm' -> minus   "the A - B (minus) operator",
	    't' -> times   "the A x B (times) operator",
	    'd' -> divide  "the A / B (divide) operator",
	    'D' -> dist    "the |A-B| (distance) operator",
	    'N' -> norm    "the hypot(A,B)=(A^2+B^2)^(1/2) (norm) operator",
	    'i' -> inf     "the inf(A,B) operator",
	    's' -> sup     "the sup(A,B) operator",
	    'l' -> less    "the A < B  operator (result: 1=true, 0=false)",
	    'g' -> greater "the A > B  operator (result: 1=true, 0=false)",
	    'e' -> equal   "the A == B operator (result: 1=true, 0=false)",
	    B->B           "input Fimage B (right term)",
	    out<-out       "result Fimage"
   };
   */
/*-- MegaWave 2- Copyright (C) 1994 Jacques Froment. All Rights Reserved. --*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

#define ABS(x) ((x)>0?(x):(-(x)))

void fop(B,out,A,a,plus,minus,times,divide,dist,norm,inf,sup,less,greater,equal)
Fimage B,out,A;
float *a;
char *plus,*minus,*times,*divide,*dist,*norm,*inf,*sup,*less,*greater,*equal;
{
  int i;
  float left,res;

  /* check options */
  if ( (plus?1:0) + (minus?1:0) + (times?1:0) + (divide?1:0) 
       + (dist?1:0) + (norm?1:0) + (inf?1:0) + (sup?1:0) != 1 )
    mwerror(USAGE, 1, "please select exactly one of the operator options");
  if ( (A?1:0) + (a?1:0) != 1 )
     mwerror(USAGE, 1, "please select exactly one left term (-a or -A)");
   
  /* prepare output */
  out = mw_change_fimage(out,B->nrow,B->ncol);
  if (!out) mwerror(FATAL, 1, "Not enough memory.");

  /* main loop */
  i = B->nrow*B->ncol;
  for (i;i--;) {
    left = (A?A->gray[i]:*a);
    if      (plus)    res = left + B->gray[i];
    else if (minus)   res = left - B->gray[i];
    else if (times)   res = left * B->gray[i];
    else if (divide)  res = left / B->gray[i];
    else if (dist)    res = ABS(left - B->gray[i]);
    else if (norm)    res = (float)hypot((double)left,(double)(B->gray[i]));
    else if (inf)     res = (left < B->gray[i] ? left : B->gray[i]);
    else if (sup)     res = (left > B->gray[i] ? left : B->gray[i]);
    else if (less)    res = (left < B->gray[i] ? 1.0 : 0.0);
    else if (greater) res = (left > B->gray[i] ? 1.0 : 0.0);
    else if (equal)   res = (left ==B->gray[i] ? 1.0 : 0.0);
    out->gray[i] = res;
  }
}

