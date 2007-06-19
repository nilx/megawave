/*~~~~~~~~~~~~~~ mwplight - The MegaWave2 Light Preprocessor ~~~~~~~~~~~~~~~~~~~

 Generate the I-file (intfile : interface file)

 Author : Jacques Froment
 Date : 2005
 Version : 0.1
 Versions history :
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

#include "mwpl_main.h"

/*~~~ Set the <protobuf> variable from the content of C->mfunc :
      prototype the main function using K&R convention.

      This text is to be set in the fsummary field of the Mwiline
      structure (see kernel/lib/include/mwi.h). From this, a
      full K&R and ANSI C compliant prototype can be derived
      using call_proto() (in kernel/lib/src/mw.c).
      When the traditional mwp processor would be removed,
      this should be simplified by getting a protobuf with
      both K&R and ANSI C declaration. Then call_proto could
      be removed and in the M-file, the default main function
      declaration (usually not ANSI compliant) could be replaced 
      by this one (update mwpl_mfile.c). So, prototypes would become 
      fully ANSI compliant without having to change the source of modules.
*/

#define ADDPROTO(a) if ((strlen(protobuf)+strlen(a))>=STRSIZE) Error("[setprotobuf] Buffer overflow for prototype (increase STRSIZE)"); strcat(protobuf, a)

#ifdef __STDC__
void setprotobuf(void)
#else
void setprotobuf()
#endif
{
  VarFunc *f;
  Variable *p;
  char buf[STRSIZE];

  if (!C) Error("[setprotobuf] NULL C tree. Need C body to be parsed");
  f=C->mfunc;
  if (!f) Error("[setprotobuf] NULL main function");

  /* Function name and arguments list */
  sprintf(protobuf,"%s %s ( ",f->v->Ftype,f->v->Name);
  for (p=f->param; p; p=p->next)
    {
      if (p!=f->param) strcat(protobuf," , ");
      ADDPROTO(p->Name);
    } 
  ADDPROTO(" )\\n");
  
  /* Type of arguments */
  for (p=f->param; p; p=p->next)
    {
      sprintf(buf,"%s %s ;\\n",p->Ftype,p->Name);
      ADDPROTO(buf);
    }  
  ADDPROTO("\\n");
}

/*~~~ main entry : generate I-file ~~~
*/

#ifdef __STDC__
void genIfile(void)
#else
void genIfile()
#endif
{ 

  if (Ifile[0]=='\0') sprintf(Ifile,"int_%s.c",H->Name);
  
  if ((fi = fopen(Ifile, "w")) == NULL) 
    Error("Cannot open I-file '%s' for writing",Ifile);    

  /* Write fields of the Mwiline structure (see kernel/lib/include/mwi.h) */

  fprintf(fi, "{ \"%s\", _%s, usage_%s, \"%s\", \"%s\", \"%s\", \"%s\"}\n",
	  module_name, module_name, module_name, group_name, H->Function,
	  usagebuf, protobuf);
  
  fclose(fi);
}
