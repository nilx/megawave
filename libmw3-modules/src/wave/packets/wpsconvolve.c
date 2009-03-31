/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {wpsconvolve};
version = {"1.1"};
author = {"Francois Malgouyres"};
function = {"Convolve a signal with a filter and downsample it (or
            upsample it)"};
usage = {
'u'->upSample
        "Upsample the signal instead of downsampling it (the impulse
        response is modified)",
'o'->oddSize
        "Obtain a result with an odd size (when upsampling only)",
'h'->band
        "Compute the convolution with the corresponding high-pass
        filter (the impulse response is modified)",
signal_in->Signal
        "Input Fsignal to be convolved",
IR->Ri
        "Impulse response of the filter (the low pass filter, when the
        module is used for wavelet packet transforms)",
signal_out<-Output
        "Result of the convolution"
};
*/
/*----------------------------------------------------------------------
 v1.1: adaptation from my_sconvolve v1.0 (fpack) (JF)
----------------------------------------------------------------------*/

#include "mw3.h"
#include "mw3-modules.h"

/************************************************************/
static void barFilter(Fsignal filter, Fsignal modifiedFilter)
     /*switch left and right : modifiedFilter[n]=filter[-n] */
                                        /* original filter */
                                             /* modified filter */
{
    int n;                      /* Index of the current point in input */
    int j;                      /* Index of the current point in output */
    int tmp = filter->size / 2; /*temporary calculus             */
    float tmp1;                 /*temporary calculus (used sothat
                                 * filter and modifiedFilter can be
                                 * the same signal) */

    for (n = 0, j = filter->size - 1; n < tmp; n++, j--)
    {
        tmp1 = filter->values[n];
        modifiedFilter->values[n] = filter->values[j];
        modifiedFilter->values[j] = tmp1;
    }

    if (filter->size & 1)
        modifiedFilter->values[n] = filter->values[n];

    modifiedFilter->shift = -(filter->shift + (float) filter->size - 1.0);
}

/**************************************************/
static void changeFilter(Fsignal filter, Fsignal modifiedFilter,
                         char *upSample, char *band)
/*--- Compute the corresponding high-pass filter ---*/
/* original filter */
/* high-pass filter */
/* incates up-sampling */
/* incates band */
{
    int n;                      /* Index of the current point in input */
    int tmp;                    /* temporary calculus */

    if (upSample)
    {
        if (band)
        {
            barFilter(filter, modifiedFilter);
            modifiedFilter->shift++;    /*now, modifiedFilter[n]=filter[1-n] */

            tmp = (int) modifiedFilter->shift;

            for (n = 0; n < modifiedFilter->size; n++)
                if ((n + tmp) & 1)
                    modifiedFilter->values[n] = -modifiedFilter->values[n];
        }
        else
        {
            for (n = 0; n < filter->size; n++)
                modifiedFilter->values[n] = filter->values[n];

            modifiedFilter->shift = filter->shift;

        }
    }
    else
    {
        if (band)
        {
            barFilter(filter, modifiedFilter);
            modifiedFilter->shift++;    /*now, modifiedFilter[n]=filter[1-n] */

            tmp = (int) modifiedFilter->shift;

            for (n = 0; n < modifiedFilter->size; n++)
                if ((n + tmp) & 1)
                    modifiedFilter->values[n] = -modifiedFilter->values[n];

            barFilter(modifiedFilter, modifiedFilter);
        }
        else
            barFilter(filter, modifiedFilter);

    }

}

/**************************************************/
static void extendInput(Fsignal input, Fsignal output, int shift,
                        char *upSample)
/*--- Extends the input signal ---*/
/* original signal */
/* extended signal */
/* shift of the filter */
/* incates up-sampling */
{
    long iInput;                /* Index of the current point in input */
    long iOutput;               /* Index of the current point in output */
    long lEnd;                  /* End of the left extenssion */
    float oddSize;              /* an extra term is added at the end
                                 * of the input signal if its size is
                                 * odd */

    if (upSample == NULL)
    {
        if (input->size & 1)
        {
            shift++;
            oddSize =
                (input->values[0] + input->values[input->size - 1]) / 2.;
        }

        lEnd = -shift;

        for (iOutput = 0, iInput = input->size + shift; iOutput < lEnd;
             iOutput++, iInput++)
            output->values[iOutput] = input->values[iInput];

        if (input->size & 1)
        {
            output->values[iOutput] = oddSize;
            iOutput++;
        }

        for (iInput = 0; iInput < input->size; iOutput++, iInput++)
            output->values[iOutput] = input->values[iInput];

        if (input->size & 1)
        {
            output->values[iOutput] = oddSize;
            iOutput++;
        }

        for (iInput = 0; iOutput < output->size; iOutput++, iInput++)
            output->values[iOutput] = input->values[iInput];
    }
    else
    {
        lEnd = -shift;

        iOutput = 0;
        iInput = input->size - lEnd / 2;
        if ((lEnd & 1) && (lEnd > 0))
        {
            output->values[0] = 0.;
            iOutput++;
        }

        for (; iOutput < lEnd; iOutput++, iInput++)
        {
            output->values[iOutput] = input->values[iInput];

            iOutput++;
            output->values[iOutput] = 0.;
        }

        for (iInput = 0; iInput < input->size; iOutput++, iInput++)
        {
            output->values[iOutput] = input->values[iInput];

            iOutput++;
            output->values[iOutput] = 0.;
        }

        lEnd = (output->size - iOutput) / 2;

        for (iInput = 0; iInput < lEnd; iOutput++, iInput++)
        {
            output->values[iOutput] = input->values[iInput];

            iOutput++;
            output->values[iOutput] = 0.;
        }
        if (iOutput < output->size)
            output->values[iOutput] = input->values[iInput];
    }
}

