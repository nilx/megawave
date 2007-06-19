/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {demohead1};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Demo of MegaWave2 header - #1 : options -"};
  usage = {
  'a'->flg                  
             "Flag -option-",

  'B':cimage_input_opt -> B 
             "Input MegaWave2 type (cimage) -option-", 

  'c':[c_opt=1.0] -> c      
             "Input scalar (float) with default value -option-",

  'd':d_opt -> d  [-10,10]  
             "Input scalar (integer) with boundary -option-",

  'e':[e_opt=0.0]->e[0.0,1e20] 
             "Input scalar (double) with default value and boundary -option-",

  'F':f_opt <- F            
             "Output MegaWave2 type (cimage) -option-",

  float_input -> input    [-1.0,1.0]
             "Input scalar (float) with boundary -needed argument-",

  cimage_output<-demohead1      
             "Output MegaWave2 type (cimage) -needed argument-"
           };
*/
/*--- MegaWave2 - Copyright (C)1994 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

Cimage demohead1(flg,B,c,d,e,F,input)

char *flg;   /* Or int *flg, ... */
Cimage B;    /* You don't need *B since Cimage is of MegaWave2 type (pointer)*/
float *c;    /* You need *c since float is a scalar (not a pointer) */
int *d;      /* You need *d since int is a scalar (not a pointer) */
double *e;   /* You need *e since double is a scalar (not a pointer) */
Cimage F;    /* You don't need *F since Cimage is of MegaWave2 type */
float input; /* You don't need any pointer since it's a needed input */


{
  Cimage output;  /* return of the module */
  
  if (flg) printf("flg flag active\n"); else printf("flg flag not active\n");

  /* B may be NULL */
  if (B) 
    {
      printf("Optional input B image selected\n"); 
      /* Here you can access to the content of the image B
	 ...
       */
    }
  else 
    {
      printf("No B image selected\n");
      /* Do not access to B */
    }


  /* c cannot be NULL since *c has a default value */
  printf("*c = %f\n",*c);


  /* d may be NULL */
  if (d) printf("*d = %d\n",*d); else printf("No d value\n");
    
  /* e cannot be NULL since *e has a default value */
  printf("*e = %lf\n",*e);

  /* F may be NULL */
  if (F)
    {
      printf("Optional output F image selected\n");
      /* Here you can compute the image F, after dimensioning.  
      */      
      F = mw_change_cimage(F,10,10); /* for a size of (10,10) */
      if (F == NULL) mwerror(FATAL,1,"Not enough memory\n");
      /*
	 ...
      */
    }
  else printf("No optional output F selected\n");

  /* Needed scalar argument is not a pointer */
  printf("input = %f\n",input);

  /* We need to create the structure and to allocate the output image */
  /* - In this example the size is (1,1) */  
  output = mw_change_cimage(NULL,1,1);
  if (output == NULL) mwerror(FATAL,1,"Not enough memory\n");

  /* Here you can compute the image output 
     ...
  */  

  return(output);
}





