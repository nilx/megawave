/*~~~~~~~~~~~~~~ mwplight - The MegaWave2 Light Preprocessor ~~~~~~~~~~~~~~~~~~~

 analyse header statements and fill the header tree

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

/*~~~~~~~~~~ 
  Get from sentence <s> an header statement of the form "<name> = { <value> }"
~~~~~~~~~~
*/
#ifdef __STDC__
void GetHeaderStatement(char *s, char *name, char *value)
#else
void GetHeaderStatement(s,name,value)
char *s;
char *name;
char *value;
#endif

{
  char v[STRSIZE];  

  *name=*v=*value='\0';

  if ((sscanf(s,"%[a-z]=%[^\n]",name,v)!=2)&&
      (sscanf(s,"%[a-z] =%[^\n]",name,v)!=2)&&
      (sscanf(s,"%[a-z]= %[^\n]",name,v)!=2)&&
      (sscanf(s,"%[a-z] = %[^\n]",name,v)!=2))
    Error("(sscanf) Header statement \"%s\" does not follow the syntax \"<name> = {<value>}\".",s);

  if (removebraces(v,value)!=1)
    Error("(v='%s') Header statement \"%s\" does not follow the syntax \"<name> = {<value>}\".",v,s);

  
} 

/*~~~~~~~~~~ 
  Get from usage value <s> the n-th usage specification arg_n "string_n" and put it in <arg>, <str>.
  Return 1 if found, 0 elsewhere (no more usage spec.)
~~~~~~~~~~
*/
#ifdef __STDC__
int GetUsageSpec(char *s, int n, char *arg, char *str)
#else
int GetUsageSpec(s,n,arg,str)
char *s;
int n;
char *arg;
char *str;
#endif

{
  int i,i0,i1,l,q0,q1;
  char us[STRSIZE];

  *arg=*str='\0';
  if (n<=0) Error("Invalid n-th index n=%d\n",n);

  /* n-th spec. begins after the (n-1)-th '",' or '" ,' or '},' or '} ,'
     the two last cases corresponding to an usage spec. following an optional argument list.
   */
  i0=0;
  for (i=1;i<n;i++) 
    {
    notasep0:
      while ((s[i0]!='\0')&&(s[i0]!=',')) i0++;
      if ((i0<2)||(s[i0]=='\0'))  return(0);
      if ( (s[i0-1]!='"')&&(s[i0-1]!='}')&& 
	   (((s[i0-1]!=' ')||((s[i0-2]!='"')&&(s[i0-2]!='}')))))					       
	{
	  i0++;
	  goto notasep0; 
	}
      
      i0++;
      if (s[i0]=='\0') return(0); 
    }
  while ((s[i0]!='\0')&&(s[i0]==' ')) i0++;
  if (s[i0]=='\0') return(0); 

  if (s[i0]=='{')
    {
      /* Open optional arguments list */
      if (inside_optionarg<0)
	Error("Invalid '{' in the usage : no more than one optional arguments list is allowed");
      if (inside_optionarg>=1)
	Error("Invalid '{' in the usage : already inside an optional arguments list");
      inside_optionarg=1;
      i0++;
     }
  
  /* seek the right ',' or end of <s> */
  i1=i0;
 notasep1:
  while ((s[i1]!='\0')&&(s[i1]!=',')&&(s[i1]!='}')) i1++;
  if ((s[i1-1]!='"')&&(((s[i1-1]!=' ')||(s[i1-2]!='"')))) 
    {
      i1++;
      goto notasep1;
    }
  if (s[i1]=='}') /* Close optional arguments list */
    {
      if (inside_optionarg!=1)
	Error("Invalid '}' in the usage : not inside an optional arguments list");
      inside_optionarg=2; /* This n-th arg is the last optional argument */
    }
  i1--;

  /* now the n-th usage spec. is in s[i0]...s[i1] */
  l=i1-i0+1;
  strncpy(us,&s[i0],l); us[l]='\0';

  /* seek the left '"' */
  q0=0;
  while ((q0<l-1)&&(us[q0]!='"')) q0++;  
  q0++;
  if (q0>=l-1) Error("Invalid usage specification \"%s\" : left string delimiter \" not found",us);
  q1=q0;
  while ((q1<l)&&(us[q1]!='"')) q1++;  
  if (us[q1]!='"') 
    Error("Invalid usage specification \"%s\" : right string delimiter \" not found",us);
  q1--;

  for (i=0;i<q0-1;i++) arg[i]=us[i];
  arg[i]='\0';
  for (i=q0;i<=q1;i++) str[i-q0]=us[i];
  str[i-q0]='\0';

  removespaces(arg);

#ifdef DEBUG
  printf("[GetUsageSpec] arg='%s' str='%s'\n",arg,str);
#endif

  return(1);
} 


