/*~~~~~~~~~~~~~~ mwplight - The MegaWave2 Light Preprocessor ~~~~~~~~~~~~~~~~~~~

 seek and analyse some instructions in C body 

 Author : Jacques Froment
 Date : 2005-2007
 Version : 1.1
 Versions history :
   0.1 (August 2005, JF) initial internal release
   0.2 (March 2006, JF) inclusion of Wpack2d
   1.0 (March 2007, JF) more explicit error messages, ready for external release.
   1.1 (May 2007, JF) 'inline' function specifier added;
                       bug on global C qualifier declaration corrected.
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

/* C keywords sorted by types */

/* C data type. Put also most common types defined in include files */
char *Cdatatype[]={"char","double","float","int","long","short","void",

		   "time_t", "bool", "byte",
		   ""};

char *Cmodifier[]={"long","short","signed","unsigned",""};
char *Cpointer[]={"*",""}; /* notice : [] not allowed */
char *Cqualifier[]={"const","volatile",""};
char *Cfuncspecifier[]={"inline",""}; /* added to c99 language specification */
char *Cstorage[]={"auto","register","static","extern",""};
char *Cdeclaretype[]={"typedef",""};
char *Cstruct[]={"enum","struct","union",""};
char *Cotherkey[]={"break","case","continue","default","do","else","for","goto","if","return","sizeof","switch","while",""};

/* List all MW2 data type, INCLUDING those without I/O procedures (such as Point_curve) */
char *Cmwdatatype[]={"Cimage","Fimage","Cmovie","Fmovie","Polygon","Polygons","Fpolygon","Fpolygons",
		     "Curve","Fcurve","Dcurve","Curves","Fcurves","Dcurves","Fsignal","Wtrans1d","Wtrans2d",
		     "Vchain_wmax","Vchains_wmax","Ccimage","Cfimage","Modules","Ccmovie","Cfmovie",
		     "Morpho_line","Fmorpho_line","Morpho_set","Morpho_sets","Mimage","Cmorpho_line",
		     "Cfmorpho_line","Cmorpho_set","Cmorpho_sets","Cmimage","Shape","Shapes","Rawdata",
		     "Flist","Flists","Dlist","Dlists","Wpack2d",

		     "Point_curve","Point_fcurve","Wframe","Color","Point_plane","Wpanel",
		     ""};

/* Data type defined by the user in the C body */
char *(Cuserdatatype[MAXDT]);
int Nudt=0; /* Number of user-defined data type */

#define Is_Cdatatype(x) (Is_Ctype((x),Cdatatype))
#define Is_Cmodifier(x) (Is_Ctype((x),Cmodifier))
#define Is_Cpointer(x) (Is_Ctype((x),Cpointer))
#define Is_Cqualifier(x) (Is_Ctype((x),Cqualifier))
#define Is_Cfuncspecifier(x) (Is_Ctype((x),Cfuncspecifier))
#define Is_Cstorage(x) (Is_Ctype((x),Cstorage))
#define Is_Cdeclaretype(x) (Is_Ctype((x),Cdeclaretype))
#define Is_Cstruct(x) (Is_Ctype((x),Cstruct))
#define Is_Cotherkey(x) (Is_Ctype((x),Cotherkey))
#define Is_Ckeyword(x) (Is_Cdatatype(x)||Is_Cmodifier(x)||Is_Cpointer(x)||Is_Cqualifier(x)||Is_Cfuncspecifier(x)||Is_Cstorage(x) ||Is_Cdeclaretype(x)||Is_Cotherkey(x))
#define Is_Cmwdatatype(x) (Is_Ctype((x),Cmwdatatype))
#define Is_Cuserdatatype(x) (Is_Ctype((x),Cuserdatatype))
#define Is_Canydatatype(x) (Is_Cdatatype(x)||Is_Cmwdatatype(x)||Is_Cuserdatatype(x))


/*~~~~~~~~~~ 
  Init the Cuserdatatype array.
  ~~~~~~~~~~
*/
void Init_Cuserdatatype()

{
  int n;

  Nudt=0;
  for (n=0;n<=MAXDT;n++) 
    if ((Cuserdatatype[n]=(char *) malloc(DTSTRSIZE*sizeof(char)))==NULL)
      Error("Not enough memory to allocate Cuserdatatype !");
  Cuserdatatype[Nudt][0]='\0';
}

/*~~~~~~~~~~ 
  Free the Cuserdatatype array.
  ~~~~~~~~~~
*/
void Free_Cuserdatatype()

{
  int n;

  Nudt=0;
  for (n=0;n<=MAXDT;n++) free(Cuserdatatype[n]);
}

/*~~~~~~~~~~ 
  Add string <s> as a new user data type in <Cuserdatatype>.
  ~~~~~~~~~~
*/
#ifdef __STDC__
void Add_Cuserdatatype(char *s)
#else
void Add_Cuserdatatype(s)
char *s;
#endif

{
  Nudt++;
  if (Nudt > MAXDT) Error("Too many C user's data types. Maximum is %d\n",MAXDT);
  strcpy(Cuserdatatype[Nudt-1],s);
  Cuserdatatype[Nudt][0]='\0';
#ifdef DEBUG
  printf("New User data type \"%s\" added !\n",s);
#endif
}

