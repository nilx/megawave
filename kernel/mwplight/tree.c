/*~~~~~~~~~~~~~~ mwplight - The MegaWave2 Light Preprocessor ~~~~~~~~~~~~~~~~~~~

 Manage trees : header/arguments and function/parameters

 Author : Jacques Froment
 Date : 2006
 Version : 0.2
 Versions history :
   0.1 (August 2005, JF) initial internal release
   0.2 (February 2006, JF) added include <string.h> (Linux 2.6.12 & gcc 4.0.2)
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


/*========== HEADER / ARGUMENTS ==========*/

/*~~~~~~~~~~ Create a new argument ~~~~~~~~~~*/

#ifdef __STDC__
Arg *new_arg(void)
#else
Arg *new_arg()
#endif
{
  Arg *arg;

  arg=(Arg *) malloc(sizeof(Arg));
  if (!arg) Error("Not enough memory to create a new Argument");

  arg->Atype=arg->IOtype=arg->ICtype=NONE;

  arg->Flag='\0';
  strcpy(arg->H_id,"");
  strcpy(arg->C_id,"");
  strcpy(arg->Cmt,"");

  strcpy(arg->Val,"");  
  strcpy(arg->Min,"");  
  strcpy(arg->Max,"");  

  arg->var=NULL;

  arg->previous=arg->next=NULL;
  return(arg);
}

/*~~~~~~~~~~ Create a new header ~~~~~~~~~~*/

#ifdef __STDC__
Header *new_header(void)
#else
Header *new_header()
#endif
{
  Header *h;

  h=(Header *) malloc(sizeof(Header));
  if (!h) Error("Not enough memory to create a new Header");

  strcpy(h->Name,"");
  strcpy(h->Author,"");
  strcpy(h->Version,"");
  strcpy(h->Function,"");
  strcpy(h->Labo,"");
  strcpy(h->Group,"");

  h->usage=h->retmod=NULL;

  h->NbOption=h->NbNeededArg=h->NbVarArg=h->NbOptionArg=h->NbNotUsedArg=0;

  return(h);
}


/*~~~~~~~~~~ Dump the content of an argument (for debug) ~~~~~~~~~~*/

#ifdef __STDC__
void dump_arg(Arg *a)
#else
dump_arg(a)
Arg *a;
#endif
{
  if (!a) Error("NULL arg !");

  printf("Dump of arg=%x\n",a);
  printf(" previous=%x  \t next=%x\n",a->previous,a->next);
  printf(" Atype=%d\n",a->Atype);
  printf(" IOtype=%d\n",a->IOtype);
  printf(" ICtype=%d\n",a->ICtype);
  if (a->Flag!='\0') printf(" Flag='%c'\n",a->Flag);
  else printf(" Flag=(NULL)\n");
  printf(" H_id='%s'\n",a->H_id);
  printf(" C_id='%s'\n",a->C_id);
  printf(" Cmt='%s'\n",a->Cmt);
  printf(" Val='%s'\n",a->Val);
  printf(" Min='%s'\n",a->Min);
  printf(" Max='%s'\n",a->Max);

  printf("\n");
}


/*~~~~~~~~~~ 
  Check consistency of H.
 ~~~~~~~~~~*/

