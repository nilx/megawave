/*~~~~~~~~~~~~~~ mwplight - The MegaWave2 Light Preprocessor ~~~~~~~~~~~~~~~~~~~

 Input/output all usage functions

 Author : Jacques Froment
 Date : 2007
 Version : 1.1
 Versions history :
   0.1 (August 2005, JF) initial internal release
   1.0 (April 2007, JF) final revision, ready for external release
   1.1 (April 2007, JF) changed getsentence() to allow ';' inside a string "..."
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
#include <unistd.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include "mwpl_main.h"


/*~~~~~~~~~~ Print on <fd> the current location of the <fs> file pointer ~~~~~~~~~~
*/

#ifdef __STDC__
void printfslocation(FILE *fd)
#else
void printfslocation(fd)
FILE *fd;
#endif
{
  long loc,l,lastline;
  int c,lines;
  
  if (!fs) return; /* file of source module not opened */
  loc=ftell(fs); 
  if (loc<0) return;/* cannot obtain the current value of the file position */

  /* rewind to count lines */
  rewind(fs); l=lines=0;
  do
    {
      c=getc(fs);
      l++;
      if (c=='\n') {lines++; lastline=l;}
    }
  while ((c!=EOF)&&(l<loc));

  if ((c!=EOF)&&(getc(fs)!=EOF)) 
    /* We have not finished to parse the module :
       print line where the error occured.
    */
    {

      fprintf(fd,"Line %d of %s\n",lines+1,module_file);
      
      /* print the line where the error occurred */
      if (fseek(fs, lastline, SEEK_SET)!=0) return;
      do
	{
	  c=getc(fs);
	  putc(c,fd);
	}
      while ((c!=EOF)&&(c!='\n'));
      
      
      if ((loc-lastline)<80)
	/* write a vertical arrow to locate more precisely the error */
	{
	  for (c=0;c<loc-lastline; c++) putc(' ',fd);
	  fprintf(fd,"^\n");
	  for (c=0;c<loc-lastline; c++) putc(' ',fd);
	  fprintf(fd,"|\n");
	}
    }

  /* go back to previous location */
  fseek(fs, loc, SEEK_SET);
  
}

/*~~~~~~~~~~ Output a message of type <type_error> in <fd> ~~~~~~~~~~
*/
#ifdef __STDC__
void message(int type_error, FILE *fd, char *fmt, ...)
#else
void message(va_alist)
va_dcl
#endif
{
  va_list marker;
  char msg[STRSIZE];
#ifndef __STDC__
  int type_error;
  FILE *fd;
  char *fmt;
#endif

#ifdef __STDC__
  va_start(marker, fmt);
#else
  va_start(marker);
  type_error = va_arg(marker, int);
  fd = va_arg(marker, FILE *);
  fmt = va_arg(marker, char *);
#endif

  vsprintf(msg, fmt, marker);
  va_end(marker);

  switch(type_error)
    {
    case WARNING :
       fprintf(fd, "\n--- mwplight warning ---");
       break;
    case ERROR :
       fprintf(fd, "\n*** mwplight error ***\n");
       break;
    }

  if (type_error==ERROR) printfslocation(fd);

  fprintf(fd, "%s\n", msg);

  if (type_error==ERROR)
    {
      fprintf(fd, "*** exit ***\n");
      exit(1);
    }
}


/*~~~~~~~~~~ 
  Remove path and extension in filename <in>. Example "/home/toto.c" -> "toto"
~~~~~~~~~~
*/
#ifdef __STDC__
void rmpathextfilename(char *in, char *out)
#else
void rmpathextfilename(in,out)
char *in;
char *out;
#endif

{
  int l;

  /* remove path */
  l=strlen(in)-1;
  while ((l>=0)&&(in[l]!='/')) l--;
  if (l>0) 
    /* path has been detected : remove it */
    { l++; in=&in[l]; }
  
  /* remove extension */
  while (*in != '\0')
    {
      if (*in=='.') { *out='\0'; return;}
      *(out++)=*(in++);
    }
  *out='\0';
}


