/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   make_type_conv.c
   
   Vers. 1.5
   Author : Jacques Froment
   Command to generate extended type conversion functions :

   - Create OCNAME source file : read INAME to figure out what are the basic
     conversion functions already implemented, and write in OCNAME all other
     possible conversion functions using the basic ones. 

   - Create OINAME include file, where are listed in the variables 
     mw_type_conv_out and mw_type_conv_in all possible conversions
     from one memory type to another one.

   Main changes :
   v1.5 (JF): added include <stdlib> for exit() (Linux 2.6.12 & gcc 4.0.2)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "native_ftype.h"

#define INAME "basic_conv.c"
#define OINAME "../include/type_conv.h"
#define OCNAME "type_conv.c"

#define MAXADJ 10
#define TNAME 20
#define MAXGN 100

#define WHITE 2
#define GRAY 1
#define BLACK 0

#define INFTY 30000

/* Node structure used to compute minimal path between two internal types :
   it lists in <adj> for each internal type <type> the types for which a 
   conversion function already exists (in the file INAME).
*/

typedef struct node {
  char type[TNAME];              /* Name of the internal types */
  struct node *adjacent[MAXADJ]; /* Pointers to types for which a conversion
				    function already exists */
  struct node *next;             /* Pointer to the next internal type */

  /* The following useful for minimal path computation only. Contents
     vary with call to GetAccessibleNodes().
  */
  unsigned char color;           /* Color of the node (WHITE,GRAY,BLACK) */
  struct node *father;           /* Father of the node */
  int distance;                  /* Distance from the root */
  
} *Node;

/* Global variables */

Node G;                              /* Graph or input node */
Node GN[MAXGN];                      /* List of gray nodes (FIFO) */


/*---------- GRAPH functions ----------*/

/* Creates a new node structure */

Node NewNode()
{
  Node g,ad;
  int i;

  if(!(g = (Node) (malloc(sizeof(struct node)))))
    {
      fprintf(stderr,"Not enough memory.\nAbort.\n");
      exit(1);
    }
  g->type[0]='\0';
  for (i=0;i<MAXADJ;i++) g->adjacent[i]=NULL;
  g->next=NULL;
  return(g);
}


/* Return the node of name (type) a in G, NULL if not found
*/

Node GetNode(a)

char *a;

{
  Node g;

  for (g=G; g; g=g->next)
      if (strcmp(g->type,a)==0) return(g);
  return(NULL);
}

/* Return the last node in G, NULL G is empty.
*/

Node GetLastNode()

{
  Node g;

  for (g=G; g && (g->next); g=g->next);
  return(g);
}


/* Add in G the node of name (type) a, if this one does not already exist.
   Return the node pointer to a.
*/

Node AddNode(a)

char *a;

{
  Node g,l;

  g=GetNode(a);
  if (g==NULL)    /* This node does not already exist : create it. */
    {
      /*printf("AddNode %s\n",a);*/
      g=NewNode();
      strcpy(g->type,a);
      l=GetLastNode();
      if (l==NULL) G=g;
      else l->next=g;
    }
  return(g);
}


/* Add in G the node of name (type) a, if this one does not already exist,
   with conversion to type b. Return the node pointer to b.
*/

Node AddNodeAndConv(a,b)

char *a,*b;

{
  Node A,B;
  int i;

  A=AddNode(a);
  for (i=0; (i<MAXADJ)&&(A->adjacent[i]!=NULL)&&
	 (strcmp(A->adjacent[i]->type,b)!=0); i++);
  if ((i<MAXADJ)&&(A->adjacent[i]!=NULL)&&
      (strcmp(A->adjacent[i]->type,b)==0))
    /* The conversion a->b already exists */
    {
      fprintf(stderr,"Warning : file '%s' seems to contain multiple conversions from %s to %s !\n",INAME,a,b);
      return;
    }
  if (i>=MAXADJ)
    {
      fprintf(stderr,"Too many conversions for the type %s !\nPlease increment the constant MAXADJ.\n",a);
      exit(1);
    }
  /* Add conversion a->b */
  B=AddNode(b);
  A->adjacent[i]=B;
  return(B);
}