/*~~~~~~~~~~ 
  Return 1 if string <s> is of type <type>, 0 elsewhere.
  <type> is assumed to be a list of C keywords.
  ~~~~~~~~~~
*/
#ifdef __STDC__
int Is_Ctype(char *s, char *type[])
#else
int Is_Ctype(s,type)
char *s;
char *type[];
#endif

{
  int i;
  for (i=0;(type[i][0]!='\0')&&(strcmp(s,type[i])!=0);i++);
  return(type[i][0]!='\0');
}


/*~~~~~~~~~~ 
  From the chain of words in <c>, analyse the instruction and set the other fields of <c>.
  Return 1 if the instruction is to be continued, 0 elsewhere.
  ~~~~~~~~~~
*/
#ifdef __STDC__
int AnalyseInstruction(Cinstruction *c)
#else
int AnalyseInstruction(c)
Cinstruction *c;
#endif

{
  Cword *cw,*cw0,*cwb;
  char *w;
  int nipar;  /* Number of non-interpreted parenthesis */

  /*#define P printf*/
#define P

  nipar=0;
  c->nparam=-1;
  c->nvar=0;
  
  /* If c is the continuation of an instruction, set the instruction type */
  if (c->previous) 
    {
      c->Itype=c->previous->Itype; P("Set Itype to previous one. ");
      c->nparam=c->previous->nparam; P("Set nparam=%d. ",c->nparam);
      c->nvar=c->previous->nvar; P("Set nvar=%d. ",c->nvar);      
    }

  /* Scan each word */
  for (cw=c->wfirst,cw0=NULL; cw; cw0=cw,cw=cw->next)
    {
      w=cw->Name;
      P("\nWord \"%s\": ",w);

      /*~~~~~ First, recognize keywords ~~~~~*/

      if (Is_Cdeclaretype(w)) 
	{
	  cw->Wtype=W_CDECLARETYPE;  P("W_CDECLARETYPE ");/* declaration of type : typedef */
	  
	  /* A typedef is not supposed to be encountered in other labelled instructions */
	  if (c->Itype!=I_UNDEFINED)
	    Error("Parse error : cannot interpret instruction \"%s\" as declaration of type",c->phrase);	    

	  c->Itype=I_CDECLARETYPE; P("I_CDECLARETYPE ");
	  continue;
	}
      if (Is_Cstruct(w)) 
	{
	  cw->Wtype=W_STRUCT;  P("W_STRUCT ");/* declaration of structure : struct, union, ... */
	  continue;
	}
      if (Is_Cstorage(w))
	{
	  cw->Wtype=W_CSTORAGE; P("W_CSTORAGE ");/* C storage : extern, static, register, ... */
	  continue;
	}
      if (Is_Cqualifier(w))
	{
	  cw->Wtype=W_CQUALIFIER; P("W_CQUALIFIER ");/* C qualifier : const, volatile */
	  continue;
	}
      if (Is_Cfuncspecifier(w))
	{
	  cw->Wtype=W_CFUNCSPECIFIER; P("W_CFUNCSPECIFIER ");/* C qualifier : const, volatile */
	  continue;
	}
      if (Is_Cdatatype(w))
	{
	  cw->Wtype=W_DATATYPE;  P("W_DATATYPE ");/* data type : char, double, ... */
	  if (c->Itype==I_FUNC_IN) c->ndatatype++;
	  continue;
	}
      if (Is_Cmodifier(w))
	{
	  cw->Wtype=W_CMODIFIER; P("W_CMODIFIER ");/* C modifier : long, unsigned, ... */
	  continue;
	}
      if (Is_Cpointer(w))
	{
	  cw->Wtype=W_CPOINTER; P("W_CPOINTER ");/* C pointer : *, [] */
	  continue;
	}
      if (Is_Cmwdatatype(w))
	{
	  cw->Wtype=W_CMWDATATYPE;   P("W_CMWDATATYPE ");/* MW type */
	  if (c->Itype==I_FUNC_IN) c->ndatatype++;
	  continue;
	}
      if (Is_Cuserdatatype(w))
	{
	  cw->Wtype=W_USERDATATYPE;  P("W_USERDATATYPE ");/* User's defined data type */
	  if (c->Itype==I_FUNC_IN) c->ndatatype++;
	  continue;
	}
      
      if (Is_Cotherkey(w))
	Error("Parse error : unexpected keyword \"%s\" outside any block of instructions",w);

      /*~~~~~ Word is not a keyword : try to interpret it using the context ~~~~~*/

      if (!IsStringCid(w))
	/*~~~~~ word is not a C identifier, such as a name~~~~~*/
	{

	  if (strcmp(w,"\"")==0) 
	    {
	      if ((!cw0)||(strcmp(cw0->Name,"\\")!=0))
		/* Enter a string : skip it */
		{
		  cw->Wtype=W_SEPARATOR; P("W_SEPARATOR ");
		  P("skip string ");
		  cw=cw->next;
		  while (cw&&((strcmp(cw->Name,"\"")!=0))) {P("%s ",cw->Name); cw=cw->next;}
		}
	      continue;
	    }

	  if (strcmp(w,"'")==0) 
	    {
	      if ((!cw0)||(strcmp(cw0->Name,"\\")!=0))
		/* Enter a char 'c' : skip it */
	      {
		cw->Wtype=W_SEPARATOR; P("W_SEPARATOR ");
		P("skip char ");
		cw=cw->next;
		while (cw&&((strcmp(cw->Name,"'")!=0))) {P("%s ",cw->Name); cw=cw->next;}
	      }
	      continue;
	    }

	  if (strcmp(w,";")==0) 
	    /* Should close the instruction */
	    {
	      cw->Wtype=W_SEPARATOR; P("W_SEPARATOR ");
	      if (cw->next) Error("Parse error in \"%s\" : unexpected ';' before end of instructions",c->phrase);
	      continue;
	    }

	  if ((strcmp(w,"{")==0)||(strcmp(w,"}")==0))
	    /* Instruction includes a block {} */
	    {
	      cw->Wtype=W_SEPARATOR; P("W_SEPARATOR ");
	      continue;
	    }
	  
	  if (strcmp(w,"(")==0) 
	    /* may be a function declaration */
	    {
	      cw->Wtype=W_SEPARATOR; P("W_SEPARATOR ");
	      if ( cw0 &&
		   ((cw0->Wtype==W_NAME) /* normal case */
		   ||
		    /* case with parenthesis such as  double (*RI) (); */
		    /* Added 27/06/05. See image/io/cfchgchannels.c and wave/sconvolve.c */
		    ((strcmp(cw0->Name,")")==0) && cw0->previous && (cw0->previous->Wtype==W_NAME)
		      && (c->Itype==I_UNDEFINED))
		    )
		   )
		/* previous word was name : interpret as a function declaration */	
		{
		  if (c->Itype!=I_UNDEFINED)
		    Error("Parse error in \"%s\" : unexpected declaration of function while scanning previous block of instructions.\nTip : check if there is no missing declaration of parameter in the previous function.\n",c->phrase);	    
		  c->Itype=I_FUNC_IN;  P("I_FUNC_IN ");
		  c->nparam=c->ndatatype=0;
		  if (cw0->Wtype==W_NAME) cwb=cw0;
		  else cwb=cw0->previous;
		  cwb->Wtype=W_FUNCNAME; P("\"%s\" : W_FUNCNAME ",cwb->Name);
		  c->nvar--;
		  continue;
		}
	      else 
		/* don't know what it is : skip it */
		{
		  nipar++;
		  continue;
		}
	    }
	  if (strcmp(w,")")==0)
	    {
	      cw->Wtype=W_SEPARATOR; P("W_SEPARATOR ");
	      if (c->Itype!=I_FUNC_IN) 
		{
		  nipar--;
		  if (nipar < 0) Error("Parse error : unexpected ')' outside function");		
		  continue; /* Added 27/06/05. See image/io/cfchgchannels.c and wave/sconvolve.c */
		}
	      if ((cw->next)&&((strcmp(cw->next->Name,";")==0)||(strcmp(cw->next->Name,",")==0)))
		/* This is a function prototype */
		  { 
		    if (c->ndatatype > 0) /* data type has been used inside the function parameter list : ANSI */
		      {
			if ((c->nparam>0)&&(c->ndatatype != c->nparam))
			  Error("Parse error in \"%s\" : number of types (%d) does not match number of parameters (%d) in ANSI function prototype",c->phrase,c->ndatatype,c->nparam);	    			  
			c->Itype=I_FUNCPROTO_ANSI; P("I_FUNCPROTO_ANSI "); 
			c->nparam=c->nvar=c->ndatatype; 
		      }
		    else
		      {
			c->Itype=I_FUNCPROTO_KR; P("I_FUNCPROTO_KR "); 
		      }
		  }
	      else
		/* This is a function declaration */
		  { 
		    if (c->ndatatype > 0)
		      {
			if (c->ndatatype != c->nparam)
			  Error("Parse error in \"%s\" : number of types (%d) does not match number of parameters (%d) in ANSI function declaration",c->phrase,c->ndatatype,c->nparam);	    			  
			c->Itype=I_FUNCDECL_ANSI; P("I_FUNCDECL_ANSI "); 
			c->nvar=c->ndatatype; 
		      }
		    else
		      {
			c->Itype=I_FUNCDECL_KR; P("I_FUNCDECL_KR "); 
		      }
		  }
	      continue;
	    }

	  if (strcmp(w,",")==0)
	      /* List of variables or parameters */
	    {
	      cw->Wtype=W_SEPARATOR; P("W_SEPARATOR ");
	      if ((c->Itype==I_FUNCPROTO_ANSI)||(c->Itype==I_FUNCPROTO_KR))
		/* To avoid error in case of several proto on the same line.
		   Ex.  extern float smean(),snorm();
		*/
		c->Itype=I_UNDEFINED;
	      continue;
	    }
	  
	  /* Other symbols */
	  if (cw0&&(cw0->Wtype==W_NAME))
	    {
	      /* previous word was name : should be a complex variable(s) definition 
		 we don't want to manage 
	      */
	      if (c->Itype==I_UNDEFINED) 
		{
		  c->Itype=I_VARDECL; P("I_VARDECL ");
		}
	      for (cw; cw && (strcmp(cw->Name,",")!=0); cw0=cw,cw=cw->next);
	      if (cw) continue; else goto EndofPass1;
	    }

	  /* Other case ; don't know what it is. 
	     May be symbol used in declaration such as '[' in char toto[10].
	     End analysis if the instruction is already interpreted.
	  */
	  if (c->Itype!=I_UNDEFINED) 
	    {
	      for (cw; cw && (strcmp(cw->Name,",")!=0); cw0=cw,cw=cw->next);
	      if (cw) continue; else goto EndofPass1;
	    }
	  

	  Error("Parse error : I don't know how to interpret symbol(s) \"%s\"",w);	    
	  continue;

	} /*  if (!IsStringCid(w)) */

      /*~~~~~ Word is a C identifier ~~~~~*/

      if (cw0&&((cw0->Wtype==W_DATATYPE)||(cw0->Wtype==W_CMWDATATYPE)||(cw0->Wtype==W_USERDATATYPE))
	  &&cw0->previous&&(cw0->previous->Wtype==W_CDECLARETYPE))
	/* typedef datatype newdatatype */
	   {
	     cw->Wtype=W_USERDATATYPE;  P("W_USERDATATYPE ");
	     Add_Cuserdatatype(w);
	     if (c->Itype==I_FUNC_IN) c->ndatatype++;
	     continue;
	   }

      if (cw0&&(cw0->Wtype==W_STRUCT))
	/* struct name */
	{
	  cw->Wtype=W_USERDATATYPE;  P("W_USERDATATYPE ");
	  /* Do not add this word as a datatype since it must be preceding 
	     by "struct" in order to be recognized as a datatype
	  */
	  if (c->Itype==I_FUNC_IN) c->ndatatype++;
	  continue;
	}
      
      if (cw0&&((cw0->Wtype==W_DATATYPE)||(cw0->Wtype==W_CMWDATATYPE)||(cw0->Wtype==W_USERDATATYPE)||
		(cw0->Wtype==W_CMODIFIER)||(cw0->Wtype==W_CPOINTER)||(strcmp(cw0->Name,"}")==0)))
	/* previous word was data type or modifier or pointer or end of block {}. 
	   This one may be new variable or function name */	
	{
	  if (c->Itype==I_CDECLARETYPE)
	    /* This name may be a new data type : add it */
	    {
	      cw->Wtype=W_USERDATATYPE;  P("W_USERDATATYPE ");
	      Add_Cuserdatatype(w);
	      if (c->Itype==I_FUNC_IN) c->ndatatype++;
	      continue;
	    }
	  else if (c->Itype!=I_FUNC_IN)
	    /* We are not inside the parameter list */
	    {
	      cw->Wtype=W_NAME;  P("W_NAME ");/* variable or function name */
	      c->nvar++;
	      continue;
	    }
	}

      if ((c->Itype==I_FUNC_IN)&&
	  ((!cw->next)||(strcmp(cw->next->Name,",")==0)||(strcmp(cw->next->Name,")")==0)))
	/* A parameter of the function */
	{
	  cw->Wtype=W_FUNCPARAM;  P("W_FUNCPARAM ");/* function's parameter */
	  c->nparam++;
	  continue; 
	}

      if (cw0&&(strcmp(cw0->Name,",")==0))
	/* Cid in a list : assume it is a variable or function name */
	{
	  cw->Wtype=W_NAME;  P("W_NAME ");
	  c->nvar++;
	  continue;
	}

      if (cw->next&&(strcmp(cw->next->Name,"(")==0))
	/* Next word is "(" : assume it is a variable or function name */
	{
	  cw->Wtype=W_NAME;  P("W_NAME ");
	  c->nvar++;
	  continue;
	}

      if ((cw->next)&&((strcmp(cw->next->Name,"*")==0)||
		       ((!Is_Ckeyword(cw->next->Name))&&
			(!Is_Canydatatype(cw->next->Name))&&(IsStringCid(cw->next->Name)))
		       ))
	/* If next word is a name or *, assume current one is an unknown type and add it. */
	{
	  cw->Wtype=W_USERDATATYPE;  P("W_USERDATATYPE ");
	  Add_Cuserdatatype(w);
	  if (c->Itype==I_FUNC_IN) c->ndatatype++;
	  continue;
	}

      /* Don't know what it is.
	 End analysis if the instruction is already interpreted.
      */
      if (c->Itype!=I_UNDEFINED) 
	{
	  for (cw; cw && (strcmp(cw->Name,",")!=0); cw0=cw,cw=cw->next);
	  if (cw) continue; else goto EndofPass1;
	}
      
      Error("Parse error : I don't know how to interpret C identifier \"%s\"",w);	    
      continue;
      
    } /* end of for (cw=c->wfirst,...) */
  

  if (cw0&&((cw0->Wtype==W_NAME)|| 
	    ( (strcmp(cw0->Name,";")==0)&&(cw0->previous)&&(cw0->previous->Wtype==W_NAME))))
    {
      /* Last word was name : word cw0 is a simple variable definition
      */
      if (c->Itype==I_UNDEFINED) 
	{
	  c->Itype=I_VARDECL; P("I_VARDECL ");
	}
    }
  
 EndofPass1:

  P("\n");

  if (c->Itype==I_UNDEFINED) 
    /* First pass couldn't interpret the instruction : assume it does not contain
       anything interesting 
    */
    return(0);


  if (c->Itype==I_FUNC_IN)
    Error("Parse error : cannot interpret instruction \"%s\" as a function (missing right parenthesis)",
	  c->phrase);	    

  if ((ISCI_FUNCTION(c))&&(c->nparam<0))
    Error("Parse error : cannot interpret instruction \"%s\" as a function (nparam<0 : function's entry not found)",
	  c->phrase);	        
 

  /* REMPLACER LE COMPTAGE PAR L'IDENTIFICATION PARAMETRE=VARIABLE */
  P("Number of parameters=%d  of variables=%d  of datatype=%d \n",c->nparam,c->nvar,c->ndatatype); 

  if (c->nparam>=0)
    /* A function has been found */
    {
      if (c->nparam<c->nvar)
	Error("Parse error : cannot interpret instruction \"%s\" as a function (only %d parameter(s) for %d variable(s)).\nTip : check declaration of last function",c->phrase,c->nparam,c->nvar);	    
      if (c->nparam>c->nvar) 
	return(1);
      else 
	return(0);
    }
  return(0);
}

