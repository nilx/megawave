/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {mk_codebook};
author = {"Jean-Pierre D'Ales"};
function = {"Generate a random codebook"};
version = {"2.01"};
usage = {
'n'->Normal
  "Gaussian distribution instead of uniform",
'm':[Mean=0.0]->Mean
  "Mean of coeff.",
'v':[Var=1.0]->Variance
  "Variance of coeff.",
's':[Size=2]->Size
  "size of codebook", 
'b':[BlockSize=4]->BlockSize
  "Size of vector",
CodeBook<-CodeBook
  "Generated CodeBook (fimage)"
};
 */

/*--- Fichiers inclus UNIX C ---*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>

/*--- Bibliotheque megawave2 ---*/
#include  "mw.h"


static double ran0(initseed)

long *initseed;

{
  static double y, v[97];
  float dum;
  static int iff=0; 
  int j; 
  
  if (*initseed < 0 || iff == 0) {
    iff=1;
    srand48(*initseed);
    *initseed=1;
    for(j=0;j<97;j++)
      dum=drand48();
    for(j=0;j<97;j++)
      v[j]=drand48();
    y = drand48();
  }

  j = y * 98.0;
  if ((j<0)||(j>97))
    mwerror(FATAL, 1, "Valeur impossible pour j : %d, (y=%.5f)", j, y);
  y = v[j]; 
  v[j] = drand48(); 
  return(y);
}


static double normaldev(graine)

long *graine;

{
  static int iset = 0;
  static double nset;
  double u1, u2;
  double l2, logl2;

  if (iset == 0) {
    do {
      u1 = (ran0(graine) - 0.5) * 2.0;
      u2 = (ran0(graine) - 0.5) * 2.0;
      l2 = u1 * u1 + u2 * u2;
    } while((l2 >= 1.0) || (l2 == 0.0));
    logl2 = sqrt((double) - 2.0 * log(l2) / l2);
    nset = u1 * logl2;
    iset = 1;
    return(u2 * logl2);
  } else
    {
      iset = 0;
      return(nset);
    }
}


void
mk_codebook(Normal, Mean, Variance, Size, BlockSize, CodeBook)

	/*--- Create a gaussian or uniform white noise image ---*/

int        *Normal;     /* Normal distribution */
double     *Mean;       /* Mean of coefficients */
double     *Variance;   /* Variance of coefficients */
int	   *Size;	/* Size of codebook */
int        *BlockSize;  /* Size of vectors */
Fimage      CodeBook;	/* Generated codebook (output) */

{
  register float *ptrc;
  long i;
  long graine;
  double ra;
  double Level;
  long sizec;

  CodeBook = mw_change_fimage(CodeBook, *Size + 4, *BlockSize);
  if (CodeBook == NULL)
    mwerror(FATAL,1,"Not enough memory for generated codebook.\n");

  ptrc = CodeBook->gray;

  sizec = *Size * *BlockSize;

  graine = time(NULL);

  if (Normal) {
    Level = sqrt(*Variance);
    for(i=0; i<sizec; i++, ptrc++) { 
      ra = *Mean + Level * normaldev(&graine);
      *ptrc = ra;
    } 
  } else
    {
      Level = sqrt((double) 12.0 * *Variance);
      for(i=0; i<sizec; i++, ptrc++) { 
	ra = *Mean + (ran0(&graine) - 0.5) * Level;
	*ptrc = ra;
      }
    }

    for(i = 0; i < 3 * *BlockSize; i++, ptrc++) 
      *ptrc = 0.0;
    for(i = 0; i < *BlockSize; i++, ptrc++) 
      *ptrc = 1.0;
}
