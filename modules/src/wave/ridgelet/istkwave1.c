/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {istkwave1};
 version = {"1.0"};
 author = {"Claire Jonchery, Amandine Robin"};
 function ={"Reconstruction of a signal from its wavelet coefficients using Starck's algorithm"};
 usage = {
   np->np     "resolution np",
   in->in     "decomposition of a signal in wavelets (2N coefficients)",
   out<-out   "signal (N elements)"
};
*/
/*----------------------------------------------------------------------
 v1.0: initial revision before publication (J.Froment)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

void istkwave1(int np, Fsignal in, Fsignal out)
{
  Fsignal temp=NULL;
  int i,j,k,N,m;

  N=(in->size)/2;
  
  out=mw_change_fsignal(out,N);
  if (!out) mwerror(FATAL,1,"not enough memory !\n");

  for (i=0;i<N;i++) out->values[i]=in->values[i];

  for (j=1;j<np;j++)
    {
      temp=mw_change_fsignal(temp,(N/(pow(2.0,(double)j))));
      if (!temp) mwerror(FATAL,1,"not enough memory !\n");
      /*on place dans temp les détails à la résolution 2^j, dans le domaine de Fourier*/

      for (k=0;k<(N/(pow(2.0,(double)j)));k++)
	{
	  m=(2*N)-N/(pow(2.0,(double)(j-1)));
          temp->values[k]=in->values[m+k];
	}
      /*les détails sont additionnés à la somme des détails des résolutions plus fines. Comme ils sont au nombre de N/(2^j), on les sépare en 2 parties pour les ajouter aux  basses fréquences */
      for (i=0;i<(N/(pow(2.0,(double)(j+1))));i++)
	out->values[i]+=temp->values[i];

      for (i=N-N/(pow(2.0,(double)(j+1)));i<N;i++)
	{
	  k=N/(pow(2.0,(double)j))-N+i;
	  out->values[i]+=temp->values[k];
	}
    }

  /*il ne reste plus qu'à ajouter l'approximation restante */

  for (i=0;i<(N/(pow(2.0,(double)np)));i++)
    {
      m=(2*N)-N/(pow(2.0,(double)(np-1)));
      out->values[i]+=in->values[m+i];
    }

  for (i=N-N/(pow(2.0,(double)np));i<N;i++)
    out->values[i]+=in->values[N+i];
       
  if (j>1) mw_delete_fsignal(temp);
}



