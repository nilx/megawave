/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {entropy};
version = {"1.0"};
author = {"Jean-Pierre D'Ales"};
function = {"Compute the entropy given by an histogram"};
usage = {
Histo->Histo
	"Input histogram (fsignal)",
Entropy<-Entropy
	""
	};
 */


/*--- Include files UNIX C ---*/
#include <math.h>

/*--- Megawave2 library ---*/
#include  "mw.h"

/*--- Constants ---*/




void
entropy(Histo, Entropy)

    Fsignal     Histo;		/* Input histogram */
    double     *Entropy;	/* Output, entropy of Histo */
{
  int i;
  double sum;				/* Sum of histogram values */
  double e;				/* Entropy of histogram */
  double s;				/* Normalised values of histogram */

  /*--- Sum of histogram values ---*/
	
  sum = 0.0;
  for(i=0;i<Histo->size;i++)
    sum+=Histo->values[i];

  /*--- Computation of entropy ---*/
	
  e = 0.0;
  for(i=0;i<Histo->size;i++)
    if(Histo->values[i] > 0.0) {
      s = Histo->values[i] / sum;
      e -= s * log((double) s);
    }
	
  e /= log((double) 2.0);

  *Entropy = e;

}