/*~~~~~~~~~~ 
  Set types in v->Stype and v->Ctype from cw, assuming this word contains a type.
  ~~~~~~~~~~
*/
#ifdef __STDC__
void SetType(Variable *v, Cinstruction *c, Cword *cw)
#else
void SetType(v,c,cw)
Variable *v;
Cinstruction *c;
Cword *cw;
#endif

{
  strcat(v->Stype,cw->Name);  
  strcat(v->Stype," ");
  switch(cw->Wtype)
    {
    case W_DATATYPE:
    case W_CMODIFIER:
      if ((v->Ctype != NONE)&&((v->Ctype < SCALARMIN_T)||(v->Ctype > UNSIGNED_T)))
	/* Allow e.g. short int */
	Error("Parse error in \"%s\" : incompatible types in \"%s\"",c->phrase,v->Stype);		    
      if (strcmp(cw->Name,"void")==0)
	v->Ctype=VOID_T;
      else
	/* Have to deal with composite types such as unsigned long int.
	   Beware : not all composite types in C are allowed, but only
	            those listed in SCALAR C types #define (see mwpl_tree.h).
	*/
	{
	  if (strcmp(cw->Name,"unsigned")==0)
	    v->Ctype=UNSIGNED_T;
	  else
	    if (strcmp(cw->Name,"long")==0)
	      {
		if (v->Ctype==UNSIGNED_T) v->Ctype=ULONG_T;
		else 
		  {
		    if (v->Ctype != NONE)
		      Error("Parse error in \"%s\" : unsupported composite long type in \"%s\"",c->phrase,v->Stype);	    
		    v->Ctype=LONG_T;
		  }
	      }
	    else
	      if (strcmp(cw->Name,"short")==0)
		{
		  if (v->Ctype==UNSIGNED_T) v->Ctype=USHORT_T;
		  else 
		    {
		      if (v->Ctype != NONE)
			Error("Parse error in \"%s\" : unsupported composite short type in \"%s\"",c->phrase,v->Stype);	    
		      v->Ctype=SHORT_T;
		    }
		}
	      else
		if (strcmp(cw->Name,"char")==0)
		  {
		    if (v->Ctype==UNSIGNED_T) v->Ctype=UCHAR_T;
		    else 
		      {
			if (v->Ctype != NONE)
			  Error("Parse error in \"%s\" : unsupported composite char type in \"%s\"",c->phrase,v->Stype);	    
			v->Ctype=CHAR_T;
		      }
		  }
		else
		  if (strcmp(cw->Name,"int")==0)
		    {
		      if (v->Ctype==UNSIGNED_T) v->Ctype=UINT_T;
		      else
			if ((v->Ctype!=USHORT_T)&&(v->Ctype!=ULONG_T)&&(v->Ctype!=SHORT_T)&&(v->Ctype!=LONG_T))
			  /* To deal with e.g. unsigned short int */
			  v->Ctype=INT_T;
		    }
		  else 
		    if (strcmp(cw->Name,"float")==0)
		      {
			if (v->Ctype != NONE)
			  Error("Parse error in \"%s\" : unsupported composite float type in \"%s\"",c->phrase,v->Stype);	    
			v->Ctype=FLOAT_T;
		      }
		    else 
		      if (strcmp(cw->Name,"double")==0)
			{
			  if (v->Ctype != NONE)
			    Error("Parse error in \"%s\" : unsupported composite double type in \"%s\"",c->phrase,v->Stype);	    
			  v->Ctype=DOUBLE_T;
			}
	}
      break;
      
    case W_USERDATATYPE:
      if (v->Ctype != NONE)
	Error("Parse error in \"%s\" : incompatible types in \"%s\"",c->phrase,v->Stype);		    
      v->Ctype=USER_T;
      break;
      
    case W_CMWDATATYPE:
      if (v->Ctype != NONE)
	Error("Parse error in \"%s\" : incompatible types in \"%s\"",c->phrase,v->Stype);		    
      v->Ctype=MW2_T;
      break;
    }
}


