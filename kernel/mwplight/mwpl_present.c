/*~~~~~~~~~~~~~~ mwplight - The MegaWave2 Light Preprocessor ~~~~~~~~~~~~~~~~~~~

 Generate the module's presentation

 Author : Jacques Froment
 Date : 2005
 Version : 0.1
 Versions history :
  Original version from mwp (the 'traditional' MegaWave2 preprocessor),
                   itself coming from MegaWave1.
  0.1 (August 2005, JF) initial internal release
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~  This file is part of the MegaWave2 light preprocessor ~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <time.h>

#include "mwpl_main.h"

/* Maximum length of a line */
#define LINE_LEN  79 

#ifdef __STDC__
void xprint(char c, short n)
#else
void xprint(c, n)
char c; 
short n;
#endif
{
  short i;
  for (i=1;i<=n;i++)
    putc(c,fa);
}


#ifdef __STDC__
void cutstring(char *Ch1, char *Ch2, char *Ch, short l)
#else
void cutstring(Ch1, Ch2, Ch, l)
char *Ch1,*Ch2,*Ch;
short l;
#endif
{
  short lC,i;

  lC = strlen(Ch);
  if (lC <= l) {
    strcpy(Ch1, Ch);
    Ch2[0]='\0';
    return;
  }
  for (i=l;(i>0) && (Ch[i] != ' ');i--) ;
  if (i == 0) Error("[cutstring] unexpected error with the header");
  strncpy(Ch1, Ch,i);
  Ch1[i] = '\0';
  Ch = (char *) &Ch[i+1];
  strncpy(Ch2, Ch,l);
  Ch2[l] = '\0';
}


#ifdef __STDC__
int module_presentation(char *Prog, char *Vers, char *Auth,
                            char *Func, char *Lab)
#else
int module_presentation(Prog, Vers, Auth, Func, Lab)
char *Prog, *Vers, *Auth, *Func, *Lab;
#endif
{ 
  short lP, lu, lV, dV;
  char *Ch, *Ch1, *Ch2, *Ch3, *Ch4;
  time_t tloc;
  int yearXX,yearXXXX;

  lP = strlen(Prog); 
  if (lP < 2)
    return(-1);
  lu = LINE_LEN - lP - 6;    /* Longueur utile du texte */
  Ch1 = (char *) calloc(lu+1, sizeof(char *));
  Ch2 = (char *) calloc(lu+1, sizeof(char *));
  Ch3 = (char *) calloc(lu+1, sizeof(char *));
  Ch4 = (char *) calloc(lu+1, sizeof(char *));
  Ch = (char *) calloc(255, sizeof(char *));
  if ((Ch1 == NULL) || (Ch2 == NULL) || (Ch3 == NULL))
    Error("[module_presentation] Not enough memory");

  /* Annee en cours */
  (void)time(&tloc);
  yearXX = localtime(&tloc)->tm_year;
  /* Au cas ou tm_year soit vraiment le nbre d'annees depuis 1900 */
  if (yearXX >= 100) yearXX -= 100;   
  if (yearXX < 98) yearXXXX = 2000 + yearXX;  /* Ecrit en 1998 ! */
  else yearXXXX = 1900+ yearXX;   

  /* Calcul des chaines */
  strcpy(Ch, "Author(s) : ");

  if (Auth != NULL && *Auth != '\0') {
    sprintf(Ch1, "%s. ", Auth);
    strcat(Ch, Ch1);
  }
  if (Lab != NULL && *Lab != '\0') {
    sprintf(Ch2, "%s.", Lab);
    strcat(Ch, Ch2);
  }
  cutstring(Ch1, Ch2, Func, lu-1);
  cutstring(Ch3, Ch4, Ch, lu);

  /* ligne 1 */
  fprintf(fa, "  fprintf(stderr, \"");
  xprint('-', LINE_LEN);
  fprintf(fa, "\\n\");\n");

  /* ligne 2 */
  fprintf(fa, "  fprintf(stderr, \"");
  xprint('\\', 4);
  xprint(' ', lP);
  fprintf(fa, "//  ");  
  if (Ch1[0] != '\0')
    fprintf(fa, "%s", Ch1);
  if (Ch2[0] == '\0')
    putc('.', fa);
  fprintf(fa, "\\n\");\n");

  /* ligne 3 */
  fprintf(fa, "  fprintf(stderr, \"");
  fprintf(fa, " \\\\\\\\");
  xprint(' ', lP-2);
  fprintf(fa, "//   ");  
  if (Ch2[0] != '\0')
    fprintf(fa, "%s.", Ch2);
  fprintf(fa, "\\n\");\n");

  /* ligne 4 */
  fprintf(fa, "  fprintf(stderr, \"");
  fprintf(fa, "  %s    ", Prog);
  if ((Auth != NULL && *Auth != '\0') || (Lab != NULL && *Lab != '\0'))
    fprintf(fa, "%s\\n", Ch3);
  else
    Ch4[0]='\0';
  fprintf(fa, "\");\n");

  /* ligne 5 */
  fprintf(fa, "  fprintf(stderr, \"");
  sprintf(Ch, "MegaWave2 : (C)1998-%d CMLA, ENS Cachan, 94235 Cachan cedex, France.\\n",yearXXXX);
  if (strlen(Ch) > lu)
  sprintf(Ch, "MegaWave2 : (C)1998-%d CMLA, ENS Cachan, 94235 Cachan cedex.\\n",yearXXXX);
  fprintf(fa, " //");
  xprint(' ', lP-2);
  fprintf(fa, "\\\\\\\\   ");
  if (Ch4[0] == '\0')
    fprintf(fa, Ch);
  else
    fprintf(fa, "%s\\n",Ch4);
  fprintf(fa, "\");\n");

  /* ligne 6 */
  fprintf(fa, "  fprintf(stderr, \"");
  lV = strlen(Vers);
  dV = (lP - lV - 1) / 2;
  fprintf(fa, "//");
  if ((lV > 0) && (lV <= lP-1)) {
    xprint(' ', dV); 
    if ((2*dV < lP - lV - 1) && (lV < lP-1)) {
      fprintf(fa, "V %s", Vers);
      dV++;
    }
    else
      fprintf(fa, "V%s", Vers); 
    xprint(' ', lP-dV-lV-1);
  }
  else
    xprint(' ', lP);
  fprintf(fa, "\\\\\\\\  ");
  if (Ch4[0] == '\0') 
    {
      sprintf(Ch, "Last version available at http://www.cmla.ens-cachan.fr/Cmla/Megawave\\n");
      if (strlen(Ch) > lu)
	sprintf(Ch, "Last version at http://www.cmla.ens-cachan.fr/Cmla/Megawave\\n");
      fprintf(fa, Ch);
    }
  else
    {
      sprintf(Ch, "MegaWave2 : (C)1998-%d CMLA, ENS Cachan, 94235 Cachan cedex. http://www.cmla.ens-cachan.fr/Cmla/Megawave\\n",yearXXXX);
      if (strlen(Ch) > lu)
	sprintf(Ch, "MegaWave2 : (C)1998-%d CMLA, ENS Cachan. http://www.cmla.ens-cachan.fr/Cmla/Megawave\\n",yearXXXX);
      fprintf(fa, Ch);
    }
  fprintf(fa, "\");\n");

  /* Dernieres lignes */
  fprintf(fa, "  fprintf(stderr, \"");
  xprint('-', LINE_LEN);
  fprintf(fa, "\\n\\n");
  fprintf(fa, "\");\n");
}    
