/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {ardecode2};
version = {"1.01"};
author = {"Jean-Pierre D'Ales"};
function = {"Arithmetic decoding of a compressed fimage"};
usage = {
'i'->Print
        "Do not print info on decoding process",
'r':NRow->NRow [1,32383]
          "Number of rows in output Fimage",
'n':NSymbol->NSymb
          "Number of different symbols in source",
'c':Cap_Histo->Cap_Histo
          "Capacity of histogram",
'p':Predic->Predic [0,1]
          "0 : no prediction, 1 : predictive encoding has been used, default : info is in header",        
'h':Fsignal->Histo
          "Histogram model for input symbols (fsignal)", 
Cimage->Input
          "Input string of symbols (cimage)",
Rate<-Rate
          "Rate of output",
Fimage<-Output
          "Output string of codewords (fimage)"
};
 */


/*--- Include files UNIX C ---*/
#include <stdio.h>
#include <math.h>

/*--- Library megawave2 ---*/
#include  "mw.h"


/*--- Constants ---*/

#define  MAX_SIZEO       32383
#define  code_value_bits 16
#define  top_value       (((long) 1 << code_value_bits) - 1)
#define  first_qrt       (top_value / 4 + 1)
#define  half            (2 * first_qrt)
#define  third_qrt       (3 * first_qrt)

static int              max_freq;       /* Effective capacity of histogram */
static long           **cum_freq;
static long           **freq;           /* Histograms */
static long             nsymbol;        /* Number of symbols in alphabet */
static int              nsymbol_pred;      
static int              value;          /* Current value of decoded symbol */
static int              buffer;
static int              garbage_bits;
static int              bits_to_go;
static long             nbitread;
static long             ncwread;        /* Number of output codewords */
static long             sizei;          /* Size of input buffer */
static int             *list;           /* List of different symbol */
static int              EOF_symb;
static unsigned char   *ptri;
static float           *ptro;  



static void
REALLOCATE_OUTPUT(output)

Fimage           output;

{
  int              i;
  Fimage           bufcomp;
  long             size;

  size = output->ncol * output->nrow;
  printf("Reallocation of Output.\n");

  bufcomp = mw_new_fimage();
  bufcomp = mw_change_fimage(bufcomp, output->nrow,  output->ncol);
  if (bufcomp == NULL)
    mwerror(FATAL, 1, "Memory allocation refused for reallocation buffer!\n");
  for (i = 0; i < size; i++)
    bufcomp->gray[i] = output->gray[i];

  output = mw_change_fimage(output, output->nrow * 2,  output->ncol);
  if (output == NULL)
    mwerror(FATAL, 1, "Memory allocation refused for reallocated output!\n");

  for (i = 0; i < size; i++) {
    output->gray[i] = bufcomp->gray[i];
    output->gray[i+size] = 0;
  }

  ptro = output->gray;
  ptro += size - 1;

  mw_delete_fimage(bufcomp);
}



static void
START_MODEL(histo, cap_histo, predict)

Fsignal      histo;
int         *cap_histo;
int          predict;

{
  int     y, z;

  if (histo)
    if (nsymbol != histo->size + 1) {
      mwerror(WARNING, 0, "Bad size of histogram! ->histogram ignored.\n");
      histo = NULL;
    }

  nsymbol++;
  EOF_symb = nsymbol - 1;
    
  /*--- Prepare predictive encoding if selected ---*/
    
  if (predict == 1) 
    nsymbol_pred = nsymbol - 1;
  else
    nsymbol_pred = 1;




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


    if (cap_histo) {
      max_freq = *cap_histo;
      if (max_freq <= cum_freq[0][0])
	max_freq = 100 * nsymbol;
    } else
      max_freq = 100 * nsymbol;

    if (max_freq > 1 + (top_value + 1) / 4)
      max_freq = 1 + (top_value + 1) / 4;

  } else
    {
      cum_freq[0][nsymbol] = 0;
      for (z = nsymbol; z > 0; z--) {
	freq[0][z] = nint(histo->values[z-1]);
	cum_freq[0][z-1] = cum_freq[0][z] + freq[0][z];
      }
      freq[0][0] = 0;

    } 


}



static void
UPDATE_MODEL(symbol, symbol_pred)

int        symbol;
int        symbol_pred;

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



static int 
READ_BIT()