/* ~~~~~~~~~~
   Convert string <in> to lowercase.
   Return the number of words in <in>, given as the number of spaces + 1.
   ~~~~~~~~~~
*/
#ifdef __STDC__
int lowerstring(char *in)
#else
int lowerstring(in) 
char *in;
#endif
{
  int ns=0;

  for (; *in != '\0'; in++)
    {
      if (isupper(*in))
	*in = tolower(*in);
      else
	if (*in==' ') ns++;
    }
  return(ns+1);
}

/*~~~~~~~~~~ 
  Return the input string s corrected to be printed using printf()
  ~~~~~~~~~~ 
*/

#ifdef __STDC__
char *getprintfstring(char* s)
#else
char *getprintfstring(s)
char *s;
#endif
{
  static char o[TREESTRSIZE];
  int i,j;

  for (i=j=0;s[i]!='\0';i++,j++)
    switch(s[i])
      {
      case '"' :
	j--;
	break;
	
      case '%':
	o[j++]='%';

      default :
	o[j]=s[i];
      }
  o[j] = '\0';
  return(o);
}



/*~~~~~~~~~~ 
  Remove unnecessary spaces in <in>. Example : " toto  titi " -> "toto titi" 
  Change also \n \t \f to a space.
~~~~~~~~~~
*/
#ifdef __STDC__
void removespaces(char *in)
#else
void removespaces(in)
char *in;
#endif

{
  int i,j;

  if (in[0]=='\0') return;
  /* change blank char to space */
  i=0;
  while (in[i]!='\0') 
    {
      if ((in[i]=='\n')||(in[i]=='\t')||(in[i]=='\f')) in[i]=' ';
      i++;
    }
  
  i=0;
  while (in[i]!='\0')
    {
      if ((in[i]==' ')&&((i==0)||(in[i-1]==' ')||(in[i+1]=='\0')))
	/* shift 1 char on the left */
	{
	  j=i;
	  do
	    {
	      in[j]=in[j+1];
	    } while (in[j++]!='\0');
	  i--;
	}
      i++;
    }
}


/*~~~~~~~~~~ 
  Skip the current line in <fs> : go to the beginning of the next line.
  '\' is  recognized as the symbol to avoid line breaking.
  If a comment is encountered in the line, skip lines until end of comment.
  Return EOF if end of file, '\n' elsewhere.
  
~~~~~~~~~~
*/
#ifdef __STDC__
int skipline(void)
#else
int skipline()
#endif

{

  int l,l0;

 cont:
  l0=0;
  while (((l=getc(fs))!=EOF)&&(l!='\n')) 
    {
      if ((l0=='/')&&(l=='*')) skipcomment();
      l0=l;
    }
  if (l0=='\\') goto cont;
  return(l);
}

/*~~~~~~~~~~ 
  Get in <line> next line in file <fs>.
  '\' is not recognized as the symbol to avoid line breaking.
  Return EOF if end of file, 0 if the line is empty, 1 elsewhere.
  
~~~~~~~~~~
*/
#ifdef __STDC__
int getline(char *line)
#else
int getline(line)
char *line;
#endif

{

  int l;

  if (!fs) Error("NULL file pointer");
  l=fscanf(fs,"%[^\n]",line);
  getc(fs); /* read the \n */
  if ((l==0)||(l==EOF)) line[0]='\0';
  else removespaces(line);
  return(l);
}

/*~~~~~~~~~~ 
  Skip comment sequence in <fs>, assuming being already inside the comment.
  Comments are delimited by '/' '*' .... '*' '/' in C body
  and by '/' '/' in header until EOL (as in C++)
  Generate an error if EOF encountered
~~~~~~~~~~
*/
#ifdef __STDC__
void skipcomment(void)
#else
void skipcomment()
#endif

{
  int l,l0=0;

  if (!fs) Error("NULL file pointer");
  if (inside_header==0)
    /* seek for end of comment sequence in C body */
    {
      while ( ((l=getc(fs))!=EOF) && ((l!='/')||(l0!='*'))) l0=l;
      if (l==EOF)
	Error("Unexpected end of file while parsing comment in C body");
    }
  else 
    /* seek for end of comment sequence in header */
    {
      while ( ((l=getc(fs))!=EOF) && (l!='\n'));
      if (l==EOF)
	Error("Unexpected end of file while parsing comment in header");
    }
}