/* Print the content of the node pointer g 
*/

void PrintNode(g)

Node g;

{
  int i;

  if (!g) return;	    
  printf("Node '%s'\n",g->type);
  printf(" Adj: ");
  for (i=0; (i<MAXADJ)&&(g->adjacent[i]!=NULL); i++)
    printf("%s ",g->adjacent[i]->type);
  printf("\n");

  printf("\n");
}

/* Print the content of the graph.
*/

void PrintGraph()

{
  Node g;

  for (g=G;g;g=g->next) PrintNode(g);
}

/* Clear GN */

void ClearGN()

{
  int i;
  for (i=0;i<MAXGN;i++) GN[i]=NULL;
}

/* Push the node s in GN (put it in the last position) */

void PushGN(s)

Node s;

{
  int l;

  for (l=0;(l<MAXGN)&&(GN[l]!=NULL);l++);
  if (l >= MAXGN)
    {
      fprintf(stderr,"Cannot push node in GN : too many gray nodes !\nPlease increment MAXGN.\n");
      exit(1);
    }
  GN[l]=s;
}

/* Pop the first node in GN (FIFO mode). Return NULL if GN is empty. */

Node PopGN()

{
  Node s;
  int i;

  s=GN[0];
  for (i=0;(i<MAXGN-1)&&(GN[i+1]!=NULL);i++) GN[i]=GN[i+1];
  GN[i]=NULL;
  return(s);
}

/* Compute the minimal path tree in G from the node s to any other node. 
*/

void GetAccessibleNodes(s)

Node s;

{
  Node u,v;
  int i;

  for (u=G; u; u=u->next)  
    {
      u->color=WHITE;
      u->distance=INFTY;
      u->father=NULL;
      }
  s->color=GRAY;
  s->distance=0;
  ClearGN();
  PushGN(s);
  while (GN[0]!=NULL)
    {
      u=GN[0];
      for (i=0;(i<MAXADJ)&&((v=u->adjacent[i])!=NULL); i++)      
	if (v->color==WHITE)
	  {
	    v->color=GRAY;
	    v->distance = u->distance + 1;
	    v->father = u;
	    PushGN(v);
	  }
      PopGN();
      u->color=BLACK;
    }
}

/* Put in p the list of intermediate types to go from node s to node v, using
   a minimal path in the graph G (must be called after GetAccessibleNodes(s)).
   Return 0 if no path exists, 1 elsewhere.
*/

int GetMinimalPath(p,s,v)

char *p;
Node s,v;

{
  int ret;
  char T[TNAME+1];

  if (s==v) {  sprintf(T,"%s ",s->type); strcat(p,T); }
  else
    {
      if (v->father==NULL) return(0);
      else 
	{
	  ret=GetMinimalPath(p,s,v->father);
	  if (ret==0) return(0);
	  sprintf(T,"%s ",v->type); 
	  strcat(p,T);
	}
    }
  return(1);
}

/*---------- I/O functions ----------*/

int SetUpper(s)
 
char *s;
 
{
  if (islower(s[0]) != 0) s[0]=toupper(s[0]); 
}



/* Print in the file fp the code corresponding to the begin of typein
*/

void PrintBeginTypeIn(fp,typein)

FILE *fp;
char *typein;

{
  fprintf(fp,"  if (strcmp(typein,\"%s\") == 0)\n    {\n",typein);
}

/* Print in the file fp the code corresponding to the end of typein
*/

void PrintEndTypeIn(fp,typein)

FILE *fp;
char *typein;

{
  fprintf(fp,"     return(NULL); /* No conversion function ! */\n");
  fprintf(fp,"    } /* End of typein = %s */\n\n",typein);
}

