/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {arencode2};
version = {"1.01"};
author = {"Jean-Pierre D'Ales"};
function = {"Arithmetic encoding of a string of symbol"};
usage = {
 'i'->Print         "Do not print info on encoding process",
 's':Size->Size     "Number of symbols to encode",
 'n':NSymbol->NSymb "Number of different symbols in source (size of alphabet)",
 'c':Cap_Histo->Cap_Histo  "Capacity of histogram",
 'p'->Predic        "Use predictive model",        
 'h':Histo->Histo   "Histogram model for input symbols (fsignal)", 
 'H'->Header        "Insert header with size of alphabet and predictive info",
 Input->Input       "Input string of symbols (fimage)",
 Rate<-Rate         "Rate of output",
  { 
    Output<-Output  "Output string of codewords (cimage)"
  }
};
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"


/*--- Constants ---*/

#define  MAX_SIZEO       32383
#define  code_value_bits 16
#define  top_value       (((long) 1 << code_value_bits) - 1)
#define  first_qrt       (top_value / 4 + 1)
#define  half            (2 * first_qrt)
#define  third_qrt       (3 * first_qrt)
#define  MAX_NSYMB       (top_value / 4 - 2)
#define  MAX_SIZEIM      ((long) (1 << 30) - 1 + (1 << 30))

static long             max_freq;        /* Effective capacity of histogram */
static long           **cum_freq;
static long           **freq;
static int              sizeo;           /* Size of Output */
static int              nsymbol;
static int              nsymbol_pred;
static long             bits_to_follow;
static int              buffer;
static int              bits_to_go;
static long             ncodewords;
static unsigned char   *ptro;            /* Pointer to output buffer */
static int             *list;            /* List of different symbol */
static int              EOF_symb;
static int              (*symb_ind) ();
static int              teststop = 0;



static int
symb_ind_direct(int symbol)
{
  return(symbol+1);
}


static int
symb_ind_list(int symbol)
{
  int    z;

  z = 0;
  while ((list[z] != symbol) && (z < nsymbol))
    z++;

  if (z == nsymbol) 
    mwerror(FATAL, 2, "Symbol %d not in the list!\n",symbol);

  return(z+1);
}




static void
REALLOCATE_OUTPUT(Cimage output)
{
  int              i;
  Cimage           bufcomp;

  printf("Reallocation of Output.\n");

  bufcomp = mw_new_cimage();
  bufcomp = mw_change_cimage(bufcomp, output->nrow,  output->ncol);
  if (bufcomp == NULL)
    mwerror(FATAL, 1, "Memory allocation refused for reallocation buffer!\n");
  for (i = 0; i < sizeo; i++)
    bufcomp->gray[i] = output->gray[i];

  output = mw_change_cimage(output, output->nrow * 2,  output->ncol);
  if (output == NULL)
    mwerror(FATAL, 1, "Memory allocation refused for reallocated output!\n");

  for (i = 0; i < sizeo; i++) {
    output->gray[i] = bufcomp->gray[i];
    output->gray[i+sizeo] = 0;
  }

  ptro = output->gray + sizeo - 1;
  sizeo = output->ncol * output->nrow;

  mw_delete_cimage(bufcomp);
}