/*~~~~~~~~~~ 
  Get from arg usage value <s> (as returned in <arg> by GetUsageSpec()) the arg usage specification
  following the syntax '<left> -> <right>' or '<left> <- <right>'.
  Return READ if the arrow is -> (input arg) or WRITE if the arrow is <- (output arg).
~~~~~~~~~~
*/
#ifdef __STDC__
int GetArgUsageSpec(char *s, char *left, char *right)
#else
int GetArgUsageSpec(s,left,right)
char *s;
char *left;
char *right;
#endif

{
  int i,rw;

  *left=*right='\0';

  /* seek the arrow */
  i=1;

 notanarrow:
  while ((s[i]!='\0')&&(s[i]!='-')&&(s[i]!='<')) i++;
  if ((s[i]=='\0')||(s[i+1]=='\0'))
    Error("Invalid argument \"%s\" : no arrow -> or <- found",s);

  if ((s[i]=='-')&&(s[i+1]=='>')) rw=READ;
  else 
    if ((s[i]=='<')&&(s[i+1]=='-')) rw=WRITE;    
    else
      {
	i++;
	goto notanarrow;
      }
  
  
  strncpy(left,s,i);  left[i]='\0';
  strcpy(right,&s[i+2]);  

  removespaces(left);
  removespaces(right);

#ifdef DEBUG
  printf("[GetArgUsageSpec] left='%s' right='%s' rw=%d\n",left,right,rw);
#endif

  return(rw);
} 


/*~~~~~~~~~~ 
  Analyse right part of the arg usage value (as returned in <right> by GetArgUsageSpec()).
  From <s> get <Cid> (needed) and optionally <ictype>,<min>,<max>.
~~~~~~~~~~
*/
#ifdef __STDC__
void AnalyseRightArgUsage(char *s, char *Cid, int *ictype, char *min, char *max)
#else
void AnalyseRightArgUsage(s,Cid,ictype,min,max)
char *s;
char *Cid;
int *ictype;
char *min;
char *max;
#endif

{
  int i,j;

  i=getCid(s,Cid);
  if (i==0) Error("C_id not found in \"%s\"",s);
  *ictype=getInterval(&s[i],min,max,&j);

  /* Check that nothing is following */
  if (*ictype==NONE)
    {
      if (s[i]!='\0')
	Error("Invalid field following C_id=\"%s\" in \"%s\"\nExpecting void or optional interval [Min,Max].",Cid,s);
    }
  else
    {
      if (s[i+j]!='\0')
	Error("Invalid field \"%s\" following interval (%s,%s) of C_id=\"%s\" in \"%s\".",&s[j],min,max,Cid,s);	
    }
} 

/*~~~~~~~~~~ 
  Analyse the default input value following the syntax <s> = "[H_id=Val]".
  Return 1 (and fill <hid>,<val>) if the input <s> follows this syntax, 0 or generate error elsewhere.
~~~~~~~~~~
*/
#ifdef __STDC__
int GetDefaultInputValue(char *s, char *hid, char *val)
#else
int GetDefaultInputValue(s, hid, val);
char *s;
char *hid;
char *val;
#endif

{
  int i,l;

  removespaces(s);
  if (s[0]!='[') return(0);
  l=strlen(s);
  if (s[l-1]!=']') 
    Error("Invalid default value field in \"%s\".\nThis string does not end with ']'.",s);
  s[l-1]='\0';
  
  if (sscanf(s,"[%[^=]=%[^\n]",hid,val)!=2)
    Error("Invalid default value field in \"%s]\".\nThis string does not follow the syntax [H_id=Val].",s);     

  removespaces(hid);
  removespaces(val);
  /*  printf("[GetDefaultInputValue] hid=\"%s\" val=\"%s\"\n",hid,val);*/

  return(1);
}


/*~~~~~~~~~~ 
  Analyse left part of the arg usage value (as returned in <left> by GetArgUsageSpec()).
  From <s> get <atype>, <flg>, <hid>, <val>.
~~~~~~~~~~
*/
#ifdef __STDC__
void AnalyseLeftArgUsage(char *s, int *atype, char *flg, char *hid, char *val)
#else
void AnalyseLeftArgUsage(s, atype, flg, hid, val);
char *s;
int *atype;
char *flg;
char *hid;
char *val;
#endif

