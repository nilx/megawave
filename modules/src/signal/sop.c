/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {sop};
   version = {"1.3"};
   author = {"Pascal Monasse, Lionel Moisan, Jacques Froment"};
   function = {"Perform elementary operations between Fsignals"};
   usage = {
            'A':A->A       "take Fsignal A as left term",
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
	    B->B           "input Fsignal B (right term)",
	    out<-out       "resulting Fsignal"
   };
*/

/*----------------------------------------------------------------------
 v1.2: -D and -N added, like in fop (JF) 
 v1.3: preserve header info for e.g. sound processing (JF)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"

#define ABS(x) ((x)>0?(x):(-(x)))

void sop(Fsignal B, Fsignal out, Fsignal A, float *a, char *plus, char *minus, char *times, char *divide, char *dist, char *norm, char *inf, char *sup, char *less, char *greater, char *equal)
{
  int i;
  float left,res=0.0;

  /* check options */
  if ( (plus?1:0) + (minus?1:0) + (times?1:0) + (divide?1:0) 
       + (dist?1:0) + (norm?1:0) + (inf?1:0) + (sup?1:0) != 1 )
    mwerror(USAGE, 1, "please select exactly one of the operator options");
  if ( (A?1:0) + (a?1:0) != 1 )
     mwerror(USAGE, 1, "please select exactly one left term (-a or -A)");
   
  /* prepare output */
  out = mw_change_fsignal(out,B->size);
  if (!out) mwerror(FATAL, 1, "Not enough memory.");
  /* This assumes A has the same header than B */
  mw_copy_fsignal_header(B,out);
  
  /* main loop */
  for (i = B->size; i--;) {
    left = (A?A->values[i]:*a);
    if      (plus)    res = left + B->values[i];
    else if (minus)   res = left - B->values[i];
    else if (times)   res = left * B->values[i];
    else if (divide)  res = left / B->values[i];
    else if (dist)    res = ABS(left - B->values[i]);
    else if (norm)    res = sqrt((double) left * (double) left
				 + (double) (B->values[i]) 
				 * (double)(B->values[i]));
    else if (inf)     res = (left < B->values[i] ? left : B->values[i]);
    else if (sup)     res = (left > B->values[i] ? left : B->values[i]);
    else if (less)    res = (left < B->values[i] ? 1.0 : 0.0);
    else if (greater) res = (left > B->values[i] ? 1.0 : 0.0);
    else if (equal)   res = (left ==B->values[i] ? 1.0 : 0.0);
    out->values[i] = res;
  }
}