/* Return the number of types in the path <Path>
*/

int GetLengthOfPath(Path)

char *Path;

{
  int i,n=1;
  
  for (i=1;i<strlen(Path)-1;i++)
    if ((Path[i]==' ')&&(Path[i-1]!=' ')) n++;
  return(n);
}


/* Return in <type> the type in the position <p> in <Path> (first position = 1)
*/

void GetTypeInPath(Path,p,type)

char *Path;
int p;
char *type;

{
  int i0,i1,n=1;
  
  i0=0;
  for (i1=1;i1<strlen(Path);i1++)
    if ((Path[i1]==' ')&&(Path[i1-1]!=' '))
      {
	if (n==p) 
	  { 
	    strncpy(type,&Path[i0],i1-i0); 
	    type[i1-i0]='\0'; 
	    return;
	  }
	n++;
	i0=i1+1;
      }
  type[0]='\0';
}

/* Print in the file fp the code to get <typeout> using the path <Path>
*/

void PrintPath(fp,typeout,Path)

FILE *fp;
char *typeout;
char *Path;

{
  int i,l;
  char type0[TNAME],type1[TNAME];
  char Type0[TNAME],Type1[TNAME];
  char Input0[TNAME],Input1[TNAME];
  char Delete[TNAME];

  fprintf(fp,"     if (strcmp(typeout,\"%s\") == 0)\n       {\n",typeout);

  l=GetLengthOfPath(Path);
  GetTypeInPath(Path,1,type0);
  strcpy(Type0,type0);
  SetUpper(Type0);
  strcpy(Input1,"mwstruct");
  Delete[0]=Input0[0]='\0';

  for (i=2;i<=l;i++)
    {
      GetTypeInPath(Path,i,type1);
      strcpy(Type1,type1);
      SetUpper(Type1);
      /*
      fprintf(fp,"        if (!(%s = (%s) mw_%s_to_%s((%s) %s))) return(NULL);\n",
	      type1,Type1,type0,type1,Type0,Input1);
      */
      fprintf(fp,"        if (!(%s = (%s) mw_%s_to_%s((%s) %s, NULL))) return(NULL);\n",
	      type1,Type1,type0,type1,Type0,Input1);

      strcpy(type0,type1);
      strcpy(Type0,Type1);
      strcpy(Input1,type1);
      strcpy(Delete,Input0);
      strcpy(Input0,Input1);      
      if ((Delete[0]!='\0')&&(strcmp(Delete,"mwstruct")!=0))
	fprintf(fp,"        mw_delete_%s(%s);\n",Delete,Delete);	
    }
  fprintf(fp,"        return(%s);\n",type0);
  fprintf(fp,"       }\n");
}


/* Print in the file fp the function mw_conv_internal_type()
   which convert any internal type to another one.
*/

void PrintConvInternalType(fp)

FILE *fp;

{
  Node A,B;
  int i;
  char Path[BUFSIZ];

  /* Print function declaration */

  fprintf(fp,"/* Convert any possible internal type to another one. */\n\n");
  fprintf(fp,"void *mw_conv_internal_type(mwstruct,typein,typeout)\n\n");
  fprintf(fp,"void *mwstruct; /* Any type of MegaWave2 structure */\n");
  fprintf(fp,"char *typein;  /* Type of the input <mwstruct> */\n");
  fprintf(fp,"char *typeout; /* Type of the output structure */\n");

  fprintf(fp,"\n{\n");


  /* Print variable declaration */
  for (A=G;A;A=A->next)
    {
      strcpy(Path,A->type);
      SetUpper(Path);
      fprintf(fp,"  %s %s;\n",Path,A->type);
    }
  fprintf(fp,"\n");
  
  /* Print conversions */
  for (A=G;A;A=A->next)
    {
      GetAccessibleNodes(A);
      PrintBeginTypeIn(fp,A->type);
      for (B=G;B;B=B->next)
	if (A!=B)
	  {
	    for (i=0; (i<MAXADJ)&&(A->adjacent[i]!=NULL)&&
		   (A->adjacent[i]!=B); i++);
	    if ((i>=MAXADJ)||(A->adjacent[i]!=B))
	      /* Basic conversion functions A->B does not exist */
	      {
		Path[0]='\0';
		/*printf("Conversion %s -> %s :",A->type,B->type);*/
		if (GetMinimalPath(Path,A,B)!=0)
		  /* Path of length > 2 exists */
		  PrintPath(fp,B->type,Path);
	      }
	    else 
	      /* Basic conversion function already exists */
	      {
		sprintf(Path,"%s %s ",A->type,B->type);
		PrintPath(fp,B->type,Path); /* Path of length 2 */
	      }		
	  }
      PrintEndTypeIn(fp,A->type);
    }

  fprintf(fp,"\n  return(NULL); /* No conversion function ! */\n}\n");
}