{
  char t[STRSIZE];

  /* Some possibilities may be determined from the first letter */
  if (s[0]=='\'')
    /* Must be an option */
    {
      if (s[2]!='\'') 
	Error("Invalid option field in \"%s\".\nThird letter : expecting \"'\" character instead of \"%c\".\nPossible error : user's options of more than one character are not allowed.",s,s[2]);
      if (inside_optionarg>0)
	Error("Invalid option occurrence in \"%s\".\nOptions are not allowed inside an optional argument list.",s);


      *atype = OPTION;
      *flg=s[1];
      if ((s[3]!=':')&&(s[3]!='\0'))
	Error("Invalid option field in \"%s\".\nFourth letter : expecting \":\" or end of char instead of \"%c\".",s,s[3]);
      if (s[3]==':')
	/* Option with input value */
	{
	  strcpy(t,&s[4]);
	  if (GetDefaultInputValue(t,hid,val)!=1)
	    /* Option with NO default input value :
	       remaining text is assumed to be hid only 
	    */
	    strcpy(hid,t);
	}
      return;
    }

  if (s[0]=='.')
    /* Must be a variable argument */
    {
      if ((s[1]!='.')||(s[2]!='.')||(s[3]!='\0'))
	Error("Invalid variable argument field (...) in \"%s\".",s);
      if (inside_optionarg>0)
	Error("Invalid variable argument occurrence in \"%s\".\nVariable arguments are not allowed inside an optional argument list.",s);
      *atype = VARARG;
      return;
    }  

  if (strcmp(s,"notused")==0)
    /* Unused argument */
    {
      if (inside_optionarg>0)
	Error("Invalid notused occurrence in \"%s\".\nUnused arguments are not allowed inside an optional argument list.",s);
      *atype = NOTUSEDARG;
      return;
    }

  /* Now, only two possibilities : needed or optional arg, the only difference being that
     with needed arg input value is not allowed.
  */
  strcpy(t,s);
  if (GetDefaultInputValue(t,hid,val)==1)
    {
      if (inside_optionarg<=0)
	Error("Invalid default value occurrence in \"%s\".\nDefault values are not allowed with needed arguments.",s);
      *atype = OPTIONARG;
      return;
    }
  /* argument with NO default input value :
     remaining text is assumed to be hid only 
  */
  strcpy(hid,s);
  if (inside_optionarg>0)
    *atype = OPTIONARG;
  else
    *atype = NEEDEDARG;
}


/*~~~~~~~~~~ 
  Add name statement to the header tree
~~~~~~~~~~
*/
#ifdef __STDC__
void AddNameStatement(char *value)
#else
void AddNameStatement(value)
char *value;
#endif

{

  if (H->Name[0]!='\0') Error("Duplicate name statement (previous name is \"%s\") !",H->Name);
  if (strlen(value)>=TREESTRSIZE)
    Error("Name \"%s\" exceeds limit of %d car.",value,TREESTRSIZE-1);
  strcpy(H->Name,value);

  /* Check whether or not this name matches the file name */
  if (strcmp(H->Name,module_name) != 0)
    Error("Invalid name. The filename imposes the name to be \"%s\"",
	  module_name);

}


/*~~~~~~~~~~ 
  Add author statement to the header tree
~~~~~~~~~~
*/
#ifdef __STDC__
void AddAuthorStatement(char *value)
#else
void AddAuthorStatement(value)
char *value;
#endif

{
  char v[STRSIZE];

  if (H->Author[0]!='\0') Error("Duplicate author statement (previous author is \"%s\") !",H->Author);
  if (getenclosedstring(value,v)!=1)
    Error("Author statement field must be enclosed by quotation marks");

  if (strlen(v)>=TREESTRSIZE)
    Error("Author statement field \"%s\" exceeds limit of %d car.",v,TREESTRSIZE-1);

  strcpy(H->Author,v);
}

/*~~~~~~~~~~ 
  Add version statement to the header tree
~~~~~~~~~~
*/
#ifdef __STDC__
void AddVersionStatement(char *value)
#else
void AddVersionStatement(value)
char *value;
#endif

{
  char v[STRSIZE];

  if (H->Version[0]!='\0') Error("Duplicate version statement (previous version is \"%s\") !",H->Version);

  if (getenclosedstring(value,v)!=1)
    Error("Version statement field must be enclosed by quotation marks");

  if (strlen(v)>=TREESTRSIZE)
    Error("Version statement field \"%s\" exceeds limit of %d car.",v,TREESTRSIZE-1);

  strcpy(H->Version,v);

}


/*~~~~~~~~~~ 
  Add function statement to the header tree
~~~~~~~~~~
*/
#ifdef __STDC__
void AddFunctionStatement(char *value)
#else
void AddFunctionStatement(value)
char *value;
#endif

{
  char v[STRSIZE];

  if (H->Function[0]!='\0') Error("Duplicate function statement (previous function is \"%s\") !",H->Function);

  if (getenclosedstring(value,v)!=1)
    Error("Function statement field must be enclosed by quotation marks");

  if (strlen(v)>=TREESTRSIZE)
    Error("Function statement field \"%s\" exceeds limit of %d car.",v,TREESTRSIZE-1);

  strcpy(H->Function,v);
}