static void
START_MODEL(Fsignal histo, long int *cap_histo, int *predic, Fimage input, long int size)
{
  float  *ptri;
  float  *ptrl;
  long    sizel;           /* Number of symbol in list */
  long    i, l;
  int     y, z;
  long    intpart;
  long    min, max;
  float   *buflist = NULL;
  int     sizebuf;

  nsymbol_pred = 1;

  if (size > 0) {

  /*--- Compute min and max of input symbols ---*/

    min = max = floor(input->gray[0] + .5);
    for (l = 1, ptri = input->gray + 1; l < size; l++, ptri++) {
      intpart = floor(*ptri + .5);
      if (max < intpart)
	max = intpart;
      if (min > intpart)
	min = intpart;
    }

    if (!histo) {

      /*--- adaptative encoding ---*/

      if ((min < 0) || (max >= nsymbol))
	if (nsymbol != 0) {
	  mwerror(WARNING, 0, "Bad value for NSymb!\n");
	  nsymbol = 0;
	}

      if (nsymbol == 0) {
    
	/*--- List and compute the number of different symbols in input ---*/

	sizebuf =  input->nrow*input->ncol + 1;
	buflist = (float *) malloc((int) sizebuf * sizeof(float));
	if (buflist == NULL)
	  mwerror(FATAL, 1, 
		  "Memory allocation refused for list of different symbol!\n");
	sizel = 1;
	buflist[0] = (float) floor(input->gray[0] + .5);
	for (i = 1, ptri = input->gray + 1; i < size; i++, ptri++) {
	  l = 0;
	  ptrl = buflist;
	  while ((l < sizel) && (floor(*ptrl + .5) != floor(*ptri + .5))) {
	    l++;
	    if (l >= sizebuf)
	      mwerror(INTERNAL,1,
		      "[START_MODEL] Index l of buflist to large\n");
	    ptrl++;
	  } 
	  if (l == sizel) {
	    sizel++;
	    *ptrl = (float) floor(*ptri + .5);
	  }
	}

	/*--- Assign a symbol for signaling end of input ---*/
      
	EOF_symb = max + 1;
	if (sizel >= sizebuf)
	  mwerror(INTERNAL,1,
		  "[START_MODEL] Index sizel=%d of buflist to large (>=%d)\n",
		  sizel,sizebuf);
	buflist[sizel] = EOF_symb;
	nsymbol = sizel + 1;
      
      } /* nsymbol == 0 */
      else
	nsymbol++;
    
      if (nsymbol > MAX_NSYMB)
	mwerror(FATAL, 2, "Number of symbols too large (%d > %d) for arithmetic coding!\n", nsymbol, (int) MAX_NSYMB);

      /*--- Prepare predictive encoding if selected ---*/
    
      if (predic) 
	nsymbol_pred = nsymbol - 1;

    } /* if (!histo) */
    else
      nsymbol = histo->size + 1;

    if (!buflist) 
      {
	if ((min < 0) || (max + 1 >= nsymbol))
	  mwerror(FATAL, 2, "Bad value of symbol in input!\n");
	EOF_symb = nsymbol - 1;
	symb_ind = symb_ind_direct;
	list = NULL;
      } else
	{
	  list = (int *) malloc((int) nsymbol * sizeof(int));
	  if (!list) 
	    mwerror(FATAL, 1, "Not enough memory\n");
	  symb_ind = symb_ind_list;
	}
  } /* if (size > 0) */
  else
    {
      nsymbol = 1;
      symb_ind = symb_ind_direct;
      EOF_symb = nsymbol - 1;
    }

  freq = (long **) malloc((int) (nsymbol_pred) * sizeof(long *));
  if (freq == NULL)
    mwerror(FATAL, 1, "Memory allocation refused for frequency buffer!\n");
  for (z = 0; z < nsymbol_pred; z++) {
    freq[z] = (long *) malloc((int) (nsymbol + 1) * sizeof(long));
    if (freq[z] == NULL)
      mwerror(FATAL, 1, "Memory allocation refused for frequency buffer!\n");
  }

  cum_freq = (long **) malloc((int) (nsymbol_pred) * sizeof(long *));
  if (cum_freq == NULL)
    mwerror(FATAL, 1, "Memory allocation refused for frequency buffer!\n");
  for (z = 0; z < nsymbol_pred; z++) {
    cum_freq[z] = (long *) malloc((int) (nsymbol + 1) * sizeof(long));
    if (cum_freq[z] == NULL)
      mwerror(FATAL, 1, "Memory allocation refused for frequency buffer!\n");
  }

  if (!histo) {
    for (y = 0; y < nsymbol_pred; y++) {
      for (z = 0; z <= nsymbol; z++) {
	freq[y][z] = 1;
	cum_freq[y][z] = nsymbol - z;
      }
      freq[y][0] = 0;
    }

    if (list) {
      for (z = 0; z < nsymbol; z++) 
	list[z] = buflist[z];
      
      free(buflist);
    }

    if (cap_histo) {
      max_freq = *cap_histo;
      if (max_freq <= cum_freq[0][0])
	max_freq = 100 * nsymbol;
    } else
      max_freq = 100 * nsymbol;

    if (max_freq > 1 + (top_value + 1) / 4)
      max_freq = 1 + (top_value + 1) / 4;

  } /* if (!histo) */
  else
    {
      cum_freq[0][nsymbol] = 0;
      for (z = nsymbol; z > 0; z--) {
	freq[0][z] = floor(histo->values[z-1] + .5);
	cum_freq[0][z-1] = cum_freq[0][z] + freq[0][z];
      }
      freq[0][0] = 0;

    } 

}



static void
UPDATE_MODEL(int symbol, int symbol_pred)
{
  int      z;
  int      cum;

  if (cum_freq[symbol_pred][0] == max_freq) {
    cum = 0;
    for (z = nsymbol; z >= 0; z--) {
      freq[symbol_pred][z] = (freq[symbol_pred][z] + 1) / 2;
      cum_freq[symbol_pred][z] = cum;
      cum += freq[symbol_pred][z];
    }
  }
  freq[symbol_pred][symbol]++;
  for (z = symbol - 1; z >= 0; z--)
    cum_freq[symbol_pred][z]++;

}



static void
ADD_BIT_TO_OUTPUT(int bit, Cimage output)

                
                               /* String of codewords */  