/* Return 1 if the path to go from A to B exists (i.e. there is a conversion
   function), or 0 if not.
   Must be called after GetAccessibleNodes(A)
*/

int CheckIfPathExists(A,B)

Node A,B;

{
  int i;
  char Path[BUFSIZ];

  if (A==B) return(0);
  for (i=0; (i<MAXADJ)&&(A->adjacent[i]!=NULL)&&
	 (A->adjacent[i]!=B); i++);
  if ((i>=MAXADJ)||(A->adjacent[i]!=B))
    /* Basic conversion functions A->B does not exist */
    {
      Path[0]='\0';
      return(GetMinimalPath(Path,A,B));
    }
  else 
    /* Basic conversion function already exists */
    return(1);
}

/* Print in the file fp the function _mw_load_etype_to_itype()
   which load any possible external type to any internal type.
*/

void PrintLoadETypeToIType(fp)

FILE *fp;

{
  Node A,B;
  int i;
  char Path[BUFSIZ];
  char TypeA[TNAME];
  char TypeB[TNAME];

  /* Print function declaration */

  fprintf(fp,"\n/* Load any possible external type to any internal type */\n\n");
  fprintf(fp,"void *_mw_load_etype_to_itype(fname,typein,typeout,Type)\n\n");
  fprintf(fp,"char *fname;  /* File name of the external structure (input) */\n");
  fprintf(fp,"char *typein; /* Internal type of the input */\n");
  fprintf(fp,"char *typeout;/* Requested internal type of the output */\n");
  fprintf(fp,"char *Type;   /*  */\n");
  fprintf(fp,"\n{\n");


  /* Print variable declaration */
  for (A=G;A;A=A->next)
    {
      strcpy(Path,A->type);
      SetUpper(Path);
      fprintf(fp,"  %s %s;\n",Path,A->type);
    }
  fprintf(fp,"\n");

  /* Print conversions */
  for (A=G;A;A=A->next)
    {
      GetAccessibleNodes(A);
      PrintBeginTypeIn(fp,A->type);
      strcpy(TypeA,A->type);
      SetUpper(TypeA);      
      fprintf(fp,"     %s = (%s) _mw_%s_load_native(fname,Type);\n",
	      A->type,TypeA,A->type);
      fprintf(fp,"     if (%s==NULL) return(NULL);\n",A->type);
      for (B=G;B;B=B->next)
	if (CheckIfPathExists(A,B)==1)
	  {
	    strcpy(TypeB,B->type);
	    SetUpper(TypeB);      
	    fprintf(fp,"     if (strcmp(typeout,\"%s\") == 0)\n       {\n",
		    B->type);
	    fprintf(fp,"        %s = (%s) mw_conv_internal_type(%s,\"%s\",\"%s\");\n",
		    B->type,TypeB,A->type,A->type,B->type);      
	    fprintf(fp,"        if (%s==NULL) mwerror(FATAL,1,\"Sorry, failure in the conversion from %s to %s does not allow to load the file into a %s structure. Abort.\\n\");\n",B->type,A->type,B->type,B->type);
	    fprintf(fp,"        return(%s);\n",B->type);

	    fprintf(fp,"       }\n",B->type);
	  }
      PrintEndTypeIn(fp,A->type);
    }

  fprintf(fp,"\n  return(NULL); /* No conversion function ! */\n}\n");
}