/*~~~~~~~~~~ 
  Add labo statement to the header tree
~~~~~~~~~~
*/
#ifdef __STDC__
void AddLaboStatement(char *value)
#else
void AddLaboStatement(value)
char *value;
#endif

{
  char v[STRSIZE];

  if (H->Labo[0]!='\0') Error("Duplicate labo statement (previous labo is \"%s\") !",H->Labo);
  if (getenclosedstring(value,v)!=1)
    Error("Labo statement field must be enclosed by quotation marks");

  if (strlen(v)>=TREESTRSIZE)
    Error("Labo statement field \"%s\" exceeds limit of %d car.",v,TREESTRSIZE-1);

  strcpy(H->Labo,v);

}


/*~~~~~~~~~~ 
  Add group statement to the header tree
~~~~~~~~~~
*/
#ifdef __STDC__
void AddGroupStatement(char *value)
#else
void AddGroupStatement(value)
char *value;
#endif

{
  char v[STRSIZE];

  if (H->Group[0]!='\0') Error("Duplicate group statement (previous group is \"%s\") !",H->Group);
  if (getenclosedstring(value,v)!=1)
    Error("Group statement field must be enclosed by quotation marks");

  if (strlen(v)>=TREESTRSIZE)
    Error("Group statement field \"%s\" exceeds limit of %d car.",v,TREESTRSIZE-1);

  strcpy(H->Group,v);

  /* Check whether or not this group matches group_name */
  if (strcmp(H->Group,group_name) != 0)
    Error("Invalid group. The directory hierarchy imposes the group to be \"%s\"",
	  group_name);

}


/*~~~~~~~~~~ 
  Add usage statement to the header tree
~~~~~~~~~~
*/
#ifdef __STDC__
void AddUsageStatement(char *value)
#else
void AddUsageStatement(value)
char *value;
#endif

{
  int n;
  char arg[STRSIZE];
  char str[STRSIZE];  
  char left[STRSIZE];
  char right[STRSIZE];
  Arg *a,*a0;

  inside_optionarg=0;

  n=1;
  a0=NULL;
  while (GetUsageSpec(value,n,arg,str)==1) 
    {
      if (strlen(str)>=TREESTRSIZE)
	Error("Comment's string \"%s\" in usage exceeds maximum size length of %d char.",
	      str,TREESTRSIZE-1);
      
      a=new_arg();
      if (!a) Error("Not enough memory for a new usage statement");
      if (!a0) H->usage=a;
      else 
	{
	  a0->next=a;
	  a->previous=a0;
	}
      a0=a;

      /* Set arg fields Cmt, IOtype */
      strcpy(a->Cmt,str); 
      a->IOtype=GetArgUsageSpec(arg,left,right);      

      /* Analyse right portion of the arg usage description.
	 Set the following arg fields :
	 ICtype, C_id, Min, Max
      */
      AnalyseRightArgUsage(right,a->C_id,&(a->ICtype),a->Min,a->Max);

      /* Analyse left portion of the arg usage description.
	 Set the following arg fields :
	 Atype, Flag, H_id, Val
      */
      AnalyseLeftArgUsage(left,&(a->Atype),&(a->Flag),a->H_id,a->Val);
      
      /* Remaining arg to set : Ctype, Vtype.
	 To be completed when the C body would be parsed.
       */

#ifdef DEBUG
      /* Dump arg content for debug */
      dump_arg(a);
#endif

      n++;
      if (inside_optionarg==2) inside_optionarg=-1; /* does not allow new optional arguments list */
    }
}



/*~~~~~~~~~~ 
  Analyse the current header statement and add it to the header tree
~~~~~~~~~~
*/
#ifdef __STDC__
void AnalyseHeaderStatement(char *argclass, char *value)
#else
void AnalyseHeaderStatement(argclass,value)
char *argclass;
char *value;
#endif

{
#ifdef DEBUG
  printf("[AnalyseHeaderStatement] argclass='%s' value='%s'\n",argclass,value);
#endif

  if (strcmp(argclass,"name")==0) AddNameStatement(value);
  else
    if (strcmp(argclass,"author")==0) AddAuthorStatement(value);
    else
      if (strcmp(argclass,"version")==0) AddVersionStatement(value);
      else
	if (strcmp(argclass,"function")==0) AddFunctionStatement(value);
	else
	  if (strcmp(argclass,"labo")==0) AddLaboStatement(value);
	  else
	    if (strcmp(argclass,"group")==0) AddGroupStatement(value);
	    else
	      if (strcmp(argclass,"usage")==0) AddUsageStatement(value);
	      else
		Error("Unimplemented \"%s\" statement in the module's header",
		      argclass);
}