{
  int           bit;

  if (bits_to_go == 0) {
    if (ncwread == sizei) {
      garbage_bits++;
      bits_to_go = 1;
      if (garbage_bits > code_value_bits - 2)
	mwerror(FATAL, 1, "Buffer ended to soon while decoding a symbol!\n");
    } else 
      {
	ptri++;
	buffer = *ptri;
	bits_to_go = 8;
	ncwread += 1;
      }
  }

  bit = buffer&1;
  buffer >>= 1;
  bits_to_go -= 1;
  nbitread++;
  return bit;
}



static void
DECODE_INT(symb, max)

long           *symb, max;

{
  int bit;

  *symb = 0;
  while (max > 0) {
    *symb *= 2;
    bit = READ_BIT();
    if (bit)
      *symb += 1;
    max /= 2;
  }
   
}



static void
READ_HEADER(input, nsymb, predic, predict, print)

Cimage            input;
int              *nsymb;
int              *predic;
long             *predict;
int              *print;

{

  sizei = input->nrow * input->ncol;
  if (sizei == 0)
    mwerror(FATAL, 1, "Compressed file empty!\n");

  if (input->firstcol > 0) {
    ncwread = input->firstcol;
    ptri = input->gray + ncwread - 1;
  } else
    {
      ncwread = 1;
      ptri = input->gray;
    }
  buffer = *ptri;
  if (input->firstrow > 0) 
    bits_to_go = input->firstrow;
  else
      bits_to_go = 8;
  buffer >>= 8 - bits_to_go;

  garbage_bits = 0;
  nbitread = 0;
  if (!nsymb)
    DECODE_INT(&nsymbol, (long) 1 << (code_value_bits - 3));
  if (!print)
    printf("Number of symbols : %d\n", nsymbol);
  if (!predic) 
    DECODE_INT(predict, 1L);
  else
    *predict = *predic;
  if ((*predict == 1) && !print)
    printf("Predictive encoding.\n");
  

}




static int
DECODE_SYMBOL(symbol_pred, low, high)

int        symbol_pred;
long      *low, *high;         /* Interval extremities for arithmetic coding */

{
  int          symbol;
  int          cum;
  long         range;

  /*--- find symbol ---*/

  range = (long) *high - *low + 1;
  cum = (((long) (value - *low) + 1) * cum_freq[symbol_pred][0] - 1) / range;
  for (symbol = 1; cum_freq[symbol_pred][symbol] > cum; symbol++);
  *high = *low + (range * cum_freq[symbol_pred][symbol - 1]) / cum_freq[symbol_pred][0] - 1;
  *low = *low + (range * cum_freq[symbol_pred][symbol]) / cum_freq[symbol_pred][0];

  for (;;) {
    if (*high < half) {
    } else
      if (*low >= half) {
	value -= half;
	*low -= half;
	*high -= half;
      } else
	if ((*low >= first_qrt) && (*high < third_qrt)) {
	  value -= first_qrt;
	  *low -= first_qrt;
	  *high -= first_qrt;
	} else
	  break;

    *low = 2 * *low;
    *high = 2 * *high + 1;
    value = 2 * value + READ_BIT();
  }

  return(symbol - 1);
}




void
ardecode2(Print, NRow, NSymb, Cap_Histo, Predic, Histo, Input, Rate, Output)

int          *Print;            /* Do not print info if selected */       
int          *NRow;             /* Number of row in Output */
int          *NSymb;            /* Size of alphabet for Input symbols */
long	     *Cap_Histo;	/* Capacity of histogram */
int          *Predic;           /* Apply predictive encoding */
Fsignal       Histo;            /* Histogram source symbols */
Cimage        Input;	        /* String of symbol to encode */
double       *Rate;             /* Input rate in bits per symbol */
Fimage        Output;           /* String of codewords */  

