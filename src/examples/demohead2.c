/*----------------------------- MegaWave Module -----------------------------*/
/* mwcommand
  name = {demohead2};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Demo of MegaWave2 header - #2 : optional arguments -"};
  usage = {

  output0 <- out0    
     "Output scalar (int) -needed argument-",

     {
     [input0=1]  -> in0 [-5,5]
       "Input scalar (int) with default value and boundary - optional argument -",
     input1 -> in1 
       "Input scalar (float) - optional argument -",
     output1 <- out1
       "Output MegaWave2 type (cimage) - optional argument -"
     }
           };
*/
/*--- MegaWave2 - Copyright (C)1994 Jacques Froment. All Rights Reserved. ---*/


#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

void demohead2(out0,in0,in1,out1)

int *out0;    /* You need a pointer since it's an output */
int *in0;     /* You need a pointer since it's an optional argument */
float *in1;   /* You need a pointer since it's an optional argument */
Cimage out1;  /* out1 is already a pointer since it's of MegaWave2 type */

{
  /* out0 cannot be NULL */
  *out0 = 3;

  /* in0 cannot be NULL */
  printf("*in0 = %d\n",*in0);

  /* in1 may be NULL */
  if (in1) printf("*in1 = %f\n",*in1); else printf("No in1 value\n");

  /* out1 may be NULL */
  if (out1) 
    {
      /* Here you can compute out1, after dimensionning */
      out1 = mw_change_cimage(out1,10,10); /* for a size of (10,10) */
      if (out1 == NULL) mwerror(FATAL,1,"Not enough memory\n");
      /*
	 ...
      */
    }
  else printf("No optional output image out1\n");

}