/* Return the maximal range (order in the array A[]) of the strings in the line
   beginning by the string <a> (that is, the number of strings).
   Return 0 if <a> does not exist.
*/

int get_max_range_array(a,A)

char *a;
char *A[];

{
  int i,j;

  for (i=0; 
       (((A[i]!=NULL)||(A[i+1]!=NULL)) &&
	((A[i]==NULL)||(strcmp(a,A[i])!=0)||((i!=0)&&(A[i-1]!=NULL)))); i++);
  if (A[i]==NULL) return(0);
  /* i = index of a in the array A */

  for (j=i+1; A[j]!=NULL; j++);
  return(j-i-1);
}

/* Put in <b> the string of range (order in A[]) <r> in the line 
   beginning by the string <a>.
*/

void put_range_array(a,b,r,A)

char *a;
char *b;
int r;
char *A[];

{
  int i,j;

  for (i=0; 
       (((A[i]!=NULL)||(A[i+1]!=NULL)) &&
	((A[i]==NULL)||(strcmp(a,A[i])!=0)||((i!=0)&&(A[i-1]!=NULL)))); i++);
  if (A[i]==NULL)
    {
      fprintf(stderr,"[put_range_array] Unknown line beginning by the string \"%s\" on array \"%s\",\"%s\",...\n",a,A[0],A[1]);
      exit(1);
    }
  /* i = index of a in the array A */  

  for (j=i+1; (A[j]!=NULL); j++);
  if ((r <= 0)||(r >= j-i))
    {
      fprintf(stderr,"[put_range_array] Invalid range %d (out of [1,%d]) for line beginning by the string \"%s\" on array \"%s\",\"%s\",...\n",
	    r,j-i-1,a,A[0],A[1]);
      exit(1);
    }
  strcpy(b,A[i+r]);
}


/* Print in the file fp the function _mw_create_etype_from_itype()
   which create any possible external type from any internal type.
*/

void PrintCreateETypeFromIType(fp)

FILE *fp;