/*~~~~~~~~~~ 
  Fill the variable <v> using the content of the Cinstruction chain <c>
  ~~~~~~~~~~
*/
#ifdef __STDC__
void FillNewVariable(VarFunc *v, Cinstruction *c)
#else
void  FillNewVariable(v,c)
VarFunc *v;
Cinstruction *c;
#endif

{
  Cword *cw;

  P("\n[FillNewVariable]\n");

  if (c->Itype != I_VARDECL) Error("[FillNewVariable] Invalid Itype=%d\n",c->Itype);
  v->Itype=c->Itype;

  cw=c->wfirst;
  while (1)
    {
      while (cw && !ISCWORD_FULLTYPE(cw))
	{
	  if (cw->Wtype==W_CSTORAGE)
	    {
	      if (v->v->Cstorage[0]!='\0')
		Error("Parse error in \"%s\" : double C storage class \"%s\" and \"%s\"",c->phrase,v->v->Cstorage,cw->Name);
	      strcpy(v->v->Cstorage,cw->Name);	
	    }
	  cw=cw->next;
	}
      if (!cw) break;
      while (ISCWORD_FULLTYPE(cw))
	{
	  strcat(v->v->Ftype,cw->Name);  
	  strcat(v->v->Ftype," ");

	  if (cw->Wtype==W_CPOINTER)
	    v->v->PtrDepth++;
	  else 
	    SetType(v->v, c, cw);
	  cw=cw->next;
	}
      do
	{
	  strcat(v->v->Name,cw->Name);
	  strcat(v->v->Name,"");
	  cw=cw->next;
	} while (cw);
    }
  RemoveTerminatingSpace(v->v->Stype);
  RemoveTerminatingSpace(v->v->Ftype);
}

