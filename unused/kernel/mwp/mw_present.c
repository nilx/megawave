/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* Module's presentation
   V 1.1

   Main changes
   V 1.1 (JF, 23/02/2006) added include <string.h>
*/

#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

static short _LONG_LIGNE = 79;  /* Longeur maxi d'une ligne */

#ifdef __STDC__
void _xprint(FILE *fd, char c, short n)
#else
_xprint(fd, c, n)
FILE *fd;
char c; 
short n;
#endif
{
  short i;
  for (i=1;i<=n;i++)
    putc(c,fd);
}


#ifdef __STDC__
void _coupe_chaine(char *Ch1, char *Ch2, char *Ch, short l)
#else
_coupe_chaine(Ch1, Ch2, Ch, l)
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
  if (i == 0)
    error("Bad initialization of the command header");
  strncpy(Ch1, Ch,i);
  Ch1[i] = '\0';
  Ch = (char *) &Ch[i+1];
  strncpy(Ch2, Ch,l);
  Ch2[l] = '\0';
}


#ifdef __STDC__
int _mw_presentation(FILE *fd, char *_Prog, char *_Vers, char *_Auth,
                            char *_Fonc, char *_Labo)
#else
_mw_presentation(fd, _Prog, _Vers, _Auth, _Fonc, _Labo)
FILE *fd;
char *_Prog, *_Vers, *_Auth, *_Fonc, *_Labo;
#endif
{ 
  short lP, lu, lV, dV;
  char *Ch, *Ch1, *Ch2, *Ch3, *Ch4;
  time_t tloc;
  int yearXX,yearXXXX;

  lP = strlen(_Prog); 
  if (lP < 2)
    return(-1);
  lu = _LONG_LIGNE - lP - 6;    /* Longueur utile du texte */
  Ch1 = (char *) calloc(lu+1, sizeof(char *));
  Ch2 = (char *) calloc(lu+1, sizeof(char *));
  Ch3 = (char *) calloc(lu+1, sizeof(char *));
  Ch4 = (char *) calloc(lu+1, sizeof(char *));
  Ch = (char *) calloc(255, sizeof(char *));
  if ((Ch1 == NULL) || (Ch2 == NULL) || (Ch3 == NULL))
      error("Not enough memory for _mw_presentation()");

  /* Annee en cours */
  (void)time(&tloc);
  yearXX = localtime(&tloc)->tm_year;
  /* Au cas ou tm_year soit vraiment le nbre d'annees depuis 1900 */
  if (yearXX >= 100) yearXX -= 100;   
  if (yearXX < 98) yearXXXX = 2000 + yearXX;  /* Ecrit en 1998 ! */
  else yearXXXX = 1900+ yearXX;   

  /* Calcul des chaines */
  strcpy(Ch, "Copyright ");
  sprintf(Ch1, "(C)%d ", yearXXXX);
  strcat(Ch, Ch1);

  if (_Auth != NULL && *_Auth != '\0') {
    sprintf(Ch2, "%s. ", _Auth);
    strcat(Ch, Ch2);
  }
  if (_Labo != NULL && *_Labo != '\0') {
    sprintf(Ch3, "%s.", _Labo);
    strcat(Ch, Ch3);
  }
  _coupe_chaine(Ch1, Ch2, _Fonc, lu-1);
  _coupe_chaine(Ch3, Ch4, Ch, lu);

  /* ligne 1 */
  fprintf(fd, "  fprintf(stderr, \"");
  _xprint(fd, '-', _LONG_LIGNE);
  fprintf(fd, "\\n\");\n");

  /* ligne 2 */
  fprintf(fd, "  fprintf(stderr, \"");
  _xprint(fd, '\\', 4);
  _xprint(fd, ' ', lP);
  fprintf(fd, "//  ");  
  if (Ch1[0] != '\0')
    fprintf(fd, "%s", Ch1);
  if (Ch2[0] == '\0')
    putc('.', fd);
  fprintf(fd, "\\n\");\n");

  /* ligne 3 */
  fprintf(fd, "  fprintf(stderr, \"");
  fprintf(fd, " \\\\\\\\");
  _xprint(fd, ' ', lP-2);
  fprintf(fd, "//   ");  
  if (Ch2[0] != '\0')
    fprintf(fd, "%s.", Ch2);
  fprintf(fd, "\\n\");\n");

  /* ligne 4 */
  fprintf(fd, "  fprintf(stderr, \"");
  fprintf(fd, "  %s    ", _Prog);
  if ((_Auth != NULL && *_Auth != '\0') || (_Labo != NULL && *_Labo != '\0'))
    fprintf(fd, "%s\\n", Ch3);
  else
    Ch4[0]='\0';
  fprintf(fd, "\");\n");

  /* ligne 5 */
  fprintf(fd, "  fprintf(stderr, \"");
  sprintf(Ch, "MegaWave2 : J.Froment (C)1988-98 CEREMADE, Univ. Paris-Dauphine\\n");
  if (strlen(Ch) > lu)
  sprintf(Ch, "MegaWave2 : J.Froment (C)1988-98 CEREMADE, Univ. Paris 9\\n");
  fprintf(fd, " //");
  _xprint(fd, ' ', lP-2);
  fprintf(fd, "\\\\\\\\   ");
  if (Ch4[0] == '\0')
    fprintf(fd, Ch);
  else
    fprintf(fd, "%s\\n",Ch4);
  fprintf(fd, "\");\n");

  /* ligne 6 */
  fprintf(fd, "  fprintf(stderr, \"");
  lV = strlen(_Vers);
  dV = (lP - lV - 1) / 2;
  fprintf(fd, "//");
  if ((lV > 0) && (lV <= lP-1)) {
    _xprint(fd, ' ', dV); 
    if ((2*dV < lP - lV - 1) && (lV < lP-1)) {
      fprintf(fd, "V %s", _Vers);
      dV++;
    }
    else
      fprintf(fd, "V%s", _Vers); 
    _xprint(fd, ' ', lP-dV-lV-1);
  }
  else
    _xprint(fd, ' ', lP);
  fprintf(fd, "\\\\\\\\  ");
  if (Ch4[0] == '\0') 
    {
      sprintf(Ch, "and (C)1998-%d CMLA, ENS Cachan, 94235 Cachan cedex, France.\\n",yearXXXX);
      if (strlen(Ch) > lu)
	sprintf(Ch, "and (C)1998-%d CMLA, ENS Cachan, 94235 Cachan cedex.\\n",yearXXXX);
      fprintf(fd, Ch);
    }
  else
    {
      sprintf(Ch, "MegaWave2 : J.Froment (C)1988-98 Univ. Paris 9 & (C)1998-%d CMLA, ENS Cachan, 94235 Cachan cedex.\\n",yearXXXX);
      if (strlen(Ch) > lu)
      sprintf(Ch, "MegaWave2 : J.Froment (C)1988-98 Univ. Paris 9 & (C)1998-%d CMLA, ENS Cachan.\\n",yearXXXX);
      fprintf(fd, Ch);
    }
  fprintf(fd, "\");\n");

  /* Dernieres lignes */
  fprintf(fd, "  fprintf(stderr, \"");
  _xprint(fd, '-', _LONG_LIGNE);
  fprintf(fd, "\\n\\n");
  fprintf(fd, "\");\n");
}    
