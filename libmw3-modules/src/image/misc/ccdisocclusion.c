/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {ccdisocclusion};
 version = {"1.1"};
 author = {"Simon Masnou"};
 function = {"Disocclusion of a RGB Ccimage by independent processing
             of each channel in the normalized YUV representation"};
 usage = {
   'e':[energy_type=1]->energy_type
                  "Energy of a level line : 0  = only length,
                  1 = only angle, otherwise = angle+length",
   'a'->angle     "If used then the orientation of each entering
                  level line is computed more accurately on a ball
                  of radius 4",
   input->Input   "Input occluded Ccimage (RGB)",
   holes->Holes   "Input Fimage containing the only occlusions",
   output<-Output "Output disoccluded Ccimage (RGB)"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

/****************************************************************************/
/* The color cfimage in the RGB color system is first transformed into the YUV
   color system. Then a gray-level disocclusion if applied to each channel. */
/****************************************************************************/

#include <math.h>
#include <stdio.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for disocclusion() */

#define COEFF_YR 0.299
#define COEFF_YG 0.587
#define COEFF_YB 0.114

#define COEFF_NORM_B ((float) 2.0 - 2.0 * COEFF_YB)
#define COEFF_NORM_R ((float) 2.0 - 2.0 * COEFF_YR)

#define Rint(u) floor((u)+0.5)
#define Unorm(u) (((u)<0.0)?0:(((u)>255.0)?255:((Rint(u)))))

void ccdisocclusion(Ccimage Input, Ccimage Output, Fimage Holes,
                    int *energy_type, char *angle)
{
    float lum, ch1, ch2;
    unsigned char *r, *g, *b, *y, *u, *v;
    Cimage InputY = NULL, InputU = NULL, InputV = NULL;
    Cimage OutputY = NULL, OutputU = NULL, OutputV = NULL;
    int line_number, col_number, i;

    line_number = Input->nrow;
    col_number = Input->ncol;

    if ((Holes->nrow != line_number) || (Holes->ncol != col_number))
        mwerror(FATAL, 1,
                "Input and Holes images must have same dimensions\n");

    Output = mw_change_ccimage(Output, line_number, col_number);
    InputY = mw_change_cimage(InputY, line_number, col_number);
    InputU = mw_change_cimage(InputU, line_number, col_number);
    InputV = mw_change_cimage(InputV, line_number, col_number);
    OutputY = mw_change_cimage(OutputY, line_number, col_number);
    OutputU = mw_change_cimage(OutputU, line_number, col_number);
    OutputV = mw_change_cimage(OutputV, line_number, col_number);

    if ((Output == NULL) || (InputY == NULL) || (InputU == NULL)
        || (InputV == NULL) || (OutputY == NULL) || (OutputU == NULL)
        || (OutputV == NULL))
        mwerror(FATAL, 1, "Not enough memory !\n");

    r = Input->red;
    g = Input->green;
    b = Input->blue;
    y = InputY->gray;
    u = InputU->gray;
    v = InputV->gray;

    printf("RGB -> YUV conversion...\n");
    for (i = 0; i < line_number * col_number;
         i++, r++, g++, b++, y++, u++, v++)
    {
        lum =
            COEFF_YR * (float) (*r) + COEFF_YG * (float) (*g) +
            COEFF_YB * (float) (*b);
        *y = (Rint(lum));
        *u = (Rint(((float) (*b) - lum) / COEFF_NORM_B + 128.0));
        *v = (Rint(((float) (*r) - lum) / COEFF_NORM_R + 128.0));
    }
    printf("Processing of the Y channel...\n");
    disocclusion(InputY, OutputY, Holes, energy_type, angle);
    printf("Processing of the U channel...\n");
    disocclusion(InputU, OutputU, Holes, energy_type, angle);
    printf("Processing of the V channel...\n");
    disocclusion(InputV, OutputV, Holes, energy_type, angle);
    printf("YUV -> RGB conversion...\n");

    r = Output->red;
    g = Output->green;
    b = Output->blue;
    y = OutputY->gray;
    u = OutputU->gray;
    v = OutputV->gray;
    for (i = 0; i < line_number * col_number;
         i++, r++, g++, b++, y++, u++, v++)
    {
        ch1 = ((float) (*u) - 128.0) * COEFF_NORM_B + (float) (*y);
        ch2 = ((float) (*v) - 128.0) * COEFF_NORM_R + (float) (*y);
        *b = Unorm(ch1);
        *r = Unorm(ch2);
        *g = Unorm(((float) (*y) - COEFF_YR * ch2 -
                    COEFF_YB * ch1) / COEFF_YG);
    }
    mw_delete_cimage(InputY);
    mw_delete_cimage(InputU);
    mw_delete_cimage(InputV);
    mw_delete_cimage(OutputY);
    mw_delete_cimage(OutputU);
    mw_delete_cimage(OutputV);

}
