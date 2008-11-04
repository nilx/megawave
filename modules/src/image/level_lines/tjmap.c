/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {tjmap};
 version = {"1.3"};
 author = {"Vicent Caselles, Bartomeu Coll, Jacques Froment, Jose-Luis Lisani"};
 labo = {"Universitat de les Illes Balears and CEREMADE"};
 function = {"Map the T and X junctions of a cimage"};
 usage = {
   'c'->connex8    "8-connexity (default : 4)",
   'v'->values     "plot in J the pixels values of the junctions",
   'a':[tarea=40]->tarea[0,5000] "area threshold",
   'q':[tquant=2]->tquant[0,255] "quantization threshold",
   U->U            "input cimage U",
   J<-J            "output cimage J : location of the junctions (map)",
   ntj<-tjmap      "number of junctions found"
};
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mw.h"
#include "mw-modules.h" /* for tjpoint() */

/* Record the junction location */

static void record_junction(U,J,values,x,y)
     Cimage U,J;
     char *values;
     int x,y;
{
  int l;

  if (!values) mw_plot_cimage(J,x,y,255);
  else
    {
      l=y*U->ncol+x;
      J->gray[l] = U->gray[l];
      J->gray[l+1] = U->gray[l+1];      
      J->gray[l+U->ncol] = U->gray[l+U->ncol];      
      J->gray[l+U->ncol+1] = U->gray[l+U->ncol+1];      
    }
}

int tjmap(connex8,values,tarea,tquant,U,J)
     char *connex8;
     char *values;
     int *tarea;
     int *tquant;
     Cimage U,J;
{
  int x,y;  /* current position (x,y) */
  int tj,ntj,l;
  unsigned char *M=NULL; /* Tab to mark the pixels in mscarea */
  int *P=NULL;           /* Tab to index area -> pixel in mscarea */

  /* Initialisations */

  J=mw_change_cimage(J, U->nrow, U->ncol);
  if (J==NULL) mwerror(FATAL,1,"Not enough memory.\n");
  mw_clear_cimage(J,0);

  M=(unsigned char *) malloc(U->nrow*U->ncol);
  if (M==NULL) mwerror(FATAL,1,"Not enough memory.\n");
  memset(M,0,U->nrow*U->ncol);
  
  P=(int *) malloc(*tarea * sizeof(int));
  if (P==NULL) mwerror(FATAL,1,"Not enough memory.\n");
  for (l=0; l<*tarea; l++) P[l]=-1;

  /* Scan each point of the image U */

  ntj=0;
  for (y=0; y < U->nrow - 1; y++)
    { 
      mwdebug("line %d/%d...\n",y,U->nrow-2);
      for (x=0; x < U->ncol - 1; x++)
	{
	  tj = tjpoint(connex8,tarea,tquant,U,x,y,
		       NULL,NULL,NULL,NULL,NULL,NULL,M,P);
	  if (tj > 0) { ntj++; record_junction(U,J,values,x,y); }
	}
    }
  free(P);
  free(M);
  return(ntj);
}


