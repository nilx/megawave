/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {sop};
   version = {"1.1"};
   author = {"Pascal Monasse, Lionel Moisan"};
   function = {"Perform elementary operations between Fsignals"};
   usage = {
            'A':A->A       "take Fsignal A as left term",
            'a':a->a       "take constant a as left term",
            'p' -> plus    "the A + B (plus) operator",
	    'm' -> minus   "the A - B (minus) operator",
	    't' -> times   "the A x B (times) operator",
	    'd' -> divide  "the A / B (divide) operator",
	    'i' -> inf     "the inf(A,B) operator",
	    's' -> sup     "the sup(A,B) operator",
	    'l' -> less    "the A < B  operator (result: 1=true, 0=false)",
	    'g' -> greater "the A > B  operator (result: 1=true, 0=false)",
	    'e' -> equal   "the A == B operator (result: 1=true, 0=false)",
	    B->B           "input Fsignal B (right term)",
	    out<-out       "resulting Fsignal"
   };
   */
/*-- MegaWave 2- Copyright (C) 1994 Jacques Froment. All Rights Reserved. --*/

#include <stdio.h>
#include "mw.h"

void sop(B,out,A,a,plus,minus,times,divide,inf,sup,less,greater,equal)
Fsignal B,out,A;
float *a;
char *plus,*minus,*times,*divide,*inf,*sup,*less,*greater,*equal;
{
  int i;
  float left,res;

  /* check options */
  if ( (plus?1:0) + (minus?1:0) + (times?1:0) + (divide?1:0) 
       + (inf?1:0) + (sup?1:0) != 1 )
    mwerror(USAGE, 1, "please select exactly one of the operator options");
  if ( (A?1:0) + (a?1:0) != 1 )
     mwerror(USAGE, 1, "please select exactly one left term (-a or -A)");
   
  /* prepare output */
  out = mw_change_fsignal(out,B->size);
  if (!out) mwerror(FATAL, 1, "Not enough memory.");

  /* main loop */
  i = B->size;
  for (i;i--;) {
    left = (A?A->values[i]:*a);
    if      (plus)    res = left + B->values[i];
    else if (minus)   res = left - B->values[i];
    else if (times)   res = left * B->values[i];
    else if (divide)  res = left / B->values[i];
    else if (inf)     res = (left < B->values[i] ? left : B->values[i]);
    else if (sup)     res = (left > B->values[i] ? left : B->values[i]);
    else if (less)    res = (left < B->values[i] ? 1.0 : 0.0);
    else if (greater) res = (left > B->values[i] ? 1.0 : 0.0);
    else if (equal)   res = (left ==B->values[i] ? 1.0 : 0.0);
    out->values[i] = res;
  }
}