{
  Node A,B;
  int i,r;
  char TypeA[TNAME];
  char TypeB[TNAME];
  char ftype[TNAME];

  /* Print function declaration */

  fprintf(fp,"\n/* Save any possible internal type to any external type */\n\n");
  fprintf(fp,"short _mw_create_etype_from_itype(fname,mwstruct,typein,ftype)\n\n");
  fprintf(fp,"char *fname;    /* File name of the external structure to save (output) */\n");
  fprintf(fp,"void *mwstruct;	/* Memory structure to save */\n");
  fprintf(fp,"char *typein;    /* Internal (Memory) type of the structure */\n");
  fprintf(fp,"char *ftype;    /* External type to be used to save the structure */\n");
  fprintf(fp,"\n{\n");


  /* Print variable declaration */
  fprintf(fp,"  short ret;\n");
  for (A=G;A;A=A->next)
    {
      strcpy(TypeA,A->type);
      SetUpper(TypeA);
      fprintf(fp,"  %s %s;\n",TypeA,A->type);
    }
  fprintf(fp,"\n");

  /* Print conversions */
  for (A=G;A;A=A->next)
    {
      GetAccessibleNodes(A);
      PrintBeginTypeIn(fp,A->type);
      strcpy(TypeA,A->type);
      SetUpper(TypeA);      

      for (B=G;B;B=B->next)
	if (CheckIfPathExists(A,B)==1)
	  {    
	    strcpy(TypeB,B->type);
	    SetUpper(TypeB);      
	    r = get_max_range_array(B->type,mw_native_ftypes);
	    if (r>0)
	      {
		for (i=1; i<=r; i++)
		  {
		    if (i==1) fprintf(fp,"      if (");
		    else fprintf(fp,"||");
		    put_range_array(B->type,ftype,i,mw_native_ftypes);
		    fprintf(fp,"(_mw_is_of_ftype(ftype,\"%s\"))",ftype);
		    if (i==r) fprintf(fp,")\n      /* Conversion to %s */\n      {\n",B->type);		
		  }
		fprintf(fp,"        %s = (%s) mw_conv_internal_type(mwstruct,\"%s\",\"%s\");\n",B->type,TypeB,A->type,B->type);
		fprintf(fp,"        if (%s==NULL) mwerror(FATAL,1,\"Sorry, failure in the conversion from %s to %s does not allow to save the %s structure. Abort.\\n\");\n",B->type,A->type,B->type,B->type);
		fprintf(fp,"        ret=_mw_%s_create_native(fname,%s,ftype);\n",
			B->type,B->type);
		fprintf(fp,"        mw_delete_%s(%s);\n",B->type,B->type);
		fprintf(fp,"        return(ret);\n");
		fprintf(fp,"       }\n");
	      }
	  }
      fprintf(fp,"     return(-1);  /* End of typein = %s */\n",A->type);
      fprintf(fp,"    }\n");
    }
  fprintf(fp," return(-1);  /* No more memory type available ! */\n");
  fprintf(fp,"}\n");
}

  
/* Print in the file fp the variables mw_type_conv_out and mw_type_conv_in
   initialized to list all possible internal type conversions.
*/

void PrintPossibleConvType(fp)

FILE *fp;

{
  Node A,B;
  int i;
  char Path[BUFSIZ];

  /* Print output internal type conversion possibilities */

  fprintf(fp,"\n/* List all output internal type conversion possibilities\n");
  fprintf(fp,"   using the following format :\n");
  fprintf(fp,"   i1,o11,o12,...,o1n,NULL,i2,o21,...o2p,NULL,...,NULL,NULL\n");
  fprintf(fp,"   String i1 contains the first input internal type and strings\n");
  fprintf(fp,"   o11,o12,...,o1n all possible output types in the conversion i->o.\n"); 
  fprintf(fp,"*/\n\n"); 
  fprintf(fp,"static char *mw_type_conv_out[]=\n\n{\n");

  i=0;
  for (A=G;A;A=A->next)
    {
      GetAccessibleNodes(A);
      fprintf(fp,"  \"%s\",",A->type);
      for (B=G;B;B=B->next)
	if (CheckIfPathExists(A,B)==1)
	  fprintf(fp,"\"%s\",",B->type);	    
      fprintf(fp,"NULL,\n");	    
    }
  fprintf(fp,"  NULL\n};\n");	    

  /* Print input internal type conversion possibilities */

  fprintf(fp,"\n/* List all input internal type conversion possibilities\n");
  fprintf(fp,"   using the following format :\n");
  fprintf(fp,"   o1,i11,i12,...,i1n,NULL,o2,i21,...i2p,NULL,...,NULL,NULL\n");
  fprintf(fp,"   String o1 contains the first output internal type and strings\n");
  fprintf(fp,"   i11,i12,...,i1n all possible input types in the conversion i->o.\n"); 
  fprintf(fp,"*/\n\n"); 
  fprintf(fp,"static char *mw_type_conv_in[]=\n\n{\n");

  i=0;
  for (A=G;A;A=A->next)
    {
      fprintf(fp,"  \"%s\",",A->type);
      for (B=G;B;B=B->next)
	{
	  GetAccessibleNodes(B);
	  if (CheckIfPathExists(B,A)==1)
	    fprintf(fp,"\"%s\",",B->type);	    
	}
      fprintf(fp,"NULL,\n");	    
    }
  fprintf(fp,"  NULL\n};\n");	    
}


