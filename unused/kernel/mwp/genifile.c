/* Generate i file
   Version 1.2
   Main changes :
   V1.2 (JF, 23/02/2006) added include <string.h>
*/

/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <string.h>

#include "bintree.h"
#include "symbol.h"
#include "token.h"
#include "mwarg.h"
#include "genmain.h"
#include "io.h"
#define GENIFILE_DEC
#include "genifile.h"

/* for mkleaf(NAME,) */
#include "y.tab.h"

extern FILE * mwerr;

#ifdef __STDC__
void genifile(FILE *fd)
#else
genifile(fd)
FILE *fd;
#endif
{
  int l;
  char buffer[BUFSIZ];

  /* for proto */
  extern Node * mwfuncdecl;
  Node * CmpdStmtTmp;

  l = strlen(mwfunction->val.text)-2;
  strncpy(buffer, mwfunction->val.text+1, l);
  buffer[l] = '\0';
  /*  {Function name, "group", "function", "usage", "proto"}, */        
  fprintf(fd, "{ \"%s\", _%s, usage_%s, \"%s\", %s, \"%s\", \"", 
              mwname->val.text, mwname->val.text, mwname->val.text, groupbuf, 
              mwfunction->val.text, usagebuf);
  
  CmpdStmtTmp = mwfuncdecl->right->right;
  mwfuncdecl->right->right = NULL;
  printprotonode(fd, mwfuncdecl);
  mwfuncdecl->right->right = CmpdStmtTmp;

  fprintf(fd, "\"}\n");
}


#ifdef __STDC__
changegroup(void)
#else
changegroup()
#endif
{
  char grp[BUFSIZ];
  
  if (mwgroup != NULL) /* Group is defined in the module's header */
    {
      int l;
      l = strlen(mwgroup->val.text)-2;
      strncpy(grp, mwgroup->val.text+1, l);
      grp[l] = '\0';
      if ((groupbuf[0]!='\0')&&(strcmp(groupbuf,grp)!=0))
	{
	  fprintf(mwerr,"Warning : group \"%s\" given in the header does not match directory name \"%s\" !\n",grp,groupbuf);
	  fprintf(mwerr,"          Forget the group given in the header (you should remove it).\n");
	  strcpy(mwgroup->val.text,groupbuf);
	}
    }
  else /* Group is not defined : take the group name to be the directory */
    mwgroup = mkleaf(NAME, NULL, 0, (void *)groupbuf);
}