#ifdef __STDC__
void CheckConsistencyH(void)
#else
void CheckConsistencyH()
#endif
{
  Arg *a,*b;
  double m0,m1;

  if ((H->Name[0]<'a')||(H->Name[0]>'z'))
    Error("Invalid header : incorrect module's name or missing name statement");

  if (strlen(H->Author)<3)
    Error("Invalid header : incorrect list of author(s) or missing author statement");

  if ((H->Version[0]<'0')||(H->Version[0]>'9'))
    Error("Invalid header : incorrect version number or missing version statement");

  if (strlen(H->Function)<3)
    Error("Invalid header : incorrect function description or missing function statement");  

  if (!H->usage) Error("Invalid header : missing usage statement");  
  for (a=H->usage;a;a=a->next)
    {
      /* C_id is always required, check it first */
      if (a->C_id[0]=='\0') 
	Error("Invalid usage for Cmt=\"%s\" : C_id is always required",a->Cmt);  

      for (b=H->usage;b;b=b->next)
	if ((a!=b)&&(strcmp(a->C_id,b->C_id)==0))
	Error("Invalid usage for C_id=\"%s\" : duplicate use of this C variable name",a->C_id);  

      if (a->Atype==NONE) 
	Error("Invalid usage for C_id=\"%s\" : incorrect argument type",a->C_id);  

      if ((a->IOtype!=READ)&&(a->IOtype!=WRITE))
	Error("Invalid usage for C_id=\"%s\" : incorrect I/O type",a->C_id);  

      if (a->ICtype != NONE)
	/* Check cases where interval checking is not allowed */
	{
	  if (a->Min=='\0')
	    Error("Invalid usage for C_id=\"%s\" : internal inconsistency between ICtype (not NULL) and Min (NULL)",a->C_id);  	          
	  if (ISARG_FLAGOPT(a))
	    Error("Invalid usage for C_id=\"%s\" : interval checking is not allowed in flag option",a->C_id);  	    
	  if (ISARG_OUTPUT(a))
	    Error("Invalid usage for C_id=\"%s\" : interval checking is not allowed in output",a->C_id);  	    
	  if (ISARG_NOTUSED(a))
	    Error("Invalid usage for C_id=\"%s\" : interval checking is not allowed in notused argument",a->C_id);  	    
	}

      /* Check Flag */
      if (ISARG_OPTION(a))
	{
	  if (a->Flag=='\0')
	    Error("Invalid usage for C_id=\"%s\" : unknown flag associated to this option",a->C_id);  	    	
	}
      else
	{
	  if (a->Flag!='\0')
	    Error("Invalid usage for C_id=\"%s\" : flag is allowed in option only",a->C_id);  	    	
	}

      /* Check H_id */
      if (ISARG_NEEDED(a)||ISARG_OPTARG(a))
	{
	  if (a->H_id[0]=='\0')
	    Error("Invalid usage for C_id=\"%s\" : H_id is required for needed and optional arguments",a->C_id);  	    		  
	}
      else
	{
	  if ((ISARG_VARIABLE(a)||ISARG_NOTUSED(a))&&(a->H_id[0]!='\0'))
	    Error("Invalid usage for C_id=\"%s\" : H_id is not allowed in variable or not used arguments",a->C_id);  	    		  	    
	}

      if (strlen(a->Cmt)<3)
	Error("Invalid usage for C_id=\"%s\" : incorrect comment",a->C_id);  	

      if (ISARG_DEFAULT(a))
	/* Check cases where default input value is not allowed */	
	{
	  if (ISARG_FLAGOPT(a))
	    Error("Invalid usage for C_id=\"%s\" : default input value is not allowed in flag option",a->C_id);  	    
	  if (ISARG_NEEDED(a))
	    Error("Invalid usage for C_id=\"%s\" : default input value is not allowed in needed argument",a->C_id);  	    
	  if (!ISARG_INPUT(a))
	    Error("Invalid usage for C_id=\"%s\" : default input value requires input",a->C_id);  	    
	  if (ISARG_VARIABLE(a))
	    Error("Invalid usage for C_id=\"%s\" : default input value is not allowed in variable argument",a->C_id);  	    

	  if (ISARG_NOTUSED(a))
	    Error("Invalid usage for C_id=\"%s\" : default input value is not allowed in notused argument",a->C_id);  	    
	}

      /* Check interval checking.
	 Cases where interval checking is not allowed are assumed to be checked with ICtype 
      */
      if ( ((a->Min[0]=='\0')&&(a->Max[0]!='\0')) || ((a->Min[0]!='\0')&&(a->Max[0]=='\0')) )
	Error("Invalid usage for C_id=\"%s\" : incorrect interval checking specification (only Min or Max specified)",a->C_id);  	          
      if (a->Min[0]!='\0')
	{
	  if (a->ICtype==NONE)
	    Error("Invalid usage for C_id=\"%s\" : internal inconsistency between ICtype (NULL) and Min (not NULL)",a->C_id);  	          
	  m0=(double) atof(a->Min);
	  m1=(double) atof(a->Max);
	  if (m0 >= m1) Error("Invalid usage for C_id=\"%s\" : incorrect interval checking specification (Min >= Max)",a->C_id);  	          
	}

    }

}

/*========== VARIABLE / VARFUNC / CBODY ==========*/

/*~~~~~~~~~~ Create a new variable ~~~~~~~~~~*/

#ifdef __STDC__
Variable *new_variable(void)
#else
Variable *new_variable()
#endif
{
  Variable *var;

  var=(Variable *) malloc(sizeof(Variable));
  if (!var) Error("Not enough memory to create a new variable");

  var->Ctype=NONE;

  strcpy(var->Stype,"");
  strcpy(var->Ftype,"");
  strcpy(var->Name,"");
  strcpy(var->Cstorage,"");

  var->PtrDepth=0;
  var->DeclType=NONE;

  var->arg=NULL;

  var->previous=var->next=NULL;
  return(var);
}

/*~~~~~~~~~~ Create a new varfunc ~~~~~~~~~~*/

#ifdef __STDC__
VarFunc *new_varfunc(void)
#else
VarFunc *new_varfunc()
#endif
{
  VarFunc *f;
  Variable *var;


  f=(VarFunc *) malloc(sizeof(VarFunc));
  if (!f) Error("Not enough memory to create a new VarFunc");
  var=new_variable();
  f->v=var;
  f->Itype=I_UNDEFINED;
  f->l0=f->l1=-1;

  f->param=NULL;
  f->previous=f->next=NULL;

  return(f);
}

/*~~~~~~~~~~ Create a new Cbody ~~~~~~~~~~*/