/*
  Return in <a> and <b> the internal type of a function conversion in <line>.
  Ex : if line="Cimage mw_fimage_to_cimage(image_fimage)", 
       return a="fimage" and b="cimage".
  Return value 1 if such types found, 0 elsewhere.
*/
 
int get_conv_type(line,a,b)

char *line,*a,*b;

{
  char Ret[BUFSIZ],Func[BUFSIZ],F[BUFSIZ];
  int i,l;

  if (strlen(line)<10) return(0);
  if (sscanf(line,"%s %s",Ret,Func)!=2) return(0);
  if (sscanf(Func,"mw_%[^(]",F) != 1) return(0);

  l=strlen(F);
  for (i=1; (i<l-4)&&
	 ((F[i]!='_')||(F[i+1]!='t')||(F[i+2]!='o')||(F[i+3]!='_')); i++);
  if (i>=l-4) return(0);

  strncpy(a,F,i);
  a[i]='\0';
  strcpy(b,&F[i+4]);  
  
  /* Check if return type in <Ret> matches type in <b>. */
  strcpy(Func,b);
  SetUpper(Func);
  if (strcmp(Ret,Func)!=0) return(0);

  /*printf("\"%s\" a=%s b=%s\n",line,a,b);*/

  return(1);
}



/* Read INAME to figure out what are the basic conversion functions 
 */

void read_in_file()

{
  FILE *fp;
  char line[BUFSIZ],a[BUFSIZ],b[BUFSIZ];

  fp = fopen(INAME,"r");
  if (fp == NULL)
    {
      fprintf(stderr,"Cannot open '%s' file for reading !\n Exit.\n",INAME);
      exit(1);
    }

  while (fscanf(fp,"%[^\n\r]",line) != EOF)
    {
      getc(fp);
      if (get_conv_type(line,a,b)==1) AddNodeAndConv(a,b);
      *line='\0';
    } 

  fclose(fp);
}

FILE *open_out_C_file(command)

char *command;

{
  FILE *fp;

  fp = fopen(OCNAME,"w");
  if (fp == NULL)
    {
      fprintf(stderr,"Cannot open '%s' file for writing !\n Exit.\n",OCNAME);
      exit(1);
    }
  fprintf(fp,"/*\n");
  fprintf(fp,"          Extended conversion function for internal and external types\n");
  fprintf(fp,"  This file has been automatically generated by the MegaWave2 kernel using\n");
  fprintf(fp,"          %s\n",command);
  
  fprintf(fp,"*/\n\n");
  fprintf(fp,"#include <stdio.h>\n");
  fprintf(fp,"#include \"mw.h\"\n\n");

  return(fp);
}


FILE *open_out_I_file(command)

char *command;

{
  FILE *fp;

  fp = fopen(OINAME,"w");
  if (fp == NULL)
    {
      fprintf(stderr,"Cannot open '%s' file for writing !\n Exit.\n",OINAME);
      exit(1);
    }
  fprintf(fp,"/*\n");
  fprintf(fp,"          List all possible conversions between internal types\n");
  fprintf(fp,"  This file has been automatically generated by the MegaWave2 kernel using\n");
  fprintf(fp,"          %s\n",command);
  
  fprintf(fp,"*/\n\n");

  return(fp);
}

int main(argc,argv)

int argc;
char **argv;

{
  FILE *fpo; /* Pointer to OCNAME file and to OINAME file */

  /*printf("** Running make_type_conv **\n"); */
  read_in_file();
  /*PrintGraph();*/

  /* Write OINAME file */
  fpo=open_out_I_file(argv[0]);
  PrintPossibleConvType(fpo);
  fclose(fpo);

  /* Write OCNAME file */
  fpo=open_out_C_file(argv[0]);
  PrintConvInternalType(fpo);
  PrintLoadETypeToIType(fpo);
  PrintCreateETypeFromIType(fpo);
  fclose(fpo);


  exit(0);
}