/*~~~~~~~~~~ 
  Get in <s> next sentence in file <fs>.
  A "sentence" is anything until next ';' not inside a string "..."
  Does not read comments (see skipcomment() for comments syntax)
  Return last char read (EOF if end of file, EOH if end of header)
  Should work inside or outside the header.
~~~~~~~~~~
*/
#ifdef __STDC__
int getsentence(char *s)
#else
int getsentence(s)
char *s;
#endif

{
  char *p;
  int n,l,l0=0,l00=0;
  int intext=0;

  if (!fs) Error("NULL file pointer");
  p=s; *p='\0';
  n=0;
  while (((l=getc(fs))!=EOF)&&((intext==1)||(l!=';'))  /* EOF or ';' but maybe inside a text */
	 && ((inside_header==0)||(l!='/')||(l0!='*')) /* seek for EOH */
	 )
    {
      if ( ((inside_header==0)&&(l0=='/')&&(l=='*')) ||
	   ((inside_header>0)&&(l0=='/')&&(l=='/')) )
	/* We enter comment sequence */
	{
	  p--; /* remove '/' from sentence */
	  n--;
	  skipcomment();
	  l0=l00;
	}
      else
	{
	  
	  if (n>STRSIZE) 
	    Error("[getsentence] sentence exceeds the maximum number of character = %d",STRSIZE);
	  *(p++)=(char)l;
	  n++;
	  l00=l0;
	  l0=l;
	  if (l=='"') intext=1-intext; /* a "text" is everything inside "..." */
	}
    }
  
  if ((inside_header!=0)&&(l=='/')&&(l0=='*')) 
    /* EOH read : '*' followed by '/' */
    {
      *(p--)='\0';
      removespaces(s);
      /*printf("EOH detected s='%s'\n",s);*/
      return(EOH);
    }

  *p='\0';
  removespaces(s);
  /*printf("s='%s'\n",s);*/
  return(l);
}


/*~~~~~~~~~~ 
  Skip block {...} in file <fs> :
  continue until next } is reached (or EOF).
  Work recursively on sub-blocks.
  Work outside the header only.
~~~~~~~~~~
*/
#ifdef __STDC__
void skipblock(void)
#else
void skipblock()
#endif

{
  int l,l0=0,l00=0,in_quote,in_dquote;
  
  if (!fs) Error("NULL file pointer <fs>");

  in_quote=in_dquote=0;

  while ((l=getc(fs))!=EOF)
    {
      switch(l)
	{
	case '"' :
	  if (l0!='\\') 
	    {
	      if (in_quote==0)
		in_dquote=1-in_dquote;
	    }
	  break;

	case '\'' :
	  if (l0!='\\') 
	    {
	      if (in_dquote==0)
		in_quote=1-in_quote;
	    }
	  break;

	case '*' :
	  if ((in_quote==0)&&(in_dquote==0))
	    {
	      if (l0=='/')
		/* We enter comment sequence */
		skipcomment();
	    }
	  break;

	case '#' :
	  if ((in_quote==0)&&(in_dquote==0))
		/* We enter a #something sequence */
	      skipline();
	  break;

	case '{' :
	  if ((in_quote==0)&&(in_dquote==0))
	    /* We enter a new block {...} */
	    skipblock();
	  break;

	case '}' :
	  if ((in_quote==0)&&(in_dquote==0))
	    /* We enter a block {...} */
	    return;  /* end of block reached */
	  break;
	  
	}
      l0=l;
    }
}


/*~~~~~~~~~~ 
  Get in <s> next instruction in file <fs>.
  An "instruction" is anything until next ';', but
  - lines beginning by #,
  - block instructions {...}
  Does not read comments (see skipcomment() for comments syntax)
  Return last char read (EOF if end of file).
  Set <lbeg>,<lend> that try to record location of the beginning
  and of the end of the "right" instruction in the file.
  Work outside the header only.  
~~~~~~~~~~
*/
#ifdef __STDC__
int getinstruction(char *s,long *lbeg, long *lend)
#else
int getinstruction(s,lbeg,lend)
char *s;
long *lbeg;
long *lend;
#endif

