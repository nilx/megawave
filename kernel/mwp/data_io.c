/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/* Fichiers d'include */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bintree.h"
#include "symbol.h"
#include "token.h"
#include "io.h"
#include "y.tab.h"
#include "mwarg.h"
#define DATA_IO_DEC
#include "data_io.h"

#define COMMENT_CHAR '#'

static DataIo defio[BUFSIZ] = {
  {QSTRING_T, NULL, READ, "_mw_atoq_",  NULL},
  {CHAR_T,    NULL, READ, "_mw_atoc_",  NULL},
  {UCHAR_T,   NULL, READ, "_mw_atouc_", NULL},
  {SHORT_T,   NULL, READ, "_mw_atos_",  NULL},
  {USHORT_T,  NULL, READ, "_mw_atous_", NULL},
  {INT_T,     NULL, READ, "_mw_atoi_",  NULL},
  {UINT_T,    NULL, READ, "_mw_atoui_", NULL},
  {LONG_T,    NULL, READ, "_mw_atol_",  NULL},
  {ULONG_T,   NULL, READ, "_mw_atoul_", NULL},
  {FLOAT_T,   NULL, READ, "_mw_atof_",  NULL},
  {DOUBLE_T,  NULL, READ, "_mw_atod_",  NULL},
  {QSTRING_T, NULL, WRITE, "_mw_qtoa_",  NULL},
  {CHAR_T,    NULL, WRITE, "_mw_ctoa_",  NULL},
  {UCHAR_T,   NULL, WRITE, "_mw_uctoa_", NULL},
  {SHORT_T,   NULL, WRITE, "_mw_stoa_",  NULL},
  {USHORT_T,  NULL, WRITE, "_mw_ustoa_", NULL},
  {INT_T,     NULL, WRITE, "_mw_itoa_",  NULL},
  {UINT_T,    NULL, WRITE, "_mw_uitoa_", NULL},
  {LONG_T,    NULL, WRITE, "_mw_ltoa_",  NULL},
  {ULONG_T,   NULL, WRITE, "_mw_ultoa_", NULL},
  {FLOAT_T,   NULL, WRITE, "_mw_ftoa_",  NULL},
  {DOUBLE_T,  NULL, WRITE, "_mw_dtoa_",  NULL},
  {NONE_T,    NULL, READ, NULL,      NULL}
};

  

#ifdef __STDC__
void setdefio(Header *h)
#else
setdefio(h)
Header *h;
#endif
{
  int i;
  for (i=0; _GET_IO_T(&defio[i]) != NONE_T; i++) {
    Cell *pc;
    DataIo *d;
    if ((d = (DataIo *)malloc(sizeof(DataIo))) != NULL) {
      if ((pc = newcell(d)) != NULL) {
        /* C'est un type predefini du langage C */
        SET_IO_T(pc, _GET_IO_T(&defio[i]));

        /* Type reconnu */
        SET_IO_TYPE(pc, _GET_IO_TYPE(&defio[i]));

        /* Sens des transfert des donnees */
        SET_RW(pc, _GET_RW(&defio[i]));

        /* Fonction associee */
        SET_FUNCTION(pc, _GET_FUNCTION(&defio[i]));

        /* Fichier d'include associee */
        SET_INCLUDE(pc, _GET_INCLUDE(&defio[i]));

        if (GET_NUMBER(h) == 0)
          (void) addcell(h, NULL, pc);
        else
          (void) addcell(h, GET_LAST(h), pc);
      }
    }
    else
      INT_ERROR("setdefio : cannot allocate memory");
  }
}

/*
Lit le fichier decrivant des foncions d'entrees/sorties et
place les differentes informations dans la liste passee en
argument
ENTREES	FILE *fd  : Descritpteur du fichier
SORTIES	Header *h : liste devant contenir les descriptifs
*/

