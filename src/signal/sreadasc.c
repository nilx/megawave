/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {sreadasc};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Read a signal in ascii format"};
  usage = {    
      out<-out  "output Fsignal",
    { n->n      "size of signal" }

    };
*/

#include <stdio.h>
#include "mw.h"
 
Fsignal sreadasc(out,n)
Fsignal out;
int *n;
{
  int i,size;
  float v;

  if (!n) { /* read them from standart input */
    if (scanf("%d",&size) != 1) 
      mwerror(FATAL,1,"Premature end of file. Cannot read size.\n");
  } else size = *n;

  out = mw_change_fsignal(out,size);
  mw_clear_fsignal(out,0.);
  if (!out) mwerror(FATAL,1,"Not enough memory\n");

  for (i=0;i<size;i++) {
    if (scanf("%f",&v) != 1) 
      mwerror(WARNING,1,"Premature end of file. Will pad data with 0.\n");
      out->values[i] = v;
    }

  return(out);
}