{
  char *p,*q;
  int n,l,l0=0,l00=0,in_quote,in_dquote,nl;

  if (!fs) Error("NULL file pointer <fs>");
  p=s; *p='\0';
  n=0;
  in_quote=in_dquote=0;
  *lbeg=*lend=-1;

  while ((l=getc(fs))!=EOF)
    {
      switch(l)
	{
	case '"' :
	  if (l0!='\\') 
	    if (in_quote==0)
	      in_dquote=1-in_dquote;
	  goto def;

	case '\'' :
	  if (l0!='\\') 
	    {
	      if (in_dquote==0)
		in_quote=1-in_quote;
	    }
	  goto def;

	case '*' :
	  if ((in_quote==0)&&(in_dquote==0))
	    {
	      if (l0=='/')
		/* We enter comment sequence */
		{
		  p--; /* remove '/' from sentence */
		  if (nl==n) *lbeg=-1; 
		  n--;
		  skipcomment();
		  l0=l00;
		  break;
		}
	      goto def;
	    }
	  break;

	case '#' :
	  if ((in_quote==0)&&(in_dquote==0))
	    {	  
		/* We enter a #something sequence */
	      skipline();
	      break;
	    }
	  goto def;

	case '{' :
	  if ((in_quote==0)&&(in_dquote==0))
	    {	  
	      /* We enter a block {...} */
	      skipblock();

	      /* Record the {} so that we will be able to keep trace of it,
		 so the block is replaced by an empty one.
	      */
	      if (n+1>STRSIZE) 
		Error("[getinstruction] sentence exceeds the maximum number of character = %d",STRSIZE);
	      q=p-1;
	      *(p++)='{';
	      n++;
	      l00=l0;
	      l0=l;
	      *(p++)='}';
	      n++;
	      l00=l0;
	      l0=l;

	      /* Analyse if char preceding '{' was ')' so that we guess a function
		 declaration and we end the instruction after '}'.
	      */
	      while ((q!=s)&&((*q==' ')||(*q=='\n')||(*q=='\t')||(*q=='\f'))) q--;
	      if (*q==')') goto EndofInstruction; /* end of the instruction */

	      break;
	    }
	  goto def;
	  
	default:
	def:            /* record the character */
	  if (n>STRSIZE) 
	    Error("[getinstruction] sentence exceeds the maximum number of character = %d",STRSIZE);
	  *(p++)=(char)l;
	  n++;
	  l00=l0;
	  l0=l;
	  
	  /* Check if the character is meaningful so that we can say here the "right"
	     instruction begins.
	  */
	  if ((*lbeg==-1)&&(l!=' ')&&(l!='\n')&&(l!='\t')&&(l!='\f')&&(l!='\r')&&(l!='\v'))
	    {
	      *lbeg=ftell(fs); 
	      nl=n;
	    }
	  
	  if (l==';') goto EndofInstruction; /* end of the instruction */
	  break;
	}
    }
 EndofInstruction:
  *p='\0';
  removespaces(s);
  *lend=ftell(fs); /* can compute better estimate (as with lbeg) if needed ! */

#ifdef DEBUG
  printf("\n[getinstruction] s='%s'\n",s);
#endif

  return(l);
}


/*~~~~~~~~~~ 
  Remove first surrounding braces in <in>, removing outside spaces.
  Output is in <out>.
  Example : in=" {toto ti{t}i } " -> out="toto ti{t}i" 
  Return 1 if braces removed, 0 elsewhere.
~~~~~~~~~~
*/
#ifdef __STDC__
int removebraces(char *in, char *out)
#else
int removebraces(in,out)
char *in;
char *out;
#endif

{
  int i,i0,i1;

  out[0]='\0';
  if (in[0]=='\0') return(0);

  i=0;
  while (in[i]==' ') i++;
  if (in[i]!='{') return(0);
  i0=i;
  
  i=strlen(in)-1;
  while ((i>=0)&&(in[i]==' ')) i--;
  if ((i<=i0) || (in[i]!='}')) return(0);
  i1=i;

  for (i=i0+1;i<i1;i++) out[i-i0-1]=in[i]; 
  out[i-i0-1]='\0';

  return(1);
}

/*~~~~~~~~~~ 
  Remove the terminating space in string <in>, if any.
  Example : in="char * " -> in="char *" 
~~~~~~~~~~
*/
#ifdef __STDC__
void RemoveTerminatingSpace(char *in)
#else
void RemoveTerminatingSpace(in)
char *in;
#endif

{
  int l;

  if ((!in)||((l=strlen(in))==0)) return;
  if (in[l-1]==' ') in[l-1]='\0';
}