{
  buffer >>= 1;
  if (bit) 
    buffer += 128;
  bits_to_go -= 1;
  if (bits_to_go == 0) {
    *ptro = buffer;
    ncodewords += 1;
    if (ncodewords == sizeo)
      REALLOCATE_OUTPUT(output);
    ptro++;
    bits_to_go = 8;
    buffer = 0;
  }
}


static void
OUTPUT_BITS(int bit, Cimage output)

                
                               /* String of codewords */  

{

  if (ptro)
    ADD_BIT_TO_OUTPUT(bit, output);
  while (bits_to_follow > 0) {
    if (ptro)
      ADD_BIT_TO_OUTPUT(!bit, output);
    bits_to_follow -= 1;
  }

}



static void
ENCODE_INT(long int symb, long int max, Cimage output)
{

  while (max > 0) {
    if (symb >= max) {
      ADD_BIT_TO_OUTPUT(1, output);
      symb = symb % max;
    } else
      ADD_BIT_TO_OUTPUT(0, output);
    max /= 2;
  }

}



static void
MAKE_HEADER(int *predic, Cimage output, long int *nbit)

                              /* Flag for predictive model */
                              /* String of codewords */  
                              /* Number of output bits */

{

  if (output) {
    ENCODE_INT((long) nsymbol - 1, (long) 1 << (code_value_bits - 3), output);
    if (predic)
      ENCODE_INT(1L, 1L, output);
    else
      ENCODE_INT(0L, 1L, output);
  }
  *nbit += 17;
}



static void
ENCODE_SYMBOL(int symbol, int symbol_pred, long int *low, long int *high, long int *nbit, Cimage output)

                  
                       
                               /* Interval extremities for arithmetic coding */
                              /* Number of output bits */
                              /* String of codewords */  

{
  long     range;

  if ((symbol_pred < 0) || (symbol_pred >= nsymbol_pred) || (symbol <= 0) || (symbol > nsymbol)) {
    printf("symbol = %d, symbol_pred = %d\n", symbol, symbol_pred);
    teststop = 1;
  } else
    {
    
  range = (long) *high - *low + 1;
  *high = *low + (range * cum_freq[symbol_pred][symbol - 1]) / cum_freq[symbol_pred][0] - 1;
  *low = *low + (range * cum_freq[symbol_pred][symbol]) / cum_freq[symbol_pred][0];

  for (;;) {
    if (*high < half) {
      OUTPUT_BITS(0, output);
      *nbit += 1;
    } else
      if (*low >= half) {
	OUTPUT_BITS(1, output);
	*nbit += 1;
	*low -= (long) half;
	*high -= (long) half;
      } else
	if ((*low >= first_qrt) && (*high < third_qrt)) {
	  bits_to_follow += 1;
	  *nbit += 1;
	  *low -= (long) first_qrt;
	  *high -= (long) first_qrt;
	} else
	  break;

    if ((*low < 0) || (*high > top_value)) {
      printf("low = %ld, high = %ld, symbol = %d, symbol_pred = %d\n", 
	     *low, *high, symbol, symbol_pred);
      teststop = 1;
      break;
    }
      

    *low = 2 * *low;
    *high = 2 * *high + 1;
  }
}
}


void
arencode2(int *Print, long int *Size, int *NSymb, long int *Cap_Histo, int *Predic, Fsignal Histo, int *Header, Fimage Input, double *Rate, Cimage Output)

                                 /* Do not print info if selected */       
                                 /* Encode only the first Size symbols 
				  * Input */
                                 /* Size of alphabet for Input symbols */
    	                	 /* Capacity of histogram */
                                 /* Apply predictive encoding */
                                 /* Histogram source symbols */
                                 /* Insert header if selected */
                    	         /* String of symbol to encode */
                                 /* Output rate in bits per symbol */
                                 /* String of codewords */  