/*~~~~~~~~~~ 
  Fill parameters of the function <f> using the content of the Cinstruction chain <c>.
  Case of a ANSI function declaration.
  Exemple : int local_func_decl_ansi(signed toto *a, double p) 
  ~~~~~~~~~~
*/

#ifdef __STDC__
void FillParam_Funcdecl_Ansi(VarFunc *f, Cinstruction *c, Cword *cwb)
#else
void FillParam_Funcdecl_Ansi(f,c,cwb)
VarFunc *f;
Cinstruction *c;
Cword *cwb;
#endif

{
  Cword *cw;
  Variable *p;
  int np;

  P("\n[FillParam_Funcdecl_Ansi]\n");

  p=f->param;
  cw=cwb;
  np=0;
  while (1)
    {
      while (cw && !ISCWORD_FULLTYPE(cw)) cw=cw->next;
      if (!cw) break;
      while (ISCWORD_FULLTYPE(cw))
	{
	  strcat(p->Ftype,cw->Name);  
	  strcat(p->Ftype," ");
	  if (cw->Wtype==W_CPOINTER)
	    p->PtrDepth++;
	  else 
	    SetType(p, c, cw);
	  cw=cw->next;
	}
      if (cw->Wtype != W_FUNCPARAM)
	Error("[FillParam_Funcdecl_Ansi] \"%s\" : W_FUNCPARAM expected for Cword '%s' instead of Wtype=%d",
	      c->phrase,cw->Name,cw->Wtype);    
      strcpy(p->Name,cw->Name);

      RemoveTerminatingSpace(p->Stype);
      RemoveTerminatingSpace(p->Ftype);
      
      p=p->next;
      np++;
    }

  if (np != c->nparam)
    Error("[ FillParam_Funcdecl_Ansi] \"%s\" : %d parameters recorded while %d expected",c->phrase,np,c->nparam);
  
}