/*~~~~~~~~~~ 
  Get the enclosed string by removing surrounding quotation marks (").
  Everything before those marks are removed, if any.
  Example : '..."toto "titi""...' -> 'toto "titi"'
  Return 1 if quotation marks found and removed, 0 if nothing done 
  (in that case <out> is undefined).
~~~~~~~~~~
*/
#ifdef __STDC__
int getenclosedstring(char *in, char *out)
#else
int getenclosedstring(in,out)
char *in;
char *out;
#endif

{
  int l,i,i0,i1;

  l=strlen(in)-1;
  /* find fist quotation mark */
  for (i0=0;(i0<=l)&&(in[i0]!='"');i0++);
  if (i0==l) return(0);

  /* find last quotation mark */
  for (i1=l;(i1>=0)&&(in[i1]!='"');i1--);
  if (i1<=i0) return(0);  

  /* set <out> */
  for (i=i0+1;i<i1;i++) out[i-i0-1]=in[i]; 
  out[i-i0-1]='\0';
  return(1);
}


/*~~~~~~~~~~ 
  Get useful environement variables
~~~~~~~~~~
*/
#ifdef __STDC__
int getenvironmentvar(void)
#else
int getenvironmentvar()
#endif

{
  char *v;

  if ((v=getenv("MEGAWAVE2"))!=NULL)
    strcpy(MEGAWAVE2,v);
  else
    Error("Environment variable $MEGAWAVE2 not set");

  if ((v=getenv("MY_MEGAWAVE2"))!=NULL)
    strcpy(MY_MEGAWAVE2,v);
  else
    Error("Environment variable $MY_MEGAWAVE2 not set");

}

/*~~~~~~~~~~ 
  Get current working directories (CWD and PWD)
~~~~~~~~~~
*/
#ifdef __STDC__
int getcurrentworkingdir(void)
#else
int getcurrentworkingdir()
#endif
     
{
  char *v;

  /* Beware : use of getcwd() can cause errors on automounted filesystems.
     Indeed, this function (as /bin/pwd) returns the fully resolved name of the 
     current directory, that can be different from the name returned by
     the shell command pwd (as this one would return aliased name).

     This is why we get 2 variables CWD and PWD
     CWD = Current working Directory as returned by getcwd() or /bin/pwd 
     PWD = Current working Directory as returned by shell built-in pwd 
  */


  /* get CWD */ 
  if (getcwd(CWD, BUFSIZ*sizeof(char)) == NULL)
    Error("Cannot get current working directory !");

  /* get PWD */
  if ((v=getenv("cwd"))!=NULL)
    /* on csh, set in $cwd */
    strcpy(PWD,v);
  else
    if ((v=getenv("PWD"))!=NULL)
      /* on sh, set in $PWD */
      strcpy(PWD,v);
    else
      strcpy(PWD,CWD);

  /*printf("[getcurrentworkingdir] CWD=%s  PWD=%s\n",CWD,PWD);*/

}  

/*~~~~~~~~~~ 
  Get group name from the current working directories
~~~~~~~~~~
*/
#ifdef __STDC__
int getgroupname(void)
#else
int getgroupname()
#endif

{
  char *v;
  char mwsrcdir[BUFSIZ]; /* megawave2 src directory to be used */
  int l;

  if (adm_mode==1) sprintf(mwsrcdir, "%s/src", MEGAWAVE2); /* $MEGAWAVE2/src */
  else sprintf(mwsrcdir, "%s/src", MY_MEGAWAVE2); /* $MY_MEGAWAVE2/src */

  if ((v = strstr(CWD, mwsrcdir)) != NULL) 
    {
      l = strlen(mwsrcdir);
      strcpy(group_name, (*(v+l)=='\0') ? "." : v+l+1);
    }
  else
    /* Unmatched dir with CWD. Try with PWD. */
    if ((v = strstr(PWD, mwsrcdir)) != NULL) 
      {
	l = strlen(mwsrcdir);
	strcpy(group_name, (*(v+l)=='\0') ? "." : v+l+1);
      }
    else
      {
	if (strcmp(CWD,PWD)==0)
	  Error("Invalid current directory \"%s\".\nExpecting subdir of \"%s\"", CWD,mwsrcdir);
	else
	  Error("Invalid current directory \"%s\" (or equivalently \"%s\").\nExpecting subdir of \"%s\"", CWD,PWD,mwsrcdir);
      }

  /*printf("[getgroupname] group_name=\"%s\"\n",group_name);*/
}