/**************************************************/
static void convDown(Fsignal signal, Fsignal filter, Fsignal result)
/*--- Convolution of the input ---*/
/*--- 'signal' with 'filter' and a downsampling ---*/
/* original signal */
/* convolution filter */
/* filtered signal */
{
    long iResult;               /* Index of the current point in result */
    long sSignal;               /* Shift in the signal */
    int iFilter;                /* Index of the current point in filter */
    double s;                   /* Partial sum for edge processing */

    for (iResult = 0, sSignal = filter->size - 1; iResult < result->size;
         iResult++, sSignal += 2)
    {
        s = 0.;
        for (iFilter = 0; iFilter < filter->size; iFilter++)
            s += filter->values[iFilter] * signal->values[sSignal - iFilter];

        result->values[iResult] = s;
    }

}

/**************************************************/
static void convUp(Fsignal signal, Fsignal filter, Fsignal result)
/*--- Convolution of the input ---*/
/*--- 'signal' with 'filter'  ---*/
/* original signal */
/* convolution filter */
/* filtered signal */
{
    long iResult;               /* Index of the current point in result */
    long sSignal;               /* Shift in the signal */
    int iFilter;                /* Index of the current point in filter */
    double s;                   /* Partial sum for edge processing */

    for (iResult = 0, sSignal = filter->size - 1; iResult < result->size;
         iResult++, sSignal++)
    {
        s = 0.;
        for (iFilter = 0; iFilter < filter->size; iFilter++)
            s += filter->values[iFilter] * signal->values[sSignal - iFilter];

        result->values[iResult] = s;
    }

}

/**************************************************/

void wpsconvolve(Fsignal Signal, Fsignal Output, Fsignal Ri, char *upSample,
                 char *band, char *oddSize)
/*----- Convolves `Signal` with `Ri`, and decimate the reult-----*/
/*----- orinterpolate it before the convolution  -----*/
/* Input signal */
/* Output : convolved signal */
/* to upsample the signal */
/* to obtain an upsampled result with an odd size */
/* Indicates convolution with low or  high-pass filter */
/* Impulse response of the low pass filter */
{
    long sizeres;               /* Size of `Output` */
    Fsignal extendedSignal;     /* Extenssion of the input to deal with edges */
    Fsignal modifiedFilter;     /* modification of the filter */

    if (upSample == NULL && oddSize)
        mwerror(FATAL, 1,
                "oddSize option should be used with upSample option!\n");

    if (upSample == NULL)
    {
        if (Signal->size & 1)   /* the size of the signal is odd */
            sizeres = (Signal->size + 1) / 2;
        else                    /* the size of the signal is even */
            sizeres = Signal->size / 2;

        if ((extendedSignal =
             mw_change_fsignal(NULL, Signal->size + Ri->size)) == NULL)
            mwerror(FATAL, 1, "Not enough memory !\n");
    }
    else
    {
        if (oddSize)
            sizeres = Signal->size * 2 - 1;
        else
            sizeres = Signal->size * 2;

        if ((extendedSignal =
             mw_change_fsignal(NULL, sizeres + Ri->size)) == NULL)
            mwerror(FATAL, 1, "Not enough memory !\n");
    }

    if ((modifiedFilter = mw_change_fsignal(NULL, Ri->size)) == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");

    Output = mw_change_fsignal(Output, sizeres);
    if (Output == NULL)
        mwerror(FATAL, 1, "Not enough memory for output!\n");

    changeFilter(Ri, modifiedFilter, upSample, band);
    extendInput(Signal, extendedSignal, (int) modifiedFilter->shift,
                upSample);

    if (upSample == NULL)
        convDown(extendedSignal, modifiedFilter, Output);
    else
        convUp(extendedSignal, modifiedFilter, Output);

    mw_delete_fsignal(extendedSignal);
    extendedSignal = NULL;
    mw_delete_fsignal(modifiedFilter);
    modifiedFilter = NULL;

}
