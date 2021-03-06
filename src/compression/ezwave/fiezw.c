/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {fiezw};
version = {"1.10"};
author = {"Jean-Pierre D'Ales"};
function = {"Decompress an image compressed by EZW algorithm"};
usage = {
'e':EdgeIR->Edge_Ri
      	"Impulse reponses of edge and preconditionning filters for orthogonal transform (fimage)",
'b':ImpulseResponse2->Ri2
	"Impulse response of filter 2 for biorthogonal transform (fsignal)",
'w':WeightFac->WeightFac
        "Scaling factor for wavelet coefficients", 
Compress->Compress
          "Input string of codewords (cimage)",
ImpulseResponse->Ri
	"Impulse response of inner filters (fsignal)", 
QImage<-Output
	"Output reconstructed image (fimage)"
	};
 */


#include <stdio.h>
#include <math.h>
#include  "mw.h"

/*--- Megawave2 modules definition ---*/

extern void iezw();
extern void iowave2();
extern void ibiowave2();

/*--- Constants ---*/






void
fiezw(Edge_Ri, Ri2, WeightFac, Compress, Ri, Output)

	/*--- Computes the orthogonal wavelet transform of image `Image` ---*/

Fimage	    Edge_Ri;		/* Impulse responses of filters for special 
				 * edge processing (including preconditionning 
				 * matrices */
Fsignal     Ri2;		/* Impulse response of the low pass filter */
				/* for synthesis */
float      *WeightFac;          /* Weighting factor for wavelet coeff. */
Cimage      Compress;		/* Input compressed image */
Fsignal     Ri;			/* Impulse response of the low pass filter */
Fimage      Output;		/* Reconstructed image */

{
  int         J;       	/* Current level of decomposition */
  Wtrans2d    QWtrans;
  int         Haar;             /* Continue decomposition with Haar wavelet
				 * until haar level */
  int         Edge;	        /* Edge processing mode */
  int         FilterNorm;	/* Normalisation of filters */
  int         Precond;          /* Preconditionning mode for orthogonal 
				 * transform */
  int         Max_Count_AC;     /* Size of histogram for arithm. coding */

  
  /*--- Reconstruct wavelet transform ---*/

  QWtrans = mw_new_wtrans2d();

  iezw(NULL, WeightFac, Compress, QWtrans);

  /*--- Inverse wavelet transform ---*/
  
  J = Haar = QWtrans->nlevel;

  if (Ri2) {
    Edge = 2;
    FilterNorm = 1;
    ibiowave2(&J, &Haar, &Edge, &FilterNorm, QWtrans, Output, Ri, Ri2);
  } else
    {
      if (Edge_Ri)
	Edge = 3;
      else
	Edge = 2;
      FilterNorm = 2;
      Precond = 0;
      iowave2(&J, &Haar, &Edge, &Precond, NULL, &FilterNorm, QWtrans, Output, Ri, Edge_Ri);
    }

  mw_delete_wtrans2d(QWtrans);
}