/*~~~~~~~~~~ 
  Get the first word in <s> and put it in <w> (a word may be a non-blank separator).
  Return the non-blank position just after the last char of word, or 0 if word not found.
  Example : " toto, titi" -> <w>="toto" pos=5 (index. of ',')
~~~~~~~~~~
*/
#ifdef __STDC__
int getword(char *s, char *w)
#else
int getword(s,w)
char *s;
char *w;
#endif

{
  int i,i0;

  i=0;
  while ((s[i]!='\0')&&(s[i]==' ')) i++;
  if (s[i]=='\0') { w[0]='\0'; return(0);}
  if (!IsCharCid(s[i])) /* Separator */
    { 
      w[0]=s[i]; w[1]='\0'; 
      return(i+1);
    }
  i0=i;
  while ((s[i]!='\0')&&(IsCharCid(s[i])))
    {
      if (i-i0>=TREESTRSIZE) 
	Error("Word in \"%s\" exceeds maximum size length of %d char.",s,TREESTRSIZE-1);
      w[i-i0]=s[i];
      i++;
    }
  w[i-i0]='\0';
  if (i>0) while ((s[i]!='\0')&&(s[i]==' ')) i++;
  return(i);
}

/*~~~~~~~~~~ 
  Get the first C_id (C variable name) in <s> and put it in <cid>.
  Return the non-blank position just after the last char of C_id, or 0 if C_id not found.
  Example : " toto [0,100]" -> <C_id>="toto" pos=6 (index. of '[')
~~~~~~~~~~
*/
#ifdef __STDC__
int getCid(char *s, char *cid)
#else
int getCid(s,cid)
char *s;
char *cid;
#endif

{
  int i,i0;

  i=0;
  while ((s[i]!='\0')&&(s[i]==' ')) i++;
  i0=i;
  while ((s[i]!='\0')&&(IsCharCid(s[i])))
    {
      if (i-i0>=TREESTRSIZE) 
	Error("C_id in \"%s\" exceeds maximum size length of %d char.",s,TREESTRSIZE-1);
      cid[i-i0]=s[i];
      i++;
    }
  cid[i-i0]='\0';
  if (i>0) while ((s[i]!='\0')&&(s[i]==' ')) i++;
  return(i);
}

/*~~~~~~~~~~ 
  Get the first interval [Min,Max] in <s> and put bounds in <min> and <max>.
  Return IC type (NONE if no interval found).
  If an interval was found, return in <ai> the non-blanck position after
  the interval in <s>.
  Note : it does not perform interval checking. This should be done after the
         C type has been determined.
~~~~~~~~~~
*/
#ifdef __STDC__
int getInterval(char *s, char *min, char *max, int *ai)
#else
int getInterval(s,min,max,ai)
char *s;
char *min;
char *max;
int *ai;
#endif

{
  int i,a0,a1,b0,b1;
  char a,b;

  min[0]=max[0]='\0';
  *ai=0;

  i=0;
  while ((s[i]!='\0')&&(s[i]==' ')) i++;
  if (s[i]=='\0') return(NONE);  

  /* seek left bracket */
  while ((s[i]!='\0')&&(s[i]!='[')&&(s[i]!=']')) i++;
  a0=i+1;
  a=s[i];
  if ((a=='\0')||(s[a0]=='\0')) return(NONE);  

  /* seek comma */
  while ((s[i]!='\0')&&(s[i]!=',')) i++;
  a1=i-1;
  b0=i+1;
  if ((s[i]=='\0')||(a1<a0)||(s[b0]=='\0')) return(NONE);  

  /* seek right bracket */
  while ((s[i]!='\0')&&(s[i]!=']')&&(s[i]!='[')) i++;
  b1=i+1;
  b=s[i];

  if (b=='\0') return(NONE);  

  if (a1-a0+1>=TREESTRSIZE) 
    Error("Left bound of interval \"%s\" exceeds maximum size length of %d char.",
	  s,TREESTRSIZE-1);
  for (i=a0;i<=a1;i++) min[i-a0]=s[i];
  min[i-a0]='\0';
  removespaces(min);

  if (b1-1-b0>=TREESTRSIZE) 
    Error("Right bound of interval \"%s\" exceeds maximum size length of %d char.",
	  s,TREESTRSIZE-1);
  for (i=b0;i<=b1-2;i++) max[i-b0]=s[i];
  max[i-b0]='\0';
  removespaces(max);  

  *ai=b1;
  while (s[*ai]==' ') (*ai)++;

  if (a=='[')
    {
      if (b==']') return(CLOSED);
      else return(MAX_EXCLUDED);
    }
  else
    {
      if (b==']') return(MIN_EXCLUDED);
      else return(OPEN);
    }
}