/*~~~~~~~~~~ 
  Fill parameters of the function <f> using the content of the Cinstruction chain <c>.
  Case of a K&R function declaration.
  Example : void frthre(A,B,noise) Fimage A,B;float *noise;
  ~~~~~~~~~~
*/

#ifdef __STDC__
void FillParam_Funcdecl_KR(VarFunc *f, Cinstruction *c, Cword *cwb)
#else
void FillParam_Funcdecl_KR(f,c,cwb)
VarFunc *f;
Cinstruction *c;
Cword *cwb;
#endif

{
  Cword *cw,*cw0,*cw1,*cwn;
  Variable *p;
  int np,commapassed,nbcomma;

  P("\n[FillParam_Funcdecl_KR]\n");

  p=f->param;
  cwn=cwb;
  np=0;

  /* Seek first variable name */
  while (cwn && (cwn->Wtype!=W_FUNCPARAM)) cwn=cwn->next;
  if (!cwn) Error("[FillParam_Funcdecl_KR] \"%s\" : cannot find first W_FUNCPARAM in parameter's list",c->phrase);     
  do
    /* For each cwn (W_FUNCPARAM), search for the data type */
    {
      cw0=cwn->next;
      if (!cw0)
	Error("[FillParam_Funcdecl_KR] \"%s\" : cannot find type declaration for parameter '%s' (0)",
	      c->phrase,cwn->Name); 
      cw1=cw0->next;
      /*printf("\n**** Search for '%s'\n",cwn->Name);*/
      while (cw1 && (strcmp(cw1->Name,cwn->Name)!=0)) cw1=cw1->next;
      if (!cw1)
	Error("[FillParam_Funcdecl_KR] \"%s\" : cannot find type declaration for parameter '%s' (1)",
	      c->phrase,cwn->Name); 
      /*
	 Go backward and set cw to the first item that may participate to the full type. 
	 Be aware of situations such as "toto(a,b,c) float a,*b,*c;" that require for c to get "float *".
	 Thus, we have to scan types from the word after ')' and to count the number of commas to know 
	 if the '*' is for this variable.
      */
      for (cw=cw1, nbcomma=0; (cw!=cw0)&&(strcmp(cw->Name,")")!=0)&&(strcmp(cw->Name,";")!=0); cw=cw->previous)
	if (strcmp(cw->Name,",")==0) nbcomma++;
      cw=cw->next; 
      commapassed=0; 
      /*printf("Search from '%s' to '%s' with nbcomma=%d\n",cw->Name,cw1->Name,nbcomma);*/
      /* data types are in cw ... cw1 */
      while (cw != cw1->next)
	{
	  if (strcmp(cw->Name,",")==0) 
	    commapassed++;
	  else
	    if (ISCWORD_SCALARTYPE(cw)||((cw->Wtype==W_CPOINTER)&&(commapassed==nbcomma)))
	      {
		/*printf("commapassed=%d. Add '%s' to Ftype\n",commapassed,cw->Name);*/
		strcat(p->Ftype,cw->Name);  
		strcat(p->Ftype," ");
		if (cw->Wtype==W_CPOINTER)
		  p->PtrDepth++;
		else
		  SetType(p, c, cw);
	      }
	  cw=cw->next;
	}
      RemoveTerminatingSpace(p->Stype);
      RemoveTerminatingSpace(p->Ftype);

      strcpy(p->Name,cwn->Name);
      p=p->next;
      np++;
  
      cwn=cwn->next;
      while (cwn && (cwn->Wtype!=W_FUNCPARAM)) cwn=cwn->next;

    } while (cwn);

  if (np != c->nparam)
    Error("[ FillParam_Funcdecl_KR] \"%s\" : %d parameters recorded while %d expected",c->phrase,np,c->nparam);

}

