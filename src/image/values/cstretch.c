/*--------------------------Commande MegaWave2------------------------------*/
/* mwcommand
name     = {cstretch};
author   = {"Regis Monneau"};
function = {"stretching of the histogram"};
version  = {"1.0"};
usage    = {
             in->in     "input image"     ,
             out<-out   "stretched image"    ,
             min->min               "minimum for histogram",
             max->max               "maximum for histogram"
           };
*/

#include <stdio.h>
#include "mw.h"

#define _(a,i,j)  ((a)->gray[(i)*(a)->ncol+(j)] )
#define GRAY_LEVEL_MAX  255
#define GRAY_LEVEL_MIN  0



/*----------------------FONCTION PRELIMINAIRE----------------*/

normalisation (out,in,i,j,max,k,Nmin,Nmax)

Cimage in,out;
int    max,k,*Nmin,*Nmax;
int    i,j;

{ 
   int v;
   v =  GRAY_LEVEL_MAX - (max-_(in,i,j))*k;
   if (v > GRAY_LEVEL_MAX) 
      {
        v = GRAY_LEVEL_MAX ;
        (*Nmax)++;
      }
   if (v < GRAY_LEVEL_MIN) 
      { 
        v = GRAY_LEVEL_MIN ;
        (*Nmin)++;
      }
   _(out,i,j)= v;
}


  
/*------------------PROGRAMME PRINCIPAL-----------------------*/

cstretch(in,out,min,max)

Cimage in,out;
int    min,max;

{
  /*---------------DECLARATION DES VARIABLES---------*/
  int i,j,k,r,Nmin,Nmax;
  Nmin = 0;
  Nmax = 0;
  out = mw_change_cimage(out,in->nrow,in->ncol);
  if (out == NULL) mwerror(FATAL,1,"Not enough memory.\n");
  mw_clear_cimage(out,GRAY_LEVEL_MIN);

  
  /*------CALCUL DES CONSTANTES STRUCTURELLES--------*/
  r = GRAY_LEVEL_MAX % (max - min);
  k = (GRAY_LEVEL_MAX - r)/(max - min);
  mwdebug("pas de repartition = %d\n",k);


  /*--------------NORMALISATION----------------------*/

  for (i=1; i<in->nrow -1; i++)
    for (j=1; j<in->ncol -1; j++)
      normalisation(out, in, i ,j,max,k,&Nmin,&Nmax);

  mwdebug("(nb pts < min) = %d\n",Nmin);
  mwdebug("(nb pts > max) = %d\n",Nmax);
}



