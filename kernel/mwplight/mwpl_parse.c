/*~~~~~~~~~~~~~~ mwplight - The MegaWave2 Light Preprocessor ~~~~~~~~~~~~~~~~~~~

 parse module source

 Author : Jacques Froment
 Date : 2007
 Version : 1.0
 Versions history :
   0.1 (August 2005, JF) initial internal release
   0.2 (February 2006, JF) added include <string.h> (Linux 2.6.12 & gcc 4.0.2)
   1.0 (April 2007, JF)  final revision, ready for external release
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
#include <unistd.h>
#include <string.h>

#include "mwpl_main.h"

/* Header ID for MegaWave2 */
#define HEADER_ID1 "/* mwcommand"
#define HEADER_ID1b "/*mwcommand"
/* New header ID for MegaWave2 */
#define HEADER_ID2 "/* mwheader"
#define HEADER_ID2b "/*mwheader"


/*~~~~~ Global variables ~~~~~*/

Header *H=NULL; /* The header tree */
Cbody *C=NULL;  /* The C body tree */
FILE *fs=NULL;  /* file pointer on the source module */
struct stat module_fstat; /* store info about the module file */


/*~~~~~~~~~~ 
  Enter the header.
  Return the header ID number (1 or 2).
  ~~~~~~~~~~
*/
#ifdef __STDC__
int EnterHeader(void)
#else
int EnterHeader()
#endif

{
  char line[STRSIZE];
  int l;

  if (!fs) Error("NULL file pointer fs");
  do
    {
      l=getline(line);
      removespaces(line);
      /*printf("line='%s'\n",line);*/
      if ((strcmp(line,HEADER_ID1)==0)||(strcmp(line,HEADER_ID1b)==0)) return(1);
      if ((strcmp(line,HEADER_ID2)==0)||(strcmp(line,HEADER_ID2b)==0)) return(2);
    }
  while (l!=EOF);
  Error("Cannot find MegaWave2 Header ID ('%s' or '%s')",HEADER_ID1,HEADER_ID2);
}


/*~~~~~~~~~~ 
  Parse the header.
  Return EOF (error) or EOH (normal case).
  ~~~~~~~~~~
*/
#ifdef __STDC__
int ParseHeader(void)
#else
int ParseHeader()
#endif

{
  char s[STRSIZE];
  char name[STRSIZE];
  char value[STRSIZE];
  int l;
  Arg *a;

  if (!fs) Error("NULL file pointer fs");
  H=new_header();
  while (((l=getsentence(s))!=EOF)&&(l!=EOH))
    /* s contains the sentence to parse */
    {
      GetHeaderStatement(s, name, value);
      AnalyseHeaderStatement(name,value);
    }
  inside_header=0;

  /* Compute the number of arguments, by types */
  for (a=H->usage;a;a=a->next)
    switch(a->Atype)
      {
      case OPTION:
	H->NbOption++;
	break;
      case NEEDEDARG:
	H->NbNeededArg++;
	break;
      case VARARG:
	H->NbVarArg++;
	break;
      case OPTIONARG:
	H->NbOptionArg++;
	break;
      case NOTUSEDARG:
	H->NbNotUsedArg++;
	break;
      default:
	Error("Invalid usage for C_id=\"%s\" : incorrect argument type",a->C_id);
      }
  
  
  return(l);
}


/*~~~~~~~~~~ 
  Parse the C body.
  ~~~~~~~~~~
*/
#ifdef __STDC__
void ParseCbody(void)
#else
void ParseCbody()
#endif

{

#ifdef DEBUG
  printf("[ParseCbody]\n");
#endif

  if (!fs) Error("NULL file pointer fs");

  Init_Cuserdatatype();

  C=new_cbody();
  while (GetNextInstruction()!=0);
  
  if (C->mfunc==NULL) Error("Cannot find module's function in C body.\nRemember that the main function's name must be the same that the module's filename");

  Free_Cuserdatatype();
}