/*~~~~~~~~~~ 
  Fill parameters of the function <f> using the content of the Cinstruction chain <c>.
  Case of a ANSI function prototype.
  ~~~~~~~~~~
*/

#ifdef __STDC__
void FillParam_Funcproto_Ansi(VarFunc *f, Cinstruction *c, Cword *cwb)
#else
void FillParam_Funcproto_Ansi(f,c,cwb)
VarFunc *f;
Cinstruction *c;
Cword *cwb;
#endif

{
  /* NOT IMPLEMENTED */
}

/*~~~~~~~~~~ 
  Fill parameters of the function <f> using the content of the Cinstruction chain <c>.
  Case of a K&R function prototype.
  ~~~~~~~~~~
*/

#ifdef __STDC__
void FillParam_Funcproto_KR(VarFunc *f, Cinstruction *c, Cword *cwb)
#else
void FillParam_Funcproto_KR(f,c,cwb)
VarFunc *f;
Cinstruction *c;
Cword *cwb;
#endif

{
  /* NOT IMPLEMENTED */
}

/*~~~~~~~~~~ 
  Fill the function <f> (and possibly the followings)  using the content of the Cinstruction chain <c>.
  Set also C->mfunc is <f> is the module's function .
  ~~~~~~~~~~
*/
#ifdef __STDC__
void FillNewFunction(VarFunc *f, Cinstruction *c)
#else
void  FillNewFunction(f,c)
VarFunc *f;
Cinstruction *c;
#endif