#ifdef __STDC__
void read_data_io(FILE *fd, char *filename, Header *h)
#else
void read_data_io(fd, filename, h)
FILE *fd;
char *filename;
Header *h;
#endif
{
  int lineno;
#ifdef DEBUG
  Cell *c;
#endif

#ifdef DEBUG
  PRDBG("read_data_io(0x%ul, %s, 0x%ul)\n", (unsigned long)fd, filename, (unsigned long)h);
#endif
  setdefio(h);

  for (lineno = 1; !feof(fd); lineno++) {
    char *pbbeg, *pbend, buffer[BUFSIZ];
    char type[BUFSIZ], rw[BUFSIZ], function[BUFSIZ], include[BUFSIZ];

    buffer[0] = '\0';
    
    (void) fgets(buffer, BUFSIZ-1, fd);

    pbbeg = &buffer[0];

    /* On elimine les "blancs" de debut de ligne */
    for ( ;*pbbeg ==' ' || *pbbeg == '\t'; pbbeg++);

    /* On elimine les commentaire de fin de ligne s'ils existent */
    if ((pbend = (char *) strchr(pbbeg, COMMENT_CHAR)) != NULL) {
      *pbend = '\0';
    }

    /* On elimine les "blancs" de fin de ligne */
    for (pbend=pbbeg+strlen(pbbeg)-1;
         *pbend==' ' || *pbend=='\t' || *pbend=='\n';
         pbend--);
    *++pbend = '\0';

    /* Si la ligne ne contient que des "blancs" on continue */
    if (*pbbeg == '\0')
      continue;

    /* Remise a zero des buffers de champs */
    type[0] = '\0';
    rw[0] = '\0';
    function[0] = '\0';
    include[0] = '\0';

    /* On lit les quatre champs */
    sscanf(pbbeg, "%s %s %s %s", type, rw, function, include);

    /* Si un des champs est vide , il y a une erreur */
    if (strlen(type) == 0 || strlen(rw) == 0 || strlen(function) == 0 ||
        strlen(include) == 0)
      warning2(filename, lineno, "Syntax error\n");
    else {
      Cell *pc;
      DataIo *d;
      if ((d = (DataIo *)malloc(sizeof(DataIo))) != NULL) {
        if ((pc = newcell(d)) != NULL) {
          /* Type reconnu */
          if (SET_IO_TYPE(pc, (char *)malloc(strlen(type)+1)) != NULL)
            strcpy(GET_IO_TYPE(pc), type);
          else
            INT_ERROR("read_data_io : cannot allocate memory");

          /* C'est un type predefini MegaWave2  */
          SET_IO_T(pc, MW2_T);

          /* Sens des transfert des donnees */
          if (!strcmp(rw, "read"))
            SET_RW(pc, READ);
          else if (!strcmp(rw, "write"))
            SET_RW(pc, WRITE);
          else {
            warning2(filename, lineno, "'%s' : unknown io\n", rw);
            continue;
          }

          /* Fonction associee */
          if (SET_FUNCTION(pc, (char *)malloc(strlen(function)+1))!=NULL)
            strcpy(GET_FUNCTION(pc), function);
          else
            INT_ERROR("read_data_io : cannot allocate memory");

          /* Fichier d'include associee */
          if (SET_INCLUDE(pc, (char *)malloc(strlen(include)+1)) != NULL)
            strcpy(GET_INCLUDE(pc), include);
          else
            INT_ERROR("read_data_io : cannot allocate memory");

          if (GET_NUMBER(h) == 0)
            (void) addcell(h, NULL, pc);
          else
            (void) addcell(h, GET_LAST(h), pc);
        }
        else
          INT_ERROR("read_data_io : cannot allocate memory");
      }
      else
        INT_ERROR("read_data_io : cannot allocate memory");
    }
  }
#ifdef DEBUG
  PRDBG("read_data_io : data_io_list = {\n");
  for (c = GET_FIRST(&data_io_list); c != NULL; c = GET_NEXT(c))
    switch(GET_IO_T(c)) {
      case MW2_T:
  PRDBG("read_data_io :   {MW2_T, %s, %s, %s, %s},\n",
               GET_IO_TYPE(c),
               GET_RW(c)==READ?"READ":(GET_RW(c)==WRITE?"WRITE":"UNKNOWN"),
               GET_FUNCTION(c),
               GET_INCLUDE(c));
        break;
      case QSTRING_T:
  PRDBG("read_data_io :   {QSTRING_T, NULL, %s, %s, %s},\n",
               GET_RW(c)==READ?"READ":(GET_RW(c)==WRITE?"WRITE":"UNKNOWN"),
               GET_FUNCTION(c),
               GET_INCLUDE(c));
        break;
      case CHAR_T:
  PRDBG("read_data_io :   {CHAR_T, NULL, %s, %s, %s},\n",
               GET_RW(c)==READ?"READ":(GET_RW(c)==WRITE?"WRITE":"UNKNOWN"),
               GET_FUNCTION(c),
               GET_INCLUDE(c));
        break;
      case UCHAR_T:
  PRDBG("read_data_io :   {UCHAR_T, NULL, %s, %s, %s},\n",
               GET_RW(c)==READ?"READ":(GET_RW(c)==WRITE?"WRITE":"UNKNOWN"),
               GET_FUNCTION(c),
               GET_INCLUDE(c));
        break;
      case SHORT_T:
  PRDBG("read_data_io :   {SHORT_T, NULL, %s, %s, %s},\n",
               GET_RW(c)==READ?"READ":(GET_RW(c)==WRITE?"WRITE":"UNKNOWN"),
               GET_FUNCTION(c),
               GET_INCLUDE(c));
        break;
      case USHORT_T:
  PRDBG("read_data_io :   {USHORT_T, NULL, %s, %s, %s},\n",
               GET_RW(c)==READ?"READ":(GET_RW(c)==WRITE?"WRITE":"UNKNOWN"),
               GET_FUNCTION(c),
               GET_INCLUDE(c));
        break;
      case INT_T:
  PRDBG("read_data_io :   {INT_T, NULL, %s, %s, %s},\n",
               GET_RW(c)==READ?"READ":(GET_RW(c)==WRITE?"WRITE":"UNKNOWN"),
               GET_FUNCTION(c),
               GET_INCLUDE(c));
        break;
      case UINT_T:
  PRDBG("read_data_io :   {UINT_T, NULL, %s, %s, %s},\n",
               GET_RW(c)==READ?"READ":(GET_RW(c)==WRITE?"WRITE":"UNKNOWN"),
               GET_FUNCTION(c),
               GET_INCLUDE(c));
        break;
      case LONG_T:
  PRDBG("read_data_io :   {LONG_T, NULL, %s, %s, %s},\n",
               GET_RW(c)==READ?"READ":(GET_RW(c)==WRITE?"WRITE":"UNKNOWN"),
               GET_FUNCTION(c),
               GET_INCLUDE(c));
        break;
      case ULONG_T:
  PRDBG("read_data_io :   {ULONG_T, NULL, %s, %s, %s},\n",
               GET_RW(c)==READ?"READ":(GET_RW(c)==WRITE?"WRITE":"UNKNOWN"),
               GET_FUNCTION(c),
               GET_INCLUDE(c));
        break;
      case FLOAT_T:
  PRDBG("read_data_io :   {FLOAT_T, NULL, %s, %s, %s},\n",
               GET_RW(c)==READ?"READ":(GET_RW(c)==WRITE?"WRITE":"UNKNOWN"),
               GET_FUNCTION(c),
               GET_INCLUDE(c));
        break;
      case DOUBLE_T:
  PRDBG("read_data_io :   {DOUBLE_T, NULL, %s, %s, %s},\n",
               GET_RW(c)==READ?"READ":(GET_RW(c)==WRITE?"WRITE":"UNKNOWN"),
               GET_FUNCTION(c),
               GET_INCLUDE(c));
        break;

      case NONE_T:
  PRDBG("read_data_io :   {NONE_T}\n");
  PRDBG("read_data_io : }\n");
        return;
        break;

      default:
        INT_ERROR("read_data_io");
        break;
    }
  PRDBG("read_data_io :   {NONE_T}\n");
  PRDBG("read_data_io : }\n");
#endif
}