/*~~~~~~~~~~ 
  Return 1 if string <s> is a valid C identifier, 0 elsewhere
~~~~~~~~~~
*/
#ifdef __STDC__
int IsStringCid(char *s)
#else
int IsStringCid(s)
char *s;
#endif

{
  char c;
  int i;

  /* First char must be a letter */
  c=s[0];
  if ( ((c<'A')||(c>'Z'))&&((c<'a')||(c>'z'))&&(c!='_'))
    return(0);

  /* Check other char. */
  i=1;
  while ((c=s[i++])!='\0')
    if (!IsCharCid(c)) return(0);
  return(1);
}

/*~~~~~~~~~~ 
  write in file <fd> the prototype of the function in <f>.
  If <ansi>=1, the prototype generates the ANSI prototype syntax
  (and the K&R ones for backward compatibility with old compilers).
  If not, the K&R prototype format is generated for all compilers.
~~~~~~~~~~
*/
#ifdef __STDC__
void WriteFuncPrototype(FILE *fd, VarFunc *f, int ansi)
#else
void WriteFuncPrototype(fd,f,ansi)
FILE *fd;
VarFunc *f;
int ansi;
#endif

{
  Variable *p;

  /* BEWARE : ANSI prototype for an external function has to be used IF AND ONLY IF the declaration
     of the function has been made following the ANSI syntax.
     For the module's main function, that means that the ANSI prototype as to be called IF AND
     ONLY IF the module's main function definition in the M-file follows the ANSI syntax.
     So if the user does not use the ANSI syntax inside the module, the light preprocessor
     would have to generate the M-file by adding the ANSI syntax (see mwpl_mfile.c)
  */

  if (!fd) Error("[WriteFuncPrototype] Cannot prototype f : NULL file descriptor");
  if (!f || !f->v) Error("[WriteFuncPrototype] Cannot prototype f : NULL function");
  if (!ISCI_FUNCTION(f)) Error("[WriteFuncPrototype] Cannot prototype f : not a function");

  if (ansi==1)
    {
      fprintf(fd,"#ifdef __STDC__\n");
      fprintf(fd,"  extern %s %s(",f->v->Ftype,f->v->Name);
      for (p=f->param; p; p=p->next)
	{
	  fprintf(fd,"%s",p->Ftype);
	  if (p->next) fprintf(fd,",");
	}
      fprintf(fd,");\n");
      fprintf(fd,"#else\n");
      fprintf(fd,"  extern %s %s();\n",f->v->Ftype,f->v->Name);
      fprintf(fd,"#endif\n");
    }
  else
    fprintf(fd,"  extern %s %s();\n",f->v->Ftype,f->v->Name);
}