{
  long	           i;
  long             predict;            /* Flag for predictive encoding */
  long	           size;
  long             sizeo;       /* Size of output */
  long             low, high;  /* Interval extremities for arithmetic coding */
  int              ncolo, nrowo;
  int              symb, symb_pred;
  int              mindif;       /* Test value for resizing of output */
  double           sizeod;

  /*--- Initialize model of source ---*/

  if (NSymb)
    nsymbol = *NSymb;
  else
    nsymbol = 0;

  READ_HEADER(Input, NSymb, Predic, &predict, Print);

  START_MODEL(Histo, Cap_Histo, predict);


  /*--- Memory allocation for Output ---*/

  Output = mw_change_fimage(Output, Input->nrow + 1, (Input->ncol +1) * 8);
  if (Output == NULL)
    mwerror(FATAL, 1, "Memory allocation refused for `RecompImage`!\n");
  ptro = Output->gray;
  size = Output->nrow * Output->ncol;

  /*--- Initialize decoding ---*/

  sizeo = 0;
  
  if (nsymbol > 2) {
    low = 0;
    high = top_value;
    value = 0;
    for (i = 1; i <= code_value_bits; i++) 
      value = 2 * value + READ_BIT();
    symb_pred = 0;
    symb = DECODE_SYMBOL(symb_pred, &low, &high);
    if (!Histo && (predict == 0)) 
      UPDATE_MODEL(symb + 1, symb_pred);
    if (symb != EOF_symb) {
      *ptro = symb;
      sizeo++;
    }

    /*--- Decode input ---*/

    while (symb != EOF_symb) {
      if (predict == 1)
	symb_pred = symb;
      symb = DECODE_SYMBOL(symb_pred, &low, &high);
      if (!Histo) 
	UPDATE_MODEL(symb + 1, symb_pred);

      if (sizeo >= size)
	REALLOCATE_OUTPUT(Output);
      ptro++;
      *ptro = symb;
      sizeo++;
    }

    /*--- Finish decoding ---*/

    sizeo--;
    nbitread -= code_value_bits - 2;
    if (garbage_bits == 0)
      if (bits_to_go >= 2) {
	ncwread -= 2;
	bits_to_go -= 2;
      } else
	{
	  ncwread -= 1;
	  bits_to_go += 6;
	}
    else
      if (garbage_bits <= 6) {
	ncwread -= 1;
	bits_to_go = 6 - garbage_bits;
      } else
	bits_to_go = 14 - garbage_bits;

  } else
    DECODE_INT(&sizeo, (long) 1 << 30); 

  if (!Print) {
    printf("Number of input 8 bits-codewords : %d (%d bits read)\n", ncwread - Input->firstcol, nbitread);
    printf("Number of output codewords : %d\n", sizeo);
  }
  if (sizeo > 0) {
    *Rate = (double) nbitread / sizeo;
    if (!Print) 
      printf("Bit rate : %.3f\n", *Rate);
  }
  Input->firstrow = bits_to_go;
  Input->firstcol = ncwread;
  if (bits_to_go == 0)
    Input->firstcol += 1;
    

  ncolo = sizeo;
  nrowo = 1 ;
  if (NRow) {
    nrowo = *NRow;
    ncolo = sizeo / nrowo;
    if (sizeo % nrowo != 0) {
      mwerror(WARNING, 0, "Bad input for NRow!\n");
      ncolo += 1;
    }
  } 

  size = ncolo * nrowo;
  if ((nrowo > MAX_SIZEO) || (ncolo > MAX_SIZEO)) {
    nrowo = sizeo;
    ncolo = 1;
    sizeod = (double) sizeo;
    sizeod = sqrt(sizeod);
    if (((int) sizeod) + 1 > MAX_SIZEO)
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
	mindif = sizeo;
	while (i <= nrowo / i) {
	  if (nrowo / i <= MAX_SIZEO) 
	    if ((nrowo / i + 1) * i - sizeo < mindif) {
	      ncolo = i;
	      mindif = (nrowo / i + 1) * i - sizeo;
	    }
	  i++;
	}
	nrowo = sizeo / ncolo + 1;
	size = nrowo * ncolo;
	if (sizeo >= size)
	  mwerror(WARNING, 0, "Something is wrong with output dimensions!\n");
      }
  }

  Output = mw_change_fimage(Output, nrowo, ncolo);
  if (size > sizeo)
    for (i = sizeo, ptro = Output->gray + sizeo; i < size; i++, ptro++) 
      *ptro = 0;
  if (nsymbol <= 2)
    for (i = 0, ptro = Output->gray; i < sizeo; i++, ptro++) 
      *ptro = 0;

  for (i = nsymbol_pred - 1; i >= 0; i--) 
    free(cum_freq[i]);
  free(cum_freq);
  for (i = nsymbol_pred - 1; i >= 0; i--) 
    free(freq[i]);
  free(freq);


}