/*~~~~~~~~~~ 
  Complete the H tree by setting fields that need the C body to be parsed (C tree filled),
  i.e. Ctype, Vtype and v fields, and complete the C tree with arg field.
  Perform also some checking.
~~~~~~~~~~
*/
#ifdef __STDC__
void CompleteHC(void)
#else
void CompleteHC()
#endif

{
  VarFunc *f;
  Arg *a;
  Variable *v,*p;

  if (!C) Error("[CompleteHC] NULL C tree. Need C body to be parsed");
  f=C->mfunc;
  if (!f) Error("[CompleteHC] NULL main function");  
  if ((f->v->Ctype != VOID_T)&&(f->v->Ctype != MW2_T)&&((f->v->Ctype < SCALARMIN_T)||(f->v->Ctype > SCALARMAX_T)))
    {
      if (f->v->Ctype != NONE)
	Error("Unsupported return type \"%s\" for the main function (Ctype=%d).\nReturn types can be only scalar or MegaWave2 internal types.\nAlso, remember that the light preprocessor does not process #define and #include directives.",f->v->Ftype,f->v->Ctype);  
      
      /* Main function does not have any declaration of type.
	 In such a case, the C language considers the type as "int".
      */
      f->v->Ctype=INT_T;
      strcpy(f->v->Stype,"int");
      strcpy(f->v->Ftype,"int");
      f->v->PtrDepth=0;
    }

  for (a=H->usage;a;a=a->next)
    {
      /* For each argument in the usage, search for the 
	 corresponding parameter in the main function.
      */
      for (p=f->param; p&&(strcmp(p->Name,a->C_id)!=0); p=p->next);
      if (p)
	/* parameter found */
	{
	  a->var=p;
	  p->arg=a;
	}
      else
	/* Not found : check if this variable is not the return value of the function */
	{
	  if (strcmp(f->v->Name,a->C_id)==0)
	    /* yes, this is the return value of the function */
	    {
	      if (H->retmod)
		Error("Two variables '%s' and '%s' on the return value of the main function",H->retmod->C_id,a->C_id);
	      a->var=f->v;
	      f->v->arg=a;
	      H->retmod=a;
	      if (a->var->Ctype==VOID_T)
		Error("Invalid usage for C_id=\"%s\" : the main function does not return any value",a->C_id);  	          
	    }
	  else
	    Error("Variable '%s' is declared in the header but is missing in main function",a->C_id);
	}
    }

  for (v=f->param; v; v=v->next)
    {
      if ((v->Ctype != MW2_T)&&((v->Ctype < SCALARMIN_T)||(v->Ctype > SCALARMAX_T)))
	Error("Unsupported type \"%s\" for the parameter \"%s\" of the main function (Ctype=%d).\nI/O variables can be only of scalar types or of MegaWave2 internal types (or pointers to).\nAlso, remember that the light preprocessor does not process #define and #include directives.", v->Ftype,v->Name,v->Ctype);  
      if (!v->arg)
	Error("Parameter '%s' of the main function is not declared in the header",v->Name);	
    }
  
}

/*~~~~~~~~~~ Parse the module ~~~~~~~~~~
*/
#ifdef __STDC__
void parse(void)
#else
void parse()
#endif
{
 
  fs = fopen(module_file,"r");
  if (!fs) Error("Cannot open file \"%s\" for reading",module_file);
  
  /* Store info about the module file */
  fstat(fileno(fs),&module_fstat);

  /* Init variables */
  inside_comment=inside_header=0;

  /* Parse the header and build the H tree */
  inside_header=EnterHeader();
  if (ParseHeader()==EOF) Error("Unexpected end of file while parsing the header !");

  /* Check consistency of the header loaded in H */
  CheckConsistencyH();

  /* Parse the C body and build the C tree */
  ParseCbody();
  
  /* Complete the H and C trees and perform some checking */
  CompleteHC();

  fclose(fs); fs=NULL;
}