/*~~~~~~~~~~ 
  write in file <fd> the text given, following the syntax of fprintf, and do needed conversion
  for inclusion in a LaTeX source (T-file : .doc).
  ~~~~~~~~~~
*/
#ifdef __STDC__
void fprinttex(FILE *fd, char *fmt, ...)
#else
void fprinttex(va_alist)
va_dcl
#endif
{
#ifndef __STDC__
  FILE *fd;
  char *fmt;
#endif
  va_list marker;
  int longflg;
  int fmtflg;
  longflg = fmtflg = 0;
#ifdef __STDC__
  va_start(marker, fmt);
#else
  va_start(marker);
  fd = va_arg(marker, FILE *);
  fmt = va_arg(marker, char *);
#endif
  while(*fmt != 0) {  
    if (*fmt != '%' && fmtflg == 0)
      fputc(*fmt++, fd);
    else {
      char c;
      int i;
      int t_val;
      char *s;
      fmtflg = 1;
      
      switch(*++fmt) {
        case '%':
          fprintf(fd, "\\%", fd);
          longflg = fmtflg = 0;
          break;
        case 'c':
	  /* JF 12/27/01: On Linux kernel 2.4.8-26mdk :
	     Need a cast here since va_arg only
	     takes fully promoted types 
	  */
	  c = (char) va_arg(marker, int);
          fprintf(fd, "%c", c);
          longflg = fmtflg = 0;
          break;
        case 's' :
          fprintf(fd, "%s", va_arg(marker, char *));
          longflg = fmtflg = 0;
          break;
        case 'T' :
          for (s = va_arg(marker, char *); *s != '\0'; s++) {
            int escflg = 0;
            switch(*s) {
              case '$' :
              case '%' :
              case '{' :
              case '}' :
              case '_' :
              case '&' :
              case '#' :
                if (escflg) {
                  fprintf(fd, "\\verb!\\!");
                  escflg = 0;
                }
                fprintf(fd, "\\%c", *s);
                break;
              case '\\' :
                if (escflg) {
                  fprintf(fd, "\\verb!\\!");
                  escflg = 0;
                }
                else
                  escflg = 1;
                break;
              case '^' :
		/* \verb!^! does not work inside a LaTeX 2e macro ! */
		if (escflg) {
		  fprintf(fd, "\\verb!\\!");
                  escflg = 0;
                }
		fprintf(fd, "$\\mathbf{\\hat{}}\\;$");
                break;
	    case '~' :
	    case '|' :
	    case '<' :
	    case '>' :
                if (escflg) {
                  fprintf(fd, "\\verb!\\!");
                  escflg = 0;
                }
                fprintf(fd, "$\\mathbf{%c}\\;$", *s);
                break;
              case 'a' :
              case 'b' :
              case 'f' :
              case 'n' :
              case 'r' :
              case 't' :
              case 'v' :
              case '?' :
              case '\'' :
              case '"' :
                if (escflg) {
                  switch (*s) {
                    case 'a' :
                    case 'b' :
                    case 'f' :
                    case 't' :
                    case 'v' :
                      break;
                    case 'n' :
                    case 'r' :
                      fprintf(fd, "\\newline\n");
                      break;
                    case '?' :
                    case '\'' :
                    case '"' :
                      putc(*s, fd);
                      break;
                  }
                  escflg = 0;
                }
                else
                  putc(*s, fd);
                break;
              default :
                if (escflg) {
                  fprintf(fd, "\\verb!\\!");
                  escflg = 0;
                }
                putc(*s, fd);
                break;
            }
          }
          longflg = fmtflg = 0;
          break;
        case 'x' :
          if (longflg)
            fprintf(fd, "%lx", va_arg(marker, unsigned long));
          else
            fprintf(fd, "%x", va_arg(marker, unsigned int));
          longflg = fmtflg = 0;
          break;
        case 'X' :
          if (longflg)
            fprintf(fd, "%lX", va_arg(marker, unsigned long));
          else
            fprintf(fd, "%X", va_arg(marker, unsigned int));
          longflg = fmtflg = 0;
          break;
        case 'u' :
          if (longflg)
            fprintf(fd, "%lu", va_arg(marker, unsigned long));
          else
            fprintf(fd, "%u", va_arg(marker, unsigned int));
          longflg = fmtflg = 0;
          break;
        case 'd' :
          if (longflg)
            fprintf(fd, "%ld", va_arg(marker, long));
          else
            fprintf(fd, "%d", va_arg(marker, int));
          longflg = fmtflg = 0;
          break;
        case 'f' :
          if (longflg)
            fprintf(fd, "%g", va_arg(marker, double));
          else
	    /* JF 12/27/01: On Linux kernel 2.4.8-26mdk :
	       Need a cast here since va_arg only
	       takes fully promoted types 
	    */
            fprintf(fd, "%g", (double)va_arg(marker, double));
          longflg = fmtflg = 0;
          break;
        case 'l' :
          longflg = 1;
          break;
        default :
          fprintf(fd, "%%%c", *fmt);
          longflg = fmtflg = 0;
          break;
      }
      if(longflg == 0)
        fmt++;
    }
  }
  va_end(marker);
}