/*
 * NOM		lookup_data_io
 * FONCTION	Fonction de recherche de fonction d'entree/sortie de data
 * ENTREES	Header *h    : liste des fonctions d'entree/sortie connues
 *              Node *type   : type associe a la fonction recherchee
 *              Node *access : moyen d'acces (NON UTILISE)
 *              int *rw      : pointeur sur le sens de transfert des donnees
 * SORTIES	Cell *lookup_data_io : pointeur sur la cellule comportant
 *                                     la fonction si elle existe, NULL sinon.
 */
#ifdef __STDC__
Cell *lookup_data_io(Header *h, Mwarg *a, Io *rw)
#else
Cell *lookup_data_io(h, a, rw)
Header *h;
Mwarg *a;
short *rw;
#endif
{
  Cell *c;
  char *name;
  Desc *d;

#ifdef DEBUG
  PRDBG("lookup_data_io(0x%x, 0x%x, %s)\n", (unsigned long)h, (unsigned long)a,
        rw==NULL?"NULL":(*rw==READ?"READ":(*rw==WRITE?"WRITE":"UNKNOWN")));
  PRDBG("lookup_data_io : a->name = %s\n", a->name);
#endif

  switch(a->t) {
    case OPTION:
      d = &(a->d.o.d);
      break;
    case NEEDEDARG:
    case VARARG:
    case OPTIONARG:
      d = &(a->d.a);
      break;
    default:
      INT_ERROR("lookup_data_io");
      break;
  }

  name = NULL;
  if (a->type->name == USRTYPID) {
    Symbol *s;
    if ((s = LOOKUP(a->type->val.text)) != NULL &&
        GET_OBJ(s) == TYPEDEF) {
      name = GET_SNAME(s);
    }
  }

#ifdef DEBUG
  PRDBG("lookup_data_io : search for \"%s\" with rw = %s\n",
        name==NULL?"<SCALAR>":name,
        rw==NULL?"NULL":(*rw==READ?"READ":(*rw==WRITE?"WRITE":"UNKNOWN")));
#endif


  for (c = GET_FIRST(h); c != NULL; c = GET_NEXT(c))
    switch(GET_IO_T(c)) {
      case MW2_T:
        if (d->t == FILEARG) {
          if (name != NULL) {
            if ((rw == NULL || *rw == GET_RW(c)) &&
                !strcmp(name, GET_IO_TYPE(c))) {
#ifdef DEBUG
              PRDBG("lookup_data_io : %s found : return 0x%x\n", GET_IO_TYPE(c), (unsigned long)c);
#endif
              return c;
            }
          }
        }
        break;

      case NONE_T:
#ifdef DEBUG
        PRDBG("lookup_data_io : NONE_T : not found\n");
#endif
        return NULL;
        break;

      default:
        if (d->t == SCALARARG) {
          if (d->v.p.t == GET_IO_T(c) &&
            (rw == NULL || *rw == GET_RW(c))) {
#ifdef DEBUG
            PRDBG("lookup_data_io : <SCALAR> found : return 0x%x\n", (unsigned long)c);
#endif
            return c;
          }
        }
        break;
    }
#ifdef DEBUG
  PRDBG("lookup_data_io : not found\n");
#endif
  return NULL;
}


