/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {one_levelset};
 version = {"1.2"};
 author = {"Georges Koepfler"};
 function = {"Get boundaries of level set, using a
             simplified merging criterion in the 'well-known'
             segmentation algorithm"};
 usage = {
   'l':[level=127.0]->level     [0.0,4e4]
       "pixels <=`level' (float) belong to the level set",
   'b':boundary<-cb
       "output boundary of levelset, file cimage formated",
   'p':polygons<-pb
       "output boundary of levelset, file fpolygons formated",
   'G':f_levelset<-fu
       "output levelset with gray-`level', file fimage formated",
   'B':b_levelset<-cu
       "output levelset b/w, file cimage formated",
   fimage->image_org
       "original image"
};
*/
/*----------------------------------------------------------------------
 v1.1: fixed ambiguous static/non static definitions (LM)
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "mw.h"
#include "mw-modules.h"

/* Define the max,min of two quantities */
#define   MAX(A,B)          ((A) > (B)) ?  (A) : (B)
#define   MIN(A,B)          ((A) < (B)) ?  (A) : (B)

#define VRAI        1           /*      Valeurs                     */
#define FAUX        0           /*          logiques                */

/* #define NC_REG      1 !         Nombre de canaux pour une region */
#define NC_BOR      1           /* Nombre de canaux pour un bord    */
#define NC_SOM      0           /* Nombre de canaux pour un sommet  */

/* Structure definitions for segmentation */

typedef struct li_pixels {
    char direction;
    unsigned short longueur;
    struct li_pixels *suiv;
} LI_PIXELS, *LI_PIXELSPTR;

typedef struct li_bords {
    struct bord *bord;
    struct li_bords *suiv;
} LI_BORDS, *LI_BORDSPTR;

typedef struct region {
    float *canal;               /* in this algorithm only the gray level */
    LI_BORDSPTR Lbords;         /* is of interest !                      */
    struct region *Rprec, *Rsuiv;
} REGION, *REGIONPTR;

typedef struct bord {           /*                              */
    float canal;                /*       sa          lab   lba  */
    REGIONPTR rg, rd;           /*        |           |     ^   */
    struct bord_cnxe *cnxe;     /*    rg  |  rd   ;   |     |   */
} BORD, *BORDPTR;               /*        |           |     |   */
                                /*       sb           v     |   */
                                /*                              */
typedef struct bord_cnxe {
    struct sommet *sa, *sb;
    LI_PIXELSPTR lab, lba;
    struct bord_cnxe *suiv;
} BORDCONNEXE, *BORDCONNEXEPTR;

typedef struct sommet {
/*Not used in this version
    float             canal[NC_SOM];
*/
    unsigned short x, y;
    LI_BORDSPTR Lbords;
} SOMMET, *SOMMETPTR;

typedef struct modele {
    unsigned short gx, gy;      /* Dimensions de la grille           */
    unsigned long nbregions;    /* Nombre de regions                 */
    unsigned long nbbords, nbsommets;   /* Nombre de bords,resp. de sommets  */
    REGIONPTR regpixel0_0;      /* Region qui contient l'origine     */
    REGIONPTR tete;             /* Tete de la liste image            */
    /* pointers to the 'callocated' memory blocks                           */
    float *b_data;              /* float-region-canal-block          */
    REGIONPTR b_reg;            /* region-block                      */
    BORDPTR b_bor;              /* bords-block                       */
    SOMMETPTR b_som;            /* sommets-block                     */
    BORDCONNEXEPTR b_borcnxe;   /* connected comp-block              */
    LI_BORDSPTR b_libor1, b_libor2;     /* bords-liste-block                 */
    LI_PIXELSPTR b_lipix;       /* pixels-liste-block                */
} MODELE, *MODELEPTR;

/* DECLARATION DE LA VARIABLE GLOBALE */

static MODELE image;            /* Image sous la forme du modele */

/* declare functions */

static void InitPixel(LI_PIXELSPTR pixelptr, char dir);
static void LiBordsUnion(REGIONPTR reg, REGIONPTR regvois, BORDPTR bordcom),
ElimBordeSom(BORDPTR bord), DegreSommet(SOMMETPTR som);
static void ElimLiReg(REGIONPTR regvois), UnionBordCnxe(REGIONPTR reg);
static void Union1Bord(SOMMETPTR som), Union2Bords(SOMMETPTR som);
static LI_PIXELSPTR LiPixelsUnion(LI_PIXELSPTR liste1, LI_PIXELSPTR liste2);
static void Repointer(SOMMETPTR som, BORDPTR newbord, BORDPTR oldbord),
ElimBordeReg(REGIONPTR reg, BORDPTR bord);
static void RegMerge(REGIONPTR reg, REGIONPTR regvois, BORDPTR bordcom);
static short TraitHVmono(Cimage whitesheet, short int a0, short int b0,
                         short int a1, short int b1, unsigned char c);
static REGIONPTR RegAdjD(REGIONPTR reg, short int i, short int j),
RegAdjB(REGIONPTR reg, short int i, short int j);

/* Channel initilization functions */

static void InitPixel(LI_PIXELSPTR pixelptr, char dir)
/* Met un segment de longeur 1     et de */
/*  direction 'dir' dans  'pixelptr'.    */
{
    pixelptr->direction = dir;
    pixelptr->longueur = 1;
    pixelptr->suiv = NULL;
}

/* Initilization of the intern image model */

static void Estimation(void)    /* Estimations occupation memoire */
{
    unsigned long simage, stotal, total,
        sregion, sbord, sbordcnxe, ssommet, sli_bords, sli_pixels,
        sdata, nbregions, nbbords, nbsommets, systeme;

    sregion = sizeof(REGION);
    sbord = sizeof(BORD);
    sbordcnxe = sizeof(BORDCONNEXE);
    ssommet = sizeof(SOMMET);
    sli_bords = sizeof(LI_BORDS);
    sli_pixels = sizeof(LI_PIXELS);
    sdata = sizeof(float);

    nbregions = (image.gx * image.gy);
    nbbords = (image.gx * (image.gy - 1) + image.gy * (image.gx - 1));
    nbsommets = ((image.gx + 1) * (image.gy + 1) - 4);

    simage = nbregions * (sregion + sdata)
        + nbbords * (sbord + sbordcnxe)
        + nbsommets * ssommet + 2 * nbbords * (2 * sli_bords + sli_pixels);

    systeme = 200000.0;         /* On utilise approximativement 200 kO */
    /* pour la reservation par 7 'calloc()' */

    stotal = systeme + image.gx * image.gy * sizeof(float);
    total = simage + stotal;
    printf("\nEstimation for memory occupation: %lu Mbytes\n", total >> 20);
    return;
}

static void Initialisation(Fimage image_org)
/* On cree huit blocs pour reserver de la memoire      */
/* aux regions,bords et sommets,... ; Grace a un codage */
/* des places memoire on peut facilement initialiser   */
{
    unsigned short i, j;
    /* les pointeurs des differentes structures.           */
    long l;
    unsigned long dbreg, dbbor, dbsom, dblibor, dblipix,
        /* Dimensions des blocs */
     lbreg, lbbor, lbsom;       /* Largeur des blocs    */

    REGIONPTR block_reg,        /* Espace pour les regions        */
     regptr1, regptr2;
    BORDPTR block_bor;          /* Espace pour les bords          */
    SOMMETPTR block_som;        /* Espace pour les sommets        */
    BORDCONNEXEPTR block_borcnxe;       /* Espace pour les comp. connexes */
    LI_BORDSPTR block_libor1, block_libor2,
        /* Espace pour el. liste  de bords */
     liborptr1, liborptr2;
    LI_PIXELSPTR block_lipix;   /* Espace pour liste de pixels    */
    float *block_data;          /* Espace pour les canaux         */

    printf("\nInitialization\n");
    lbreg = image.gx;
    dbreg = lbreg * image.gy;
    lbbor = 2 * image.gx - 1;
    dbbor = lbbor * image.gy;
    lbsom = image.gx + 1;
    dbsom = lbsom * (image.gy + 1);
    dblibor = dblipix =
        2 * ((image.gx - 1) * image.gy + image.gx * (image.gy - 1)) + 1L;

    block_reg = (REGIONPTR) calloc((long) dbreg, sizeof(REGION));
    block_data = (float *) calloc((long) dbreg, sizeof(float));
    block_bor = (BORDPTR) calloc((long) dbbor, sizeof(BORD));
    block_som = (SOMMETPTR) calloc((long) dbsom, sizeof(SOMMET));
    block_borcnxe =
        (BORDCONNEXEPTR) calloc((long) dbbor, sizeof(BORDCONNEXE));
    block_libor1 = (LI_BORDSPTR) calloc((long) dblibor, sizeof(LI_BORDS));
    block_libor2 = (LI_BORDSPTR) calloc((long) dblibor, sizeof(LI_BORDS));
    block_lipix = (LI_PIXELSPTR) calloc((long) dblipix, sizeof(LI_PIXELS));

    if (block_lipix == NULL || block_libor2 == NULL || block_libor1 == NULL
        || block_borcnxe == NULL || block_som == NULL || block_bor == NULL
        || block_reg == NULL || block_data == NULL)
        mwerror(FATAL, 1, "\n Not enough memory for initialisation!!\n");

    /*                                     */
    /*  CODAGE DES REGIONS,BORDS,SOMMETS   */
    /*     DANS LES BLOCS RESPECTIFS       */
    /*                                     */
    /*             b[2i,j-1]               */
    /*        s[i,j]       s[i+1,j]        */
    /*            S---------S              */
    /*            |         |              */
    /*  b[2i-1,j] |  r[i,j] |  b[2i+1,j]   */
    /*            |         |              */
    /*            S---------S              */
    /*      s[i,j+1]       s[i+1,j+1]      */
    /*              b[2i,j]                */
    /*                                     */
    /*                                     */

    image.tete = block_reg;
    image.regpixel0_0 = image.tete;
    image.nbregions = image.gx * image.gy;
    image.nbbords = image.gx * (image.gy - 1) + (image.gx - 1) * image.gy;
    image.nbsommets = (image.gx + 1) * (image.gy + 1) - 4;

    image.b_data = block_data;
    image.b_reg = block_reg;
    image.b_bor = block_bor;
    image.b_som = block_som;
    image.b_borcnxe = block_borcnxe;
    image.b_libor1 = block_libor1;
    image.b_libor2 = block_libor2;
    image.b_lipix = block_lipix;

    for (l = 0; l < image.gx * image.gy; l++)   /* copy the gray-level */
        block_data[l] = image_org->gray[l];

    printf("   ...of the regions,   initial number of regions    %ld\n",
           image.nbregions);
    regptr1 = image.tete;
    for (j = 0; j < image.gy; j++)
        for (i = 0; i < image.gx; i++)
        {
            regptr1->Rprec = ((i == 0) && (j == 0)) ? NULL : regptr2;
            regptr2 = regptr1;
            liborptr1 = liborptr2 = NULL;
            regptr2->canal = block_data;        /* avoid computations */
            if (i != image.gx - 1)
            {                   /* Bord droit */
                liborptr2 = block_libor1++;
                liborptr2->bord =
                    (block_bor + ((long) (2 * i + 1) + lbbor * j));
                liborptr1 = liborptr2;
            }
            if (j != 0)
            {                   /* Bord dessus */
                if (liborptr2 == NULL)
                    liborptr2 = block_libor1++;
                else
                {
                    liborptr2->suiv = block_libor1++;
                    liborptr2 = liborptr2->suiv;
                }
                liborptr2->bord =
                    (block_bor + ((long) (2 * i) + lbbor * (j - 1)));
                if (liborptr1 == NULL)
                    liborptr1 = liborptr2;
            }
            if (i != 0)
            {                   /* Bord gauche */
                if (liborptr2 == NULL)
                    liborptr2 = block_libor1++;
                else
                {
                    liborptr2->suiv = block_libor1++;
                    liborptr2 = liborptr2->suiv;
                }
                liborptr2->bord =
                    (block_bor + ((long) (2 * i - 1) + lbbor * j));
                if (liborptr1 == NULL)
                    liborptr1 = liborptr2;
            }
            if (j != image.gy - 1)
            {                   /* Bord dessous */
                if (liborptr2 == NULL)
                    liborptr2 = block_libor1++;
                else
                {
                    liborptr2->suiv = block_libor1++;
                    liborptr2 = liborptr2->suiv;
                }
                liborptr2->bord = (block_bor + ((long) (2 * i) + lbbor * j));
                if (liborptr1 == NULL)
                    liborptr1 = liborptr2;
            }
            liborptr2->suiv = NULL;
            regptr2->Lbords = liborptr1;
            regptr1 = ((i == image.gx - 1) && (j == image.gy - 1)) ?
                NULL : (block_reg + ((long) j * lbreg + i + 1));
            regptr2->Rsuiv = regptr1;
            block_data++;       /* update the data pointer */
        }

    printf("   ...of the boundary,  initial number of frontiers  %ld\n",
           image.nbbords);
    for (j = 0; j < image.gy; j++)
        for (i = 0; i < image.gx; i++)
        {
            /*                 */
            /*       rg        */
            /* sb --------> sa */
            /*       rd        */
            /*                 */

            /*             */
            /*     'h'     */
            /* 'g' SOM 'd' */
            /*     'b'     */
            /*             */
            if (j != image.gy - 1)
            {
                l = (long) 2 *i + lbbor * j;
                block_bor[l].canal = 1.0;
                block_bor[l].rg = block_reg + (long) i + lbreg * j;

                block_bor[l].rd = block_reg + (long) i + lbreg * (j + 1);
                block_bor[l].cnxe = block_borcnxe + l;
                block_bor[l].cnxe->sa = block_som + (long) (i + 1) +
                    lbsom * (j + 1);
                block_bor[l].cnxe->sb =
                    block_som + (long) i + lbsom * (j + 1);
                block_bor[l].cnxe->lab = block_lipix++;
                InitPixel(block_bor[l].cnxe->lab, 'g');
                block_bor[l].cnxe->lba = block_lipix++;
                InitPixel(block_bor[l].cnxe->lba, 'd');
                block_bor[l].cnxe->suiv = NULL;
            }
            /*       */
            /*  sa   */
            /*   ^   */
            /*   |   */
            /* rg|   */
            /*   |rd */
            /*   |   */
            /*  sb   */
            /*       */
            if (i != image.gx - 1)
            {
                l = (long) 2 *i + 1 + lbbor * j;
                block_bor[l].canal = 1.0;
                block_bor[l].rg = block_reg + (long) i + lbreg * j;
                block_bor[l].rd = block_reg + (long) (i + 1) + lbreg * j;
                block_bor[l].cnxe = block_borcnxe + l;
                block_bor[l].cnxe->sa =
                    block_som + (long) (i + 1) + lbsom * j;
                block_bor[l].cnxe->sb =
                    block_som + (long) (i + 1) + lbsom * (j + 1);
                block_bor[l].cnxe->lab = block_lipix++;
                InitPixel(block_bor[l].cnxe->lab, 'b');
                block_bor[l].cnxe->lba = block_lipix++;
                InitPixel(block_bor[l].cnxe->lba, 'h');
                block_bor[l].cnxe->suiv = NULL;
            }
        }

    printf("   ...of tips,          initial number of tips       %ld\n",
           image.nbsommets);
    for (j = 0; j <= image.gy; j++)
        for (i = 0; i <= image.gx; i++)
        {
            liborptr1 = liborptr2 = NULL;
            l = (long) i + lbsom * j;
            if (((i != 0) || (j != 0 && j != image.gy))
                && ((i != image.gx) || (j != 0 && j != image.gy)))
            {
                block_som[l].x = i;
                /*On place la frontiere dans la region droite */
                /*            respectivement la region desous */
                block_som[l].y = j;
                if ((i != 0) && (i != image.gx) && (j != image.gy))
                {               /* Bord en dessous */
                    liborptr2 = block_libor2++;
                    liborptr2->bord =
                        block_bor + (long) (2 * i - 1) + lbbor * j;
                    liborptr1 = liborptr2;
                }
                if ((i != image.gx) && (j != 0) && (j != image.gy))
                {               /* Bord a droite   */
                    if (liborptr2 == NULL)
                        liborptr2 = block_libor2++;
                    else
                    {
                        liborptr2->suiv = block_libor2++;
                        liborptr2 = liborptr2->suiv;
                    }
                    liborptr2->bord =
                        block_bor + (long) (2 * i) + lbbor * (j - 1);
                    if (liborptr1 == NULL)
                        liborptr1 = liborptr2;
                }
                if ((i != 0) && (i != image.gx) && (j != 0))
                {               /* Bord en dessus  */
                    if (liborptr2 == NULL)
                        liborptr2 = block_libor2++;
                    else
                    {
                        liborptr2->suiv = block_libor2++;
                        liborptr2 = liborptr2->suiv;
                    }
                    liborptr2->bord =
                        block_bor + (long) (2 * i - 1) + lbbor * (j - 1);
                    if (liborptr1 == NULL)
                        liborptr1 = liborptr2;
                }
                if ((i != 0) && (j != 0) && (j != image.gy))
                {               /* Bord a gauche   */
                    if (liborptr2 == NULL)
                        liborptr2 = block_libor2++;
                    else
                    {
                        liborptr2->suiv = block_libor2++;
                        liborptr2 = liborptr2->suiv;
                    }
                    liborptr2->bord =
                        block_bor + (long) 2 *(i - 1) + lbbor * (j - 1);
                    if (liborptr1 == NULL)
                        liborptr1 = liborptr2;
                }
                liborptr2->suiv = NULL;
                block_som[l].Lbords = liborptr1;
            }
        }
    printf(" finished !\n");
}

/* free all the 'callocated' memory blocks */
static void Image_free(MODELEPTR imageptr)
{
    free((void *) imageptr->b_data);
    free((void *) imageptr->b_reg);
    free((void *) imageptr->b_bor);
    free((void *) imageptr->b_som);
    free((void *) imageptr->b_borcnxe);
    free((void *) imageptr->b_libor1);
    free((void *) imageptr->b_libor2);
    free((void *) imageptr->b_lipix);
}

/* Merging tools */

static void RegMerge(REGIONPTR reg, REGIONPTR regvois, BORDPTR bordcom)
/* On agglutine les regions reg et regvois */
/* de bord commun bordcom,dans l'espace    */
/* memoire de reg                          */
{
    BORDCONNEXEPTR cnxeptr;

    *reg->canal = MAX(*reg->canal, *regvois->canal);

    if (image.regpixel0_0 == regvois)
        image.regpixel0_0 = reg;

    ElimLiReg(regvois);

    LiBordsUnion(reg, regvois, bordcom);

    ElimBordeSom(bordcom);

    image.nbregions--;
    image.nbbords--;

    for (cnxeptr = bordcom->cnxe; cnxeptr != NULL; cnxeptr = cnxeptr->suiv)
    {
        DegreSommet(cnxeptr->sa);
        if (cnxeptr->sb != cnxeptr->sa)
            DegreSommet(cnxeptr->sb);
    }
    UnionBordCnxe(reg);
}

static void ElimLiReg(REGIONPTR regvois)
/* Elimine la region regvois de la liste */
/* des regions,exit si deux regions      */
/* Utiliser avant LiBordsUnion()         */
{

    if (((regvois->Rprec == NULL) && (regvois->Rsuiv->Rsuiv == NULL)) ||
        ((regvois->Rsuiv == NULL) && (regvois->Rprec->Rprec == NULL)))
    {
        printf("\nThere is just one region which stays, no boundaries!\n");
        exit(0);
    }
    if (regvois->Rprec == NULL)
    {
        image.tete = regvois->Rsuiv;
        image.tete->Rprec = NULL;
    }
    else if (regvois->Rsuiv == NULL)
        regvois->Rprec->Rsuiv = NULL;
    else
    {
        regvois->Rprec->Rsuiv = regvois->Rsuiv;
        regvois->Rsuiv->Rprec = regvois->Rprec;
    }
}

static void LiBordsUnion(REGIONPTR reg, REGIONPTR regvois, BORDPTR bordcom)
/*  Fait l'union,suivant l'ordre de    */
/*  parcours de la frontiere de reg+regvois,de */
/*  la liste de bords de reg et regvois.       */
/* En eliminant l'element pointant sur bordcom */
{
    LI_BORDSPTR li, L1, L2;

    /* Marquer bordcom dans la liste des bords de reg */
    if (reg->Lbords->bord == bordcom)
        L1 = reg->Lbords;
    else
    {
        for (li = reg->Lbords; li->suiv->bord != bordcom; li = li->suiv);
        L1 = li->suiv;
    }

    /* Les bords de regvois doivent pointer vers reg,marquer bordcom */
    for (li = regvois->Lbords; li != NULL; li = li->suiv)
    {
        if (li->bord == bordcom)
            L2 = li;
        if (li->bord->rg == regvois)
            li->bord->rg = reg;
        else
            li->bord->rd = reg;
    }

    /* Mise en place ,dans l'ordre, des bords de reg (sens de rot. +). */
    /* Si  reg->Lbords = a-L1-b  et regvois->Lbords = c-L2-d  ,       */
    /*  alors le resultat est   reg->Lbords = a-d-c-b .               */
    if (reg->Lbords == L1)
        if (regvois->Lbords == L2)
            if (L1->suiv == NULL)
                if (L2->suiv == NULL)
                {               /*   L1-0    et   L2-0   ,li=0        */
                    li = NULL;
                }
                else
                {               /*   L1-0    et   L2-d   ,li=d        */
                    li = L2->suiv;
                }
            else if (L2->suiv == NULL)
            {                   /*   L1-b    et   L2-0   ,li=b        */
                li = L1->suiv;
            }
            else
            {                   /*   L1-b    et   L2-d   ,li=d-b      */
                for (li = L2; li->suiv != NULL; li = li->suiv);
                li->suiv = L1->suiv;
                li = L2->suiv;
            }
        else if (L1->suiv == NULL)
            if (L2->suiv == NULL)
            {                   /*   L1-0    et   c-L2-0 ,li=c        */
                for (li = regvois->Lbords; li->suiv != L2; li = li->suiv);
                li->suiv = NULL;
                li = regvois->Lbords;
            }
            else
            {                   /*   L1-0    et   c-L2-d ,li=d-c      */
                for (li = L2; li->suiv != L2; li = li->suiv)
                    if (li->suiv == NULL)
                        li->suiv = regvois->Lbords;
                li->suiv = NULL;
                li = L2->suiv;
            }
        else if (L2->suiv == NULL)
        {                       /*   L1-b    et   c-L2-0 ,li=c-b      */
            for (li = regvois->Lbords; li->suiv != L2; li = li->suiv);
            li->suiv = L1->suiv;
            li = regvois->Lbords;
        }
        else
        {                       /*   L1-b    et   c-L2-d  ,li=d-c-b   */
            for (li = L2; li->suiv != L2; li = li->suiv)
                if (li->suiv == NULL)
                    li->suiv = regvois->Lbords;
            li->suiv = L1->suiv;
            li = L2->suiv;
        }
    else if (regvois->Lbords == L2)
        if (L1->suiv == NULL)
            if (L2->suiv == NULL)
            {                   /*   a-L1-0  et   L2-0    ,li=a       */
                for (li = reg->Lbords; li->suiv != L1; li = li->suiv);
                li->suiv = NULL;
                li = reg->Lbords;
            }
            else
            {                   /*   a-L1-0  et   L2-d    ,li=a-d     */
                for (li = reg->Lbords; li->suiv != L1; li = li->suiv);
                li->suiv = L2->suiv;
                li = reg->Lbords;
            }
        else if (L2->suiv == NULL)
        {                       /*   a-L1-b  et   L2-0    ,li=a-b     */
            for (li = reg->Lbords; li->suiv != L1; li = li->suiv);
            li->suiv = L1->suiv;
            li = reg->Lbords;
        }
        else
        {                       /*   a-L1-b  et   L2-d    ,li=a-d-b   */
            for (li = reg->Lbords; li->suiv != NULL; li = li->suiv)
                if (li->suiv == L1)
                    li->suiv = L2->suiv;
            li->suiv = L1->suiv;
            li = reg->Lbords;
        }
    else if (L1->suiv == NULL)
        if (L2->suiv == NULL)
        {                       /*   a-L1-0  et   c-L2-0  ,li=a-c     */
            for (li = reg->Lbords; li->suiv != L2; li = li->suiv)
                if (li->suiv == L1)
                    li->suiv = regvois->Lbords;
            li->suiv = NULL;
            li = reg->Lbords;
        }
        else
        {                       /*   a-L1-0  et   c-L2-d  ,li=a-d-c   */
            for (li = reg->Lbords; li->suiv != L2; li = li->suiv)
            {
                if (li->suiv == L1)
                    li->suiv = L2->suiv;
                if (li->suiv == NULL)
                    li->suiv = regvois->Lbords;
            }
            li->suiv = NULL;
            li = reg->Lbords;
        }
    else if (L2->suiv == NULL)
    {                           /*   a-L1-b  et   c-L2-0  ,li=a-c-b   */
        for (li = reg->Lbords; li->suiv != L2; li = li->suiv)
            if (li->suiv == L1)
                li->suiv = regvois->Lbords;
        li->suiv = L1->suiv;
        li = reg->Lbords;
    }
    else
    {                           /*   a-L1-b  et   c-L2-d  ,li=a-d-c-b */
        for (li = reg->Lbords; li->suiv != L2; li = li->suiv)
        {
            if (li->suiv == L1)
                li->suiv = L2->suiv;
            if (li->suiv == NULL)
                li->suiv = regvois->Lbords;
        }
        li->suiv = L1->suiv;
        li = reg->Lbords;
    }

    reg->Lbords = li;
    /* Fin mise en place des bords                                    */
}

static void ElimBordeSom(BORDPTR bord)
/* Elimine 'bord' des listes de bord des   */
/*  sommets des comp. cnxes. de 'bord'     */
{
    LI_BORDSPTR li;
    BORDCONNEXEPTR cnxeptr;

    for (cnxeptr = bord->cnxe; cnxeptr != NULL; cnxeptr = cnxeptr->suiv)
    {
        li = cnxeptr->sa->Lbords;
        if (li->bord == bord)
            cnxeptr->sa->Lbords = li->suiv;
        else
        {
            for (; (li->suiv != NULL) && (li->suiv->bord != bord);
                 li = li->suiv);
            if (li->suiv != NULL)
                /* 'bord' n'existe plus dans la liste de sa */
                li->suiv = li->suiv->suiv;
        }

        if (cnxeptr->sa == cnxeptr->sb)
            break;              /* On elimine une boucle */

        li = cnxeptr->sb->Lbords;
        if (li->bord == bord)
            cnxeptr->sb->Lbords = li->suiv;
        else
        {
            for (; (li->suiv != NULL) && (li->suiv->bord != bord);
                 li = li->suiv);
            if (li->suiv != NULL)
                /* 'bord' n'existe plus dans la liste de sb */
                li->suiv = li->suiv->suiv;
        }
    }
}

static void DegreSommet(SOMMETPTR som)
/* Calcule le degre du sommet 'som'         */
/* Suivant les cas on procede a la suite    */
{
    unsigned short degre;
    LI_BORDSPTR li;

    degre = 0;
    for (li = som->Lbords; li != NULL; li = li->suiv)
        degre++;

    if (degre == 0)
        image.nbsommets--;
    if (degre == 1)
        Union1Bord(som);
    if (degre == 2)
        Union2Bords(som);
}

static void Union1Bord(SOMMETPTR som)
/* Si 'som' appartient a deux composantes */
/*  cnxes. d'un meme bord on reunit ces   */
/*  parties connexes                      */
{
    BORDPTR bord;
    BORDCONNEXEPTR bcnxe, bcnxe1, bcnxe2;
    unsigned short compteur;

    bord = som->Lbords->bord;
    compteur = 0;
    for (bcnxe = bord->cnxe; bcnxe != NULL; bcnxe = bcnxe->suiv)
        if ((bcnxe->sa == som) || (bcnxe->sb == som))
            compteur++;

    if (compteur == 1)
        return;
    /* 'som' est sur le bord de l'image  */

    if (compteur == 0)
        return;
    /* 'som' est deja enleve de la liste des bords  */

    image.nbsommets--;          /*On enleve 'som'       */

    /* Determination des comp. cnxes. de sommet som */
    /*                                     */
    /*     (r1)            (r1)     */
    /*  sb ----> sa  $  sb ----> sa */
    /*    bcnxe1   :som:  bcnxe2    */
    /*                              */
    for (bcnxe = bord->cnxe; bcnxe != NULL; bcnxe = bcnxe->suiv)
    {
        if (bcnxe->sa == som)
            bcnxe1 = bcnxe;
        if (bcnxe->sb == som)
            bcnxe2 = bcnxe;
    }

    /* Reunir les deux bcnxes dont som est un sommet dans bcnxe1 */
    bcnxe1->sa = bcnxe2->sa;
    bcnxe1->lba = LiPixelsUnion(bcnxe1->lba, bcnxe2->lba);
    bcnxe1->lab = LiPixelsUnion(bcnxe2->lab, bcnxe1->lab);
    if (bord->cnxe == bcnxe2)
        bord->cnxe = bcnxe2->suiv;
    else
    {
        for (bcnxe = bord->cnxe; bcnxe->suiv != bcnxe2; bcnxe = bcnxe->suiv);
        bcnxe->suiv = bcnxe2->suiv;
    }
}

static void Union2Bords(SOMMETPTR som)
/* Si som est entre deux bords exactement */
/*  on reunit ces bords en un bord unique */
{
    short dir_bcnxe1 = 0, dir_bcnxe2 = 0;
    SOMMETPTR sa_old;
    BORDPTR bord1, bord2, bord;
    BORDCONNEXEPTR bcnxe1 = NULL, bcnxe2 = NULL, bcnxe;
    LI_BORDSPTR li;
    LI_PIXELSPTR lab_old;

    bord1 = som->Lbords->bord;
    bord2 = som->Lbords->suiv->bord;

    /* Est-ce que 'som' est le sommet d'un bord ferme,lacet (au moins) * */
    /* Dans ce cas on ne fait rien */

    if ((bord1->cnxe->sa == bord1->cnxe->sb)
        || (bord2->cnxe->sa == bord2->cnxe->sb))
        return;

    /*  Si som se trouve entre deux comp. cnxes. de bords differents * */
    /*  C'est le cas standard                            */

    image.nbsommets--;          /* On enleve 'som' */
    image.nbbords--;            /* De deux bords on en fait un */

    /* Determination de l'ordre des deux bords a reunir */
    for (li = bord1->rg->Lbords; li->bord != bord1; li = li->suiv);
    if (((li->suiv != NULL) && (li->suiv->bord != bord2))
        || ((li->suiv == NULL) && (bord2 != bord1->rg->Lbords->bord)))
    {
        bord = bord1;
        bord1 = bord2;
        bord2 = bord;
    }

    /* Determination des comp. cnxes. de sommet 'som' et de leur orientation */
    /* avec:                                                                 */
    /*                                                              */
    /*         bcnxe1            som        bcnxe2         */
    /*     sb -------> sa (+1)   $     sb -------> sa (+1) */
    /*     sb -------> sa (+1)   $     sa <------- sb (-1) */
    /*     sa <------- sb (-1)   $     sb -------> sa (+1) */
    /*     sa <------- sb (-1)   $     sa <------- sb (-1) */
    /*                                                     */
    /*                                                     */

    for (bcnxe = bord1->cnxe; bcnxe != NULL; bcnxe = bcnxe->suiv)
    {
        if (bcnxe->sa == som)
        {
            bcnxe1 = bcnxe;
            dir_bcnxe1 = 1;
            break;
        }
        if (bcnxe->sb == som)
        {
            bcnxe1 = bcnxe;
            dir_bcnxe1 = -1;
            break;
        }
    }

    for (bcnxe = bord2->cnxe; bcnxe != NULL; bcnxe = bcnxe->suiv)
    {
        if (bcnxe->sa == som)
        {
            bcnxe2 = bcnxe;
            dir_bcnxe2 = -1;
            break;
        }
        if (bcnxe->sb == som)
        {
            bcnxe2 = bcnxe;
            dir_bcnxe2 = 1;
            break;
        }
    }

    /* Mettre dans bord1 les deux bords: b1 <- b1+b2 */

    bord1->canal += bord2->canal;

    /* Est-ce qu'il faut changer l'orientation des bcnxes de bord2 */
    if (dir_bcnxe1 != dir_bcnxe2)
        for (bcnxe = bord2->cnxe; bcnxe != NULL; bcnxe = bcnxe->suiv)
        {
            sa_old = bcnxe->sa;
            bcnxe->sa = bcnxe->sb;
            bcnxe->sb = sa_old;
            lab_old = bcnxe->lab;
            bcnxe->lab = bcnxe->lba;
            bcnxe->lba = lab_old;
        }

    /* Repointer les sommets sur bord */
    for (bcnxe = bord2->cnxe; bcnxe != NULL; bcnxe = bcnxe->suiv)
    {
        Repointer(bcnxe->sa, bord1, bord2);
        Repointer(bcnxe->sb, bord1, bord2);
    }

    /* Mettre les bcnxes ensembles dans bord1 */
    for (bcnxe = bord1->cnxe; bcnxe->suiv != NULL; bcnxe = bcnxe->suiv);
    bcnxe->suiv = bord2->cnxe;

    /* Reunir les deux bcnxes dont som est un sommet dans bcnxe1 */
    if (dir_bcnxe1 == 1)
    {
        bcnxe1->sa = bcnxe2->sa;
        bcnxe1->lba = LiPixelsUnion(bcnxe1->lba, bcnxe2->lba);
        bcnxe1->lab = LiPixelsUnion(bcnxe2->lab, bcnxe1->lab);
    }
    else
    {
        bcnxe1->sb = bcnxe2->sb;
        bcnxe1->lba = LiPixelsUnion(bcnxe2->lba, bcnxe1->lba);
        bcnxe1->lab = LiPixelsUnion(bcnxe1->lab, bcnxe2->lab);
    }
    for (bcnxe = bord1->cnxe; bcnxe->suiv != bcnxe2; bcnxe = bcnxe->suiv);
    bcnxe->suiv = bcnxe2->suiv;

    /* Elimination de bord2 des listes de bords des regions */
    ElimBordeReg(bord1->rg, bord2);
    ElimBordeReg(bord1->rd, bord2);
}

static void UnionBordCnxe(REGIONPTR reg)
/* Reunit les bords de reg,qui font frontiere a une */
/*  meme region voisine,                            */
/*  on obtient le comp. connexes d'un bord          */
{
    short dir_bord2;
    SOMMETPTR som;
    LI_PIXELSPTR lpix;
    LI_BORDSPTR lbor1, lbor2, lbor3, lbor;
    BORDPTR bord1, bord2;
    BORDCONNEXEPTR bcnxe;
    REGIONPTR reg1, reg2;

    lbor1 = reg->Lbords;
    while ((lbor1 != NULL) && (lbor1->suiv != NULL))
    {
        bord1 = lbor1->bord;
        reg1 = (bord1->rg == reg) ? bord1->rd : bord1->rg;
        lbor2 = lbor1->suiv;
        do
        {
            bord2 = lbor2->bord;
            reg2 = (bord2->rg == reg) ? bord2->rd : bord2->rg;
            /* Est-ce qu'il y a deux bords de reg,frontiere avec une
             * meme autre region */
            if (reg1 == reg2)
            {
                image.nbbords--;        /* De deux bords on en fait un */
                dir_bord2 = (bord1->rg == bord2->rg) ? 1 : -1;
                /* Les sommets des comp. cnxes. de bord2 doivent
                 * pointer vers bord1 */
                for (bcnxe = bord2->cnxe; bcnxe != NULL; bcnxe = bcnxe->suiv)
                {
                    Repointer(bcnxe->sa, bord1, bord2);
                    Repointer(bcnxe->sb, bord1, bord2);
                    if (dir_bord2 == -1)
                    {
                        som = bcnxe->sa;
                        bcnxe->sa = bcnxe->sb;
                        bcnxe->sb = som;
                        lpix = bcnxe->lab;
                        bcnxe->lab = bcnxe->lba;
                        bcnxe->lba = lpix;
                    }
                }
                /* Mettre les parties connexes dans bord1 */
                bord1->canal += bord2->canal;
                for (bcnxe = bord1->cnxe; bcnxe->suiv != NULL;
                     bcnxe = bcnxe->suiv);
                bcnxe->suiv = bord2->cnxe;

                /* Eliminer bord2 de la liste de bords de reg */
                for (lbor3 = lbor1; lbor3->suiv->bord != bord2;
                     lbor3 = lbor3->suiv);
                lbor = lbor3->suiv;
                lbor3->suiv = lbor->suiv;

                /* Eliminer bord2 de la liste de bords de reg1=reg2 */
                lbor3 = reg1->Lbords;
                if (lbor3->bord == bord2)
                {
                    lbor = lbor3;
                    reg1->Lbords = lbor->suiv;
                }
                else
                {
                    for (; lbor3->suiv->bord != bord2; lbor3 = lbor3->suiv);
                    lbor = lbor3->suiv;
                    lbor3->suiv = lbor->suiv;
                }
                break;
            }
            else
                lbor2 = lbor2->suiv;
        }
        while (lbor2 != NULL);
        lbor1 = lbor1->suiv;
    }
}

static void Repointer(SOMMETPTR som, BORDPTR newbord, BORDPTR oldbord)
/* som va pointer sur newbord et plus sur */
/*  oldbord,si'newbord'est deja dans la  */
/*  liste som->Lbords on elimine'oldbord' */
{
    LI_BORDSPTR li;
    unsigned char existe_new;

    existe_new = FAUX;
    for (li = som->Lbords; li != NULL; li = li->suiv)
        existe_new |= (li->bord == newbord);
    if (existe_new == VRAI)
    {
        /* On elimine 'oldbord' de la liste 'som->Lbords' */
        li = som->Lbords;
        if (li->bord == oldbord)
            som->Lbords = li->suiv;
        else
        {
            for (li = som->Lbords;
                 (li->suiv != NULL) && (li->suiv->bord != oldbord);
                 li = li->suiv);
            if (li->suiv == NULL)
                return;         /*'newbord existe deja et 'oldbord' plus */
            li->suiv = li->suiv->suiv;
        }
    }
    else
    {                           /* On fait pointer 'som' sur 'newbord' */
        for (li = som->Lbords; li->bord != oldbord; li = li->suiv);
        li->bord = newbord;
    }
}

static void ElimBordeReg(REGIONPTR reg, BORDPTR bord)
/* Elimination de bord dans la */
/*  liste de bords de region   */
{
    LI_BORDSPTR li;

    li = reg->Lbords;
    if (li->bord == bord)
        reg->Lbords = li->suiv;
    else
    {
        for (; li->suiv->bord != bord; li = li->suiv);
        li->suiv = li->suiv->suiv;
    }
}

static LI_PIXELSPTR LiPixelsUnion(LI_PIXELSPTR liste1, LI_PIXELSPTR liste2)
/* Met la liste de pixels liste2 */
/*  derriere liste1              */
{
    LI_PIXELSPTR liste, lptr;

    for (liste = liste1; liste->suiv != NULL; liste = liste->suiv);
    if (liste->direction == liste2->direction)
    {
        liste->longueur += liste2->longueur;
        lptr = liste2;
        liste->suiv = lptr->suiv;
    }
    else
        liste->suiv = liste2;

    return (liste1);
}

/* 'segmentation' PROCEDURES */
static void segment(float level)
{
    unsigned char agglutination = VRAI;
    REGIONPTR regact, regvois;
    BORDPTR bordcom;
    LI_BORDSPTR libords;

    /* Boucle balayage de l'image jusqu'a obtention des composantes connexes */
    while (agglutination == VRAI)
    {
        agglutination = FAUX;
        for (regact = image.tete; regact != NULL; regact = regact->Rsuiv)
        {
            for (libords = regact->Lbords; libords != NULL;
                 libords = libords->suiv)
            {
                bordcom = libords->bord;
                regvois = (bordcom->rd == regact) ? bordcom->rg : bordcom->rd;
                if (*regact->canal == *regvois->canal)
                {
                    agglutination = VRAI;
                    RegMerge(regact, regvois, bordcom);
                    break;      /* we found a candidate region */
                }
            }
        }
    }
    printf(" Decomposition in connected components finished.\n");
    agglutination = VRAI;

    printf(" Extraction of contours of regions with gray <= %.4f.\n", level);
    while (agglutination == VRAI)
    {
        agglutination = FAUX;
        for (regact = image.tete; regact != NULL; regact = regact->Rsuiv)
        {
            for (libords = regact->Lbords; libords != NULL;
                 libords = libords->suiv)
            {
                bordcom = libords->bord;
                regvois = (bordcom->rd == regact) ? bordcom->rg : bordcom->rd;
                if (((*regact->canal <= level) && (*regvois->canal <= level))
                    || ((*regact->canal > level)
                        && (*regvois->canal > level)))
                {
                    agglutination = VRAI;
                    RegMerge(regact, regvois, bordcom);
                    break;      /* we found a candidate region */
                }
            }
        }
    }
}

/* This function puts the coordinates of the points from the segmentation    */
/* in 'point_curves'. We give the (integer) coordinates of the tips...       */
static void border2polygon(Fpolygons setpoly, int nb_poly, float level)
{
    float pX, pY, dX, dY;
    int i, nb_points, nb_angles;
    unsigned short len, poly_nb = 1;
    float *f;
    REGIONPTR rptr;
    BORDPTR bord;
    LI_BORDSPTR lbptr;
    LI_PIXELSPTR lipptr;
    Fpolygon poly;
    Point_fcurve newpc;
    char old_dir, first_dir;

    if ((setpoly->first =
         (Fpolygon) malloc(nb_poly * sizeof(struct fpolygon))) == NULL)
        mwerror(FATAL, 1, "Not enough memory.");
    if ((f = (float *) malloc(2 * nb_poly * sizeof(float))) == NULL)
        mwerror(FATAL, 1, "Not enough memory.");

    /* Initialize the 'nb_poly' polygons */
    for (i = 0, poly = setpoly->first; i < nb_poly; i++, poly++)
    {
        poly->nb_channels = 2;
        poly->channel = f++;
        f++;
        poly->first = NULL;
        poly->previous = (i == 0) ? NULL : (poly - 1);
        poly->next = (i == nb_poly - 1) ? NULL : (poly + 1);
    }

    poly = setpoly->first;
    for (rptr = image.tete; rptr != NULL; rptr = rptr->Rsuiv)
        if (*rptr->canal <= level)
        {
            for (lbptr = rptr->Lbords; lbptr != NULL; lbptr = lbptr->suiv)
            {
                bord = lbptr->bord;
                poly->channel[0] = level;
                /* here we give the levelset to the polygon */
                nb_points = (int) bord->canal;
                if ((poly->first = (Point_fcurve)
                     malloc(nb_points * sizeof(struct point_fcurve))) == NULL)
                    mwerror(FATAL, 1, "Not enough memory.");
                /* Initialize the 'nb_points' points */
                for (i = 0, newpc = poly->first; i < nb_points; i++, newpc++)
                {
                    newpc->previous = (i == 0) ? NULL : (newpc - 1);
                    newpc->next = (i == nb_points - 1) ? NULL : (newpc + 1);
                }
                if (bord->rg == rptr)
                {
                    pX = (float) bord->cnxe->sb->x - 0.5;
                    pY = (float) bord->cnxe->sb->y - 0.5;
                    lipptr = bord->cnxe->lba;
                }
                else
                {
                    pX = (float) bord->cnxe->sa->x - 0.5;
                    pY = (float) bord->cnxe->sa->y - 0.5;
                    lipptr = bord->cnxe->lab;
                }
                first_dir = lipptr->direction;
                nb_angles = 0;
                old_dir = '0';
                newpc = poly->first;
                for (; lipptr != NULL; lipptr = lipptr->suiv)
                {
                    newpc->x = pX;
                    newpc->y = pY;
                    newpc++;
                    switch (lipptr->direction)
                    {
                    case 'b':
                        {
                            dX = 0.0;
                            dY = 1.0;
                            if (old_dir == 'g')
                            {
                                nb_angles++;
                                break;
                            }
                            if (old_dir == 'd')
                            {
                                nb_angles--;
                                break;
                            }
                            break;
                        }
                    case 'd':
                        {
                            dX = 1.0;
                            dY = 0.0;
                            if (old_dir == 'b')
                            {
                                nb_angles++;
                                break;
                            }
                            if (old_dir == 'h')
                            {
                                nb_angles--;
                                break;
                            }
                            break;
                        }
                    case 'h':
                        {
                            dX = 0.0;
                            dY = -1.0;
                            if (old_dir == 'd')
                            {
                                nb_angles++;
                                break;
                            }
                            if (old_dir == 'g')
                            {
                                nb_angles--;
                                break;
                            }
                            break;
                        }
                    case 'g':
                        {
                            dX = -1.0;
                            dY = 0.0;
                            if (old_dir == 'h')
                            {
                                nb_angles++;
                                break;
                            }
                            if (old_dir == 'b')
                            {
                                nb_angles--;
                                break;
                            }
                            break;
                        }
                    }
                    len = lipptr->longueur;
                    while (--len > 0)
                    {
                        newpc->x = (pX += dX);
                        newpc->y = (pY += dY);
                        newpc++;
                    }
                    pX += dX;
                    pY += dY;
                    old_dir = lipptr->direction;
                }               /* end-for lipptr     */
                if (old_dir != first_dir)
                    switch (first_dir)
                    {
                    case 'b':
                        {
                            if (old_dir == 'g')
                            {
                                nb_angles++;
                                break;
                            }
                            if (old_dir == 'd')
                            {
                                nb_angles--;
                                break;
                            }
                            break;
                        }
                    case 'd':
                        {
                            if (old_dir == 'b')
                            {
                                nb_angles++;
                                break;
                            }
                            if (old_dir == 'h')
                            {
                                nb_angles--;
                                break;
                            }
                            break;
                        }
                    case 'h':
                        {
                            if (old_dir == 'd')
                            {
                                nb_angles++;
                                break;
                            }
                            if (old_dir == 'g')
                            {
                                nb_angles--;
                                break;
                            }
                            break;
                        }
                    case 'g':
                        {
                            if (old_dir == 'h')
                            {
                                nb_angles++;
                                break;
                            }
                            if (old_dir == 'b')
                            {
                                nb_angles--;
                                break;
                            }
                            break;
                        }
                    }
                assert((nb_angles == 4) || (nb_angles == -4));
                /* as we have only closed Jordan curves the sum of angles in */
                /* a polygon is always 2\pi=4*(\pi/2), the sign gives the    */
                /* orientation of the curve (+=outer, - inner boundary)      */
                poly->channel[1] =
                    (nb_angles == 4) ? (float) poly_nb : -(float) poly_nb;
                poly++;
                /* this fixes the label&orientation  of the current polygon */
            }                   /* end-for lbptr      */
            poly_nb++;
        }                       /* end loop levelset  */
}

static void BlackBound(Cimage whitesheet, float level)
{
    short a0, b0, a1, b1, dx, dy;
    REGIONPTR rptr;
    LI_BORDSPTR lbptr;
    BORDCONNEXEPTR cnxeptr;
    LI_PIXELSPTR lipptr;

    for (rptr = image.tete; rptr != NULL; rptr = rptr->Rsuiv)
        if (*rptr->canal <= level)
            for (lbptr = rptr->Lbords; lbptr != NULL; lbptr = lbptr->suiv)
                for (cnxeptr = lbptr->bord->cnxe; cnxeptr != NULL;
                     cnxeptr = cnxeptr->suiv)
                {
                    a0 = cnxeptr->sb->x;
                    b0 = cnxeptr->sb->y;
                    for (lipptr = cnxeptr->lba; lipptr != NULL;
                         lipptr = lipptr->suiv)
                    {
                        dx = dy = 0;
                        switch (lipptr->direction)
                        {
                        case 'b':
                            dy = lipptr->longueur;
                            break;
                        case 'd':
                            dx = lipptr->longueur;
                            break;
                        case 'h':
                            dy = -lipptr->longueur;
                            break;
                        case 'g':
                            dx = -lipptr->longueur;
                            break;
                        }
                        a1 = a0 + dx;
                        b1 = b0 + dy;
                        TraitHVmono(whitesheet, a0, b0, a1, b1, 0);
                        a0 = a1;
                        b0 = b1;
                    }
                }
}

static short TraitHVmono(Cimage whitesheet, short int a0, short int b0,
                         short int a1, short int b1, unsigned char c)
{
    short bdx = whitesheet->ncol, bdy = whitesheet->nrow;
    short z = 0, dz, sz;

    if ((a0 != a1) && (b0 != b1))
        return (-2);
    if (b0 == b1)
    {
        if (a0 < 0)
            a0 = 0;
        else if (a0 >= bdx)
            a0 = bdx - 1;
        if (a1 < 0)
        {
            a1 = 0;
            if (a0 == 0)
                return (-1);
        }
        else if (a1 >= bdx)
        {
            a1 = bdx - 1;
            if (a0 == bdx - 1)
                return (-1);
        }
        if (a0 < a1)
        {
            sz = 1;
            dz = a1 - a0;
        }
        else
        {
            sz = -1;
            dz = a0 - a1;
        }
        while (abs(z) <= dz)
        {
            whitesheet->gray[((long) bdx) * b0 + z + a0] = c;
            z += sz;
        }
    }
    else
    {
        if (b0 < 0)
            b0 = 0;
        else if (b0 >= bdy)
            b0 = bdy - 1;
        if (b1 < 0)
        {
            b1 = 0;
            if (b0 == 0)
                return (-1);
        }
        else if (b1 >= bdy)
        {
            b1 = bdy - 1;
            if (b0 == bdy - 1)
                return (-1);
        }
        if (b0 < b1)
        {
            sz = 1;
            dz = b1 - b0;
        }
        else
        {
            sz = -1;
            dz = b0 - b1;
        }
        while (abs(z) <= dz)
        {
            whitesheet->gray[((long) bdx) * (z + b0) + a0] = c;
            z += sz;
        }
    }
    return (0);
}

static void cDess_u_LS(Cimage boundary, Cimage u, float level, char inside,
                       char outside)
/* Balayage de l'image gauche/droite et haut/bas   */
/* Determine s'il ya un bord ou non                */
/* Si oui on change de region, dans les deux cas   */
/* on dessine en utilisant la fonction correspondant */
/* a la region actuelle.   */
{
    REGIONPTR regact, regtop;
    short i, j, nrows = boundary->nrow, ncols = boundary->ncol;

    regtop = image.regpixel0_0;
    for (i = 0; i < ncols; i++)
    {
        regact = regtop;
        if ((i != ncols - 1) && (boundary->gray[i + 1] == 0))
            regtop = RegAdjD(regact, i + 1, 0);
        /* Fin image ou bord vertical 1ere ligne */
        u->gray[i] = (*regact->canal <= level) ? inside : outside;
        /* Dessine le premier pixel de la i-ieme colonne   */
        for (j = 1; j < nrows; j++)
        {
            /* Dessine les pixels restants de la i-ieme colonne */
            if (boundary->gray[ncols * j + i] == 0)
                regact = RegAdjB(regact, i, j);
            u->gray[ncols * j + i] =
                (*regact->canal <= level) ? inside : outside;
        }
    }
}

static void fDess_u_LS(Cimage boundary, Fimage u)
/* Balayage de l'image gauche/droite et haut/bas   */
/* Determine s'il ya un bord ou non                */
/* Si oui on change de region, dans les deux cas   */
/* on dessine en utilisant la fonction correspondant */
/* a la region actuelle.   */
{
    REGIONPTR regact, regtop;
    short i, j, nrows = boundary->nrow, ncols = boundary->ncol;

    regtop = image.regpixel0_0;
    for (i = 0; i < ncols - 1; i++)
    {
        regact = regtop;
        if ((i != ncols - 1) && (boundary->gray[i + 1] == 0))
            regtop = RegAdjD(regact, i + 1, 0);
        /* Fin image ou bord vertical 1ere ligne */
        u->gray[i] = *regact->canal;
        /* Dessine le premier pixel de la i-ieme colonne   */
        for (j = 1; j < nrows - 1; j++)
        {
            /* Dessine les pixels restants de la i-ieme colonne */
            if (boundary->gray[ncols * j + i] == 0)
                regact = RegAdjB(regact, i, j);
            u->gray[ncols * j + i] = *regact->canal;
        }
    }
}

static REGIONPTR RegAdjD(REGIONPTR reg, short int i, short int j)
{
    LI_BORDSPTR lbords;
    BORDPTR bordptr;
    BORDCONNEXEPTR cnxeptr;
    LI_PIXELSPTR lipptr;
    short a0, b0, a1, b1, dx, dy;

    for (lbords = reg->Lbords; lbords != NULL; lbords = lbords->suiv)
    {
        bordptr = lbords->bord;
        cnxeptr = bordptr->cnxe;
        while (cnxeptr != NULL)
        {
            a0 = cnxeptr->sb->x;
            b0 = cnxeptr->sb->y;
            for (lipptr = cnxeptr->lba; lipptr != NULL; lipptr = lipptr->suiv)
            {
                dx = dy = 0;
                switch (lipptr->direction)
                {
                case 'b':
                    dy = lipptr->longueur;
                    break;
                case 'd':
                    dx = lipptr->longueur;
                    break;
                case 'h':
                    dy = -lipptr->longueur;
                    break;
                case 'g':
                    dx = -lipptr->longueur;
                    break;
                }
                a1 = a0 + dx;
                b1 = b0 + dy;
                /* conditions pour que (i,j) soit un pixel de bord vertical */
                if ((a0 == a1) && (i == a0)
                    && (((j >= b1) && (j < b0)) || ((j >= b0) && (j < b1))))
                    return ((reg == bordptr->rg) ? bordptr->rd : bordptr->rg);
                a0 = a1;
                b0 = b1;
            }
            cnxeptr = cnxeptr->suiv;
        }
    }
    return (reg);
}

static REGIONPTR RegAdjB(REGIONPTR reg, short int i, short int j)
{
    LI_BORDSPTR lbords;
    BORDPTR bordptr;
    BORDCONNEXEPTR cnxeptr;
    LI_PIXELSPTR lipptr;
    short a0, b0, a1, b1, dx, dy;

    for (lbords = reg->Lbords; lbords != NULL; lbords = lbords->suiv)
    {
        bordptr = lbords->bord;
        cnxeptr = bordptr->cnxe;
        while (cnxeptr != NULL)
        {
            a0 = cnxeptr->sb->x;
            b0 = cnxeptr->sb->y;
            for (lipptr = cnxeptr->lba; lipptr != NULL; lipptr = lipptr->suiv)
            {
                dx = dy = 0;
                switch (lipptr->direction)
                {
                case 'b':
                    dy = lipptr->longueur;
                    break;
                case 'd':
                    dx = lipptr->longueur;
                    break;
                case 'h':
                    dy = -lipptr->longueur;
                    break;
                case 'g':
                    dx = -lipptr->longueur;
                    break;
                }
                a1 = a0 + dx;
                b1 = b0 + dy;
                /* conditions pour que (i,j) soit un pixel de bord horizontal */
                if ((b0 == b1) && (j == b0)
                    && (((i >= a1) && (i < a0)) || ((i >= a0) && (i < a1))))
                    return ((reg == bordptr->rg) ? bordptr->rd : bordptr->rg);
                a0 = a1;
                b0 = b1;
            }
            cnxeptr = cnxeptr->suiv;
        }
    }
    return (reg);
}

void
one_levelset(float *level, Cimage cb, Fpolygons pb, Fimage fu, Cimage cu,
             Fimage image_org)
{
    Cimage boundary = NULL;
    long l;
    REGIONPTR rptr;
    LI_BORDSPTR lbptr;
    int number_lreg = 0, number_points = 0, number_cbords = 0;

    if ((!cb) && (!pb) && (!cu) && (!fu))
        mwerror(USAGE, 1, "Select at least one output option !\n");

    image.gx = image_org->ncol;
    image.gy = image_org->nrow;
    Estimation();
    Initialisation(image_org);

    segment(*level);

    for (rptr = image.tete; rptr != NULL; rptr = rptr->Rsuiv)
        if (*rptr->canal <= *level)
            number_lreg++;

    printf("\n Found %d connected regions for the levelset.", number_lreg);

    if ((cb) || (cu) || (fu))
    {
        if ((boundary =
             mw_change_cimage(NULL, image_org->ncol,
                              image_org->nrow)) == NULL)
            mwerror(FATAL, 1, "Not enough memory.");
        for (l = 0; l < image_org->nrow * image_org->ncol; l++)
            boundary->gray[l] = 255;
        BlackBound(boundary, *level);
    }

    if (cu)
    {
        printf("\n Constructing b/w levelset picture.");
        cu = mw_change_cimage(cu, image_org->ncol, image_org->nrow);
        cDess_u_LS(boundary, cu, *level, (char) 0, (char) 255);
    }

    if (fu)
    {
        printf("\n Constructing gray (%.1f/%.1f) levelset picture.",
               MIN(*image.tete->Lbords->bord->rg->canal,
                   *image.tete->Lbords->bord->rd->canal),
               MAX(*image.tete->Lbords->bord->rg->canal,
                   *image.tete->Lbords->bord->rd->canal));
        fu = mw_change_fimage(fu, image_org->ncol, image_org->nrow);
        fDess_u_LS(boundary, fu);
    }

    if (cb)
        *cb = *boundary;        /* either spitt it out */
    else if ((cu) || (fu))
        mw_delete_cimage(boundary);     /* else to the trash   */

    if (pb)
    {
        /* Check that all regions have just connected, closed boundaries */
        /* else we will immediately exit, if everything is fine then     */
        /* count the total space needed in order to allocate memory (??) */
        /* Just consider the regions \leq than the current levellines    */
        for (rptr = image.tete; rptr != NULL; rptr = rptr->Rsuiv)
            if (*rptr->canal <= *level)
                for (lbptr = rptr->Lbords; lbptr != NULL; lbptr = lbptr->suiv)
                {
                    if (lbptr->bord->cnxe->suiv != NULL)
                    {
                        fprintf(stderr,
                                "\n Complex image structure... "
                                "conversion to polygons not implemented\n");
                        Image_free(&image);
                        return;
                    }
                    number_points += lbptr->bord->canal;
                    number_cbords++;
                }
        /* Now at least we are tranquil (hopefully!)                     */

        printf("\n Writting %d contours, with %d points, "
               "in MW2-polygons structure.", number_cbords, number_points);
        pb = mw_change_fpolygons(pb);
        border2polygon(pb, number_cbords, *level);
    }
    Image_free(&image);
    printf("\n");
}