{
  register float  *ptri;         /* Pointer to input buffer */
  long	           i;            /* Index of input symbol currently encoded */
  long	           size;         /* Size of input */
  long             nbit;         /* Current number of bit in output */
  long             low, high;    /* Interval extremities for arithmetic 
				  * coding */
  int              ncolo, nrowo; /* Number of rows and columns in output */
  int              symb, symb_pred;   /* values of current and preceding 
				 * symbols */
  int              mindif;       /* Test value for resizing of output */


  /*--- Initialize model of source ---*/

  if (NSymb)
    nsymbol = *NSymb;
  else
    nsymbol = 0;

  size = Input->ncol * Input->nrow;
  if (Size)
    if ((*Size < size) && (*Size >= 0))
      size = *Size;

  if (size > MAX_SIZEIM)
    mwerror(FATAL, 2, "Input image is too large!\n");

  START_MODEL(Histo, Cap_Histo, Predic, Input, size);

  /*--- Memory allocation for Output ---*/

  if (Output) {
    nrowo = Input->nrow + 1;
    ncolo = Input->ncol + 1;
    ncolo *= ((double) log((double) nsymbol) / log((double) 2.0) / (double) 8.0);
    if (ncolo == 0)
      ncolo = 1;
    Output = mw_change_cimage(Output, nrowo, ncolo);
    if (Output == NULL)
      mwerror(FATAL, 1, "Memory allocation refused for `RecompImage`!\n");
    ptro = Output->gray;
    sizeo = Output->ncol * Output->nrow;
  } else
    ptro = NULL;

  /*--- Initialize encoding ---*/

  nbit = 0;
  low = 0;
  high = top_value;
  bits_to_follow = 0;
  buffer = 0;
  bits_to_go = 8;
  ncodewords = 0;

  /*--- Make header ---*/

  if (Header && Output) 
    MAKE_HEADER(Predic, Output, &nbit);

  if (nsymbol > 2) {

    symb_pred = 0;
    if (size > 0) {
      symb = symb_ind(floor(Input->gray[0] + .5));
      ENCODE_SYMBOL(symb, symb_pred, &low, &high, &nbit, Output);
      if ((!Histo) && !Predic) 
	UPDATE_MODEL(symb, symb_pred);
      if (size > 1)
	ptri = Input->gray + 1;
    } else
      symb = 1;

    /*--- Encode input ---*/

    for (i = 1; i < size; i++, ptri++) {
      if (Predic)
	symb_pred = symb - 1;
      symb = symb_ind(floor(*ptri + .5));
      ENCODE_SYMBOL(symb, symb_pred, &low, &high, &nbit, Output);

      if (teststop == 1)
	mwerror(FATAL, 1, "i = %d, symb = %d, symb_pred = %d, *ptri = %.2f,\nnbit = %d, first_qrt = %d, half = %d, top_value = %d\n", i, symb, symb_pred, *ptri, nbit, (long) first_qrt, (long) half, (long) top_value);

      if (!Histo) 
	UPDATE_MODEL(symb, symb_pred);
    }

    /*--- Finish encoding ---*/

    if (Predic)
      symb_pred = symb - 1;
    symb = symb_ind(EOF_symb);
    ENCODE_SYMBOL(symb, symb_pred, &low, &high, &nbit, Output);
    bits_to_follow += 1;
    nbit += bits_to_follow + 1;
    if (low < first_qrt)
      OUTPUT_BITS(0, Output);
    else
      OUTPUT_BITS(1, Output);

  } else
    {
      if (Output)
	ENCODE_INT(size, (long) 1 << 30, Output);
      nbit += 31;
    }

  if (ptro && (bits_to_go < 8)) {
    *ptro = buffer>>bits_to_go;
    ncodewords += 1;
  } else
    bits_to_go = 0;

  if (size == 0)
    size = 1;
  *Rate = (double) nbit / size;

  if (list)
    free(list);
  for (i = nsymbol_pred - 1; i >= 0; i--) {
    free(cum_freq[i]);
    free(freq[i]);
  }
  free(cum_freq);
  free(freq);

  size = ncodewords;
  if (Output) {
    if (!Print)
      printf("Number of output 8-bits codewords %ld (%ld bits)\n", 
	     ncodewords, nbit);

    /*--- Compute dimensions of Output ---*/

    ncolo = 1;
    nrowo = ncodewords;
    if (nrowo > MAX_SIZEO) {
      if ((int) sqrt((double) nrowo) + 1 > MAX_SIZEO)
	mwerror(FATAL, 2, "Number of codewords is too large!\n");
      i = 2;
      while (nrowo / i > MAX_SIZEO)
	i++;
      while ((nrowo % i != 0) && (i <= nrowo / i))
	i++;
      if (i <= nrowo / i) {
	nrowo /= i;
	ncolo = i;
      } else
	{
	  i = 2;
	  mindif = ncodewords;
	  while (i <= nrowo / i) {
	    if (nrowo / i <= MAX_SIZEO) 
	      if ((nrowo / i + 1) * i - ncodewords < mindif) {
		ncolo = i;
		mindif = (nrowo / i + 1) * i - ncodewords;
	      }
	    i++;
	  }
	  nrowo = ncodewords / ncolo + 1;
	  size = nrowo * ncolo;
	  if (ncodewords >= size)
	    mwerror(WARNING, 0, "Something is wrong with output dimensions!\n");
	}
    }

    if (nrowo > ncolo) {
      mindif = ncolo;
      ncolo = nrowo;
      nrowo = mindif;
    }

    /*--- Resize Output ---*/

    Output = mw_change_cimage(Output, nrowo, ncolo);
    if (size > ncodewords)
      for (i = ncodewords, ptro = Output->gray + ncodewords; i < size; i++, ptro++) 
	*ptro = 0;

    Output->firstrow = 8 - bits_to_go;
    Output->cmt[0]='\0';
  }
  
}