/*
 * NOM		lookup_data_io_for_node
 * FONCTION	Fonction de recherche de fonction d'entree/sortie de data
 * ENTREES	Header *h    : liste des fonctions d'entree/sortie connues
 *              Node *type   : type associe a la fonction recherchee
 *              Node *access : moyen d'acces (NON UTILISE)
 *              int *rw      : pointeur sur le sens de transfert des donnees
 * SORTIES	Cell *lookup_data_io : pointeur sur la cellule comportant
 *                                     la fonction si elle existe, NULL sinon.
 */
#ifdef __STDC__
Cell *lookup_data_io_for_node(Header *h, Node *type, Node *access, Io *rw)
#else
Cell *lookup_data_io_for_node(h, type, access, rw)
Header *h;
Node *type;
Node *access;
int *rw;
#endif
{
  Cell *c;
  char *name;

  if (type->name == USRTYPID) {
    Symbol *s;
    if ((s = LOOKUP(type->val.text)) != NULL && GET_OBJ(s) == TYPEDEF)
      name = GET_SNAME(s);
    else
      return NULL;
    for (c = GET_FIRST(h); c != NULL; c = GET_NEXT(c)) {
      if (GET_IO_T(c) == MW2_T &&
          (rw == NULL || *rw == GET_RW(c)) &&
          !strcmp(name, GET_IO_TYPE(c)))
        return c;
    }
    return NULL;
  }
  else
    return NULL;
}


static char buffer[BUFSIZ];

/*
 * FONCTION	Retourne la bonne fonction avec les bons arguments en appel
 * ENTREES	int rw        : sens de transfert des donnees
 *              Mwarg *a      : Cellule d'argument MegaWave2
 *              
 * SORTIES	char *mwio : pointeur sur la chaine de caractere contenant
 *                           la fonction d'interfacage.
 */
#ifdef __STDC__
char *mwio(Io rw, Mwarg *a)
#else
char *mwio(rw, a)
int rw;
Mwarg *a;
#endif
{
  Cell *c;
  int no = 0; /* TEMPORAIRE : pour compil ! */

  if ((c = LOOKUP_DATA_IO(a, &rw)) != NULL) {
    switch(rw) {
      case READ :
        if (GET_IO_T(c) == MW2_T)
          sprintf(buffer, "%s(argv[optind + %d], type, comment, &%s)", 
                                                   GET_FUNCTION(c), no, a->name);
        else
          sprintf(buffer, "%s(argv[optind + %d])", GET_FUNCTION(c));
        break;
      case WRITE :
        if (GET_IO_T(c) == MW2_T)
          sprintf(buffer, "%s(argv[optind + %d], type, type_force, comment, %s)",
                                                   GET_FUNCTION(c), no, a->name);
        else
          INT_ERROR("mwio");
        break;
      default:
        INT_ERROR("mwio");
        break;
    }
    return buffer;
  }
  else
    return NULL;
}