#ifdef __STDC__
Cbody *new_cbody(void)
#else
Cbody *new_cbody()
#endif
{
  Cbody *c;

  c=(Cbody *) malloc(sizeof(Cbody));
  if (!c) Error("Not enough memory to create a new Cbody");

  c->varfunc=NULL;
  c->mfunc=NULL;

  return(c);
}

/*~~~~~~~~~~ Dump the content of a Variable (for debug) ~~~~~~~~~~*/

#ifdef __STDC__
void dump_variable(Variable *v)
#else
dump_variable(v)
Variable *v;
#endif
{
  if (!v) Error("NULL variable !");

  printf("  Dump of variable=%x\n",v);
  printf("   Name='%s'\n",v->Name);
  printf("   Scalar type='%s'\n",v->Stype);
  printf("   Full type='%s'\n",v->Ftype);
  printf("   Ctype=%d\n",v->Ctype);
  printf("   PtrDepth=%d\n",v->PtrDepth);
  printf("   Cstorage='%s'\n",v->Cstorage);
  printf("   previous=%x  \t next=%x\n",v->previous,v->next);
}

/*~~~~~~~~~~ Dump the content of a VarFunc (for debug) ~~~~~~~~~~*/

#ifdef __STDC__
void dump_varfunc(VarFunc *f)
#else
dump_varfunc(f)
VarFunc *f;
#endif
{
  Variable *p;
  int i;

  if (!f) Error("NULL varfunc !");

  printf("Dump of varfunc=%x\n",f);
  printf(" Itype=%d\n",f->Itype);
  printf(" Name='%s'\n",f->v->Name);
  printf(" Scalar type='%s'\n",f->v->Stype);
  printf(" Full type='%s'\n",f->v->Ftype);
  printf(" Ctype=%d\n",f->v->Ctype);
  printf(" PtrDepth=%d\n",f->v->PtrDepth);
  printf(" Cstorage='%s'\n",f->v->Cstorage);
  printf(" l0=%ld \t l1=%ld\n",f->l0,f->l1);
  printf(" previous=%x  \t next=%x\n",f->previous,f->next);
  if (!f->param)
    printf(" no param\n");
  else
    for (p=f->param, i=1; p; p=p->next,i++)
      {
	printf(" Parameter #%d :\n",i);
	dump_variable(p);
      }
  printf("\n");
}


/*========== CWORD / CINSTRUCTION ==========*/

/*~~~~~~~~~~ Create a new cword ~~~~~~~~~~*/

#ifdef __STDC__
Cword *new_cword(void)
#else
Cword *new_cword()
#endif
{
  Cword *cword;

  cword=(Cword *) malloc(sizeof(Cword));
  if (!cword) Error("Not enough memory to create a new Cword");

  cword->Wtype=W_UNDEFINED;
  strcpy(cword->Name,"");

  cword->previous=cword->next=NULL;
  return(cword);
}

/*~~~~~~~~~~ Create a new Cinstruction ~~~~~~~~~~*/

#ifdef __STDC__
Cinstruction *new_cinstruction(void)
#else
Cinstruction *new_cinstruction()
#endif
{
  Cinstruction *c;

  c=(Cinstruction *) malloc(sizeof(Cinstruction));
  if (!c) Error("Not enough memory to create a new Cinstruction");

  strcpy(c->phrase,"");
  c->Itype=I_UNDEFINED;
  c->nparam=c->ndatatype=-1;
  c->nvar=0;
  c->wfirst=NULL;
  c->previous=c->next=NULL;
  return(c);
}

/*~~~~~~~~~~ Delete a Cinstruction tree ~~~~~~~~~~*/

#ifdef __STDC__
void delete_cinstruction(Cinstruction *c)
#else
void delete_cinstruction(c)
Cinstruction *c;
#endif
{
  Cword *w,*wn;

  if (!c) Error("Cannot delete NULL Cinstruction !");

  for (w=wn=c->wfirst;w&&wn;w=wn)
    {
      wn=w->next;
      free(w);
    }

  if (c->next) delete_cinstruction(c->next);
  free(c);
}

/*~~~~~~~~~~ Merge a Cinstruction ~~~~~~~~~~

  Merge Cwords in a Cinstruction to avoid
  any c->next.

*/

#ifdef __STDC__
void merge_cinstruction(Cinstruction *c)
#else
void merge_cinstruction(c)
Cinstruction *c;
#endif
{
  Cword *cw0,*cw1;

  if (!c) Error("Cannot merge NULL Cinstruction !");

  if (c->next) 
    {
    doitagain:
      cw0=c->wfirst;
      while (cw0->next) cw0=cw0->next;

      cw1=c->next->wfirst;
      cw0->next=cw1;
      cw1->previous=cw0;

      if (c->next->next) 
	{
	  c->next=c->next->next;
	  goto doitagain;
	}
      else c->next=NULL;
    }
}