{
  Cword *cw,*cwfn;
  Variable *p,*p0;
  int np;

  P("\n[FillNewFunction] nparam=%d\n",c->nparam);
  f->Itype=c->Itype;

  /* Allocate the chain of parameters */
  for (np=1, p0=NULL; np<=c->nparam; np++)
    {
      p=new_variable();
      if (!f->param) f->param=p;
      else p0->next=p;
      p->previous=p0;
      p0=p;
    }
  
  np=0;

  /* First, seek for function's name so that we can set cwfn */
  cwfn=NULL;
  for (cw=c->wfirst; cw; cw=cw->next)
    if (cw->Wtype==W_FUNCNAME)
      {
	strcpy(f->v->Name,cw->Name);	  
	cwfn=cw;
	break;
      }
  if (!cwfn) Error("[FillNewFunction] function's name not found in \"%s\"",c->phrase);

  /* Seek for C storage of function */
  for (cw=c->wfirst; cw!=cwfn; cw=cw->next)
    {
      if (cw->Wtype==W_CSTORAGE)
	{
	  if (f->v->Cstorage[0]!='\0')
	    Error("Parse error in \"%s\" : double C storage class \"%s\" and \"%s\"",c->phrase,f->v->Cstorage,cw->Name);
	  strcpy(f->v->Cstorage,cw->Name);	
	}
      if (ISCWORD_FULLTYPE(cw))
	{
	  strcat(f->v->Ftype,cw->Name);  
	  strcat(f->v->Ftype," ");
	  if (cw->Wtype==W_CPOINTER)
	    f->v->PtrDepth++;
	  else 
	    SetType(f->v, c, cw);
	}
    }
  RemoveTerminatingSpace(f->v->Stype);
  RemoveTerminatingSpace(f->v->Ftype);

  /* Check for main function */
  if ( ((f->Itype==I_FUNCDECL_ANSI)||(f->Itype==I_FUNCDECL_KR)) &&
       (strcmp(f->v->Name,module_name)==0) )
    /* This is the main function */
    {
      if (C->mfunc) Error("Duplicate main function in \"%s\"\n",c->phrase);
      C->mfunc=f;
    }

  
  if (c->nparam == 0) return; /* If the function doesn't contain any parameter, we have finished */
  if (!cwfn->next) 
    Error("[FillNewFunction] \"%s\" : no Cword after function's name while nparam=%d",c->phrase,c->nparam);

  switch(f->Itype)
    {
    case I_FUNCDECL_ANSI:
      FillParam_Funcdecl_Ansi(f,c,cwfn->next);
      break;

    case I_FUNCDECL_KR:
      FillParam_Funcdecl_KR(f,c,cwfn->next);
      break;

    case I_FUNCPROTO_ANSI:
      FillParam_Funcproto_Ansi(f,c,cwfn->next);
      break;

    case I_FUNCPROTO_KR:  
      FillParam_Funcproto_KR(f,c,cwfn->next);
      break;

    default:
      Error("[FillNewFunction] \"%s\" : not a function Itype=%d\n",c->phrase,c->Itype);
    }

}

/*~~~~~~~~~~ 
  Get from input module's file <fs> the next instruction and set the corresponding structure.
  Recognized instructions :
  - Declarations of 
    + global variables;
    + functions.
  Copy C body from <fs> to <fm> and change variable / function declarations.
  Return 1 if one instruction found (even if not recorded in C), 0 elsewhere (end of file)

  Assumptions :
  - Enter in the beginning of a line.
  ~~~~~~~~~~
*/
#ifdef __STDC__
int GetNextInstruction(void)
#else
int GetNextInstruction()
#endif

{
  char s[STRSIZE];
  char w[TREESTRSIZE];
  long lb,lbeg,lend;
  int i,i0;
  int m0,m;
  Cinstruction *c,*c0,*cfirst;
  Cword *cw,*cw0;
  VarFunc *f,*f0;

  if (!fs) return(0);
  cfirst=c0=NULL;
  lbeg=-1;

  /*===== Build the Cinstruction chain =====*/

 ContinueInstruction:
  if ((getinstruction(s,&lb,&lend)==EOF)&&(s[0]=='\0')) 
    {
      if (c0 && (c0==c)) Error("Parse error : unexpected end of file while decoding a block of instructions.\nTip : check if there is no missing '}' nor missing declaration of parameter in a function.");	
      return(0);
    }
  if (lbeg==-1) lbeg=lb; /* To get the beginning of instruction with continuing inst. */

  i0=0;
  cw=NULL;
  c=new_cinstruction();
  if (!cfirst) cfirst=c;
  strcpy(c->phrase,s);
  if (c0) /* c = next instruction to continue c0 */
    {
      c0->next=c;
      c->previous=c0;
      c0=NULL;
    }
  while ((i=getword(&s[i0],w))!=0)
    {
      cw0=cw;
      cw=new_cword();
      if (!c->wfirst) c->wfirst=cw;
      else 
	{
	  cw0->next=cw;
	  cw->previous=cw0;
	}
      
      strcpy(cw->Name,w);
      i0+=i;
    }
  if (AnalyseInstruction(c)==1) 
    /* the instruction has to be continued */
    {
      c0=c;
      P("*** Continue Instruction \n");
      goto ContinueInstruction;
    }

  /*===== Complete the Cbody with the content of the Cinstruction chain =====*/
  
  c=cfirst;
  merge_cinstruction(c);

  switch(c->Itype)
    {
    case I_UNDEFINED:
    case I_CDECLARETYPE:
      /* No an instruction to be recorded in C */
      return(1);
      
    case  I_VARDECL:
      f=new_varfunc();
      for (f0=C->varfunc;f0&&f0->next;f0=f0->next);
      if (!f0) C->varfunc=f;
      else f0->next=f;
      f->previous=f0;      
      FillNewVariable(f,c);
      f->l0=lbeg; f->l1=lend;
      delete_cinstruction(c);
#ifdef DEBUG
      dump_varfunc(f);  
#endif
      break;
      
    case I_FUNCDECL_ANSI:
    case I_FUNCDECL_KR:
    case I_FUNCPROTO_ANSI:
    case I_FUNCPROTO_KR:
      f=new_varfunc();
      for (f0=C->varfunc;f0&&f0->next;f0=f0->next);
      if (!f0) C->varfunc=f;
      else f0->next=f;
      f->previous=f0;      
      FillNewFunction(f,c);
      f->l0=lbeg; f->l1=lend;
      delete_cinstruction(c);
#ifdef DEBUG
      dump_varfunc(f);  
#endif
      break;


    default:
      Error("Parse error : unexpected type of instruction Itype=%d",c->Itype);
      
    }
  return(1);
} 

