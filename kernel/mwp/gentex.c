/**
 ** gentex : generation of documentation file (.doc) in LaTeX format
 ** (c)1993-2003 J.Froment - S.Parrino
 ** Version 1.8
 **/

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
#include "token.h"
#include "io.h"
#include "symbol.h"
#include "mwarg.h"
#include "y.tab.h"

/* Remove " " in a word, as "word" -> word */
#ifdef __STDC__
void remove_dquote(char *buffer1, char *buffer2)
#else
remove_dquote(buffer1,buffer2)
char *buffer1,*buffer2;
#endif
{
  int l;

  if (buffer2 == NULL) strcpy(buffer1,"??");
  else
    {
      l = strlen(buffer2);
      if ((buffer2[0]!='"')||(buffer2[l-1]!='"'))
	strcpy(buffer1,buffer2);
      else
	{
	  strncpy(buffer1, buffer2+1, l-2);
	  buffer1[l-2] = '\0';
	}
    }
}

#ifdef __STDC__
gentex(FILE *fd, char *lastmodif_date)
#else
gentex(fd,lastmodif_date)
FILE *fd;
char *lastmodif_date;
#endif
{
  extern Node * mwfuncdecl;
  Node * CmpdStmtTmp;
  Cell *c;
  char function_descript[BUFSIZ],labo_name[BUFSIZ],group_name[BUFSIZ],
       version_num[BUFSIZ],author_name[BUFSIZ],description[BUFSIZ];
  char User_DepFile[BUFSIZ];
  char An0[5],An1[5];

#ifdef DEBUG
  PRDBG("gentex()\n");
#endif

  if ((mwgroup != NULL) && (mwgroup->val.text != NULL))
    {
      /*
      if (mwgroup->val.text[0]=='*')
	remove_dquote(group_name,mwgroup->val.text);
      else
	strcpy(group_name,mwgroup->val.text);
      */
      remove_dquote(group_name,mwgroup->val.text);
    }
  else strcpy(group_name,"??");

  /* Comments which may be used by a shell to make the whole doc */
  fprintf(fd, "%%*** Group=%s\n",group_name);
  fprintf(fd, "%%*** Name=%s\n\n",mwname->val.text);

  /* Header for the page */
  fprintf(fd,"\\markboth");
  fprinttex(fd,"{{\\em %T} \\hfill MegaWave2 User's Modules Library \\hfill {\\bf %T} \\hspace{1cm}}",group_name,mwname->val.text);
  fprinttex(fd,"{{\\em %T} \\hfill MegaWave2 User's Modules Library \\hfill {\\bf %T} \\hspace{1cm}}\n\n",group_name,mwname->val.text);

  /* === Label for the index === */
  fprintf(fd,"\\label{%s}\n\n",mwname->val.text);

  /* === Index entry  === */
  remove_dquote(function_descript,mwfunction->val.text);
  fprinttex(fd, "\\index{%T@{\\tt %T}}\n\n",mwname->val.text,mwname->val.text);  

  /* === Name === */
  remove_dquote(function_descript,mwfunction->val.text);
  fprinttex(fd, "\\Name{%T}{%T}\n\n", mwname->val.text, function_descript);

  /* === Synopsis === (command) */
  fprinttex(fd, "\\Synopsis{%T}{", mwname->val.text);
  /* Options */
  for (c = GET_FIRST(optionlist); c != NULL; c = GET_NEXT(c)) {
    Mwarg *opt;
    opt = (Mwarg *)GET_ELT(c);
      fprintf(fd, "[-%c", opt->d.o.o);
      /*if (!(opt->d.o.d.t == SCALARARG &&  opt->d.o.d.rw == WRITE)) 
	Removed JF 13/10/2000
       */
	{
	  if (opt->texname != NULL)
	    fprinttex(fd, " {\\em %T}] ", opt->texname);
	  else
	    fprinttex(fd, "] "); 
	}
	/*      else        fprinttex(fd, "] ");*/
  }
  /* Needed arg */
  for (c = GET_FIRST(neededarglist); c != NULL; c = GET_NEXT(c)) 
    {
      Mwarg *opt;
      opt = (Mwarg *)GET_ELT(c);
      if (!(opt->d.a.t == SCALARARG &&  opt->d.a.rw == WRITE))
	fprinttex(fd, "{\\em %T} ", opt->texname);
      else
	/* -- Added JF 07/06/01 -- */
	fprinttex(fd, "{\\em .} ");	
	}
  /* Optionnal arg */
  if (GET_NUMBER(optarglist) != 0) {
    int in_flg;
    for (c=GET_FIRST(optarglist), in_flg=FALSE; c != NULL; c = GET_NEXT(c)) {
      Mwarg *opt;
      opt = (Mwarg *)GET_ELT(c);
      if (!(opt->d.a.t == SCALARARG &&  opt->d.a.rw == WRITE))
        in_flg = TRUE;
    }
    if (in_flg) {
      fprinttex(fd, "[");
      for (c = GET_FIRST(optarglist); c != NULL; c = GET_NEXT(c)) {
        Mwarg *opt;
        opt = (Mwarg *)GET_ELT(c);
        if (!(opt->d.a.t == SCALARARG &&  opt->d.a.rw == WRITE))
          fprinttex(fd, "{\\em %T} ", opt->texname);
      }
      fprinttex(fd, "]");
    }
  }
  /* Var arg */
  if (GET_NUMBER(vararglist) != 0) {
    Mwarg *opt;
    c = GET_FIRST(vararglist);
    opt = (Mwarg *)GET_ELT(c);
    if (!(opt->d.a.t == SCALARARG &&  opt->d.a.rw == WRITE))
      fprinttex(fd, " {\\em ...}");
  }
  fprinttex(fd, "}\n\n");

  /* === Arguments === (command) */
  /* JF 04/02/02 : macros \Argument, \Summary, \Description
     replaced with plain text to allow \verb with LaTeX 2e
  */

  fprinttex(fd, "\n% --- ARGUMENTS : BEGIN --- \n");
  /*fprinttex(fd, "\\samepage\n");*/
  fprinttex(fd, "\\nopagebreak\n");
  fprinttex(fd, "\\vspace{-5mm}\n");  
  fprinttex(fd, "\\nopagebreak\n");
  fprinttex(fd, "\\begin{quotation}\n");

  /* Options */
  for (c = GET_FIRST(optionlist); c != NULL; c = GET_NEXT(c)) {
    Mwarg *opt;
    opt = (Mwarg *)GET_ELT(c);
    /* Modified JF 06/10/2000
    if (!(opt->d.o.d.t == SCALARARG && opt->d.o.d.rw == WRITE)) 
      {
	fprintf(fd, "-%c ", opt->d.o.o);
	if (opt->texname != NULL) {
	  if (!(opt->d.o.d.t == SCALARARG &&  opt->d.o.d.rw == WRITE)) 
	    fprinttex(fd, "{\\em %T}", opt->texname);
	  else
	    fprinttex(fd, " screen output");
	}
	fprinttex(fd, " : %T\n\n", noquote(opt->desc));
      }
    */
    fprintf(fd, "-%c ", opt->d.o.o);
    if (opt->texname != NULL) 
      {
	if (!(opt->d.o.d.t == SCALARARG && opt->d.o.d.rw == WRITE)) 
	  fprinttex(fd, "{\\em %T} : %T\n\n", opt->texname,noquote(opt->desc));
	else
	  fprinttex(fd, "{\\em %T} (screen output) : %T\n\n", opt->texname,noquote(opt->desc));
      }
    else
      {
	if (!(opt->d.o.d.t == SCALARARG && opt->d.o.d.rw == WRITE)) 
	  fprinttex(fd, " : %T\n\n", noquote(opt->desc));
	else
	  fprinttex(fd, " screen output : %T\n\n",noquote(opt->desc));
      }
  }

  /* Needed arg */
  for (c = GET_FIRST(neededarglist); c != NULL; c = GET_NEXT(c)) {
    Mwarg *opt;
    opt = (Mwarg *)GET_ELT(c);
    if (!(opt->d.a.t == SCALARARG && opt->d.a.rw == WRITE))
      fprinttex(fd, "{\\em %T} : %T\n\n", opt->texname, noquote(opt->desc));
    else
      /* -- Added JF 07/06/01 -- */
      fprinttex(fd, ". (screen output) : %T\n\n", noquote(opt->desc));
  }
  /* Opt arg */
  if (GET_NUMBER(optarglist) != 0) {
    for (c = GET_FIRST(optarglist); c != NULL; c = GET_NEXT(c)) {
      Mwarg *opt;
      opt = (Mwarg *)GET_ELT(c);
      if (!(opt->d.a.t == SCALARARG && opt->d.a.rw == WRITE))
        fprinttex(fd, "{\\em %T} : %T\n\n", opt->texname, noquote(opt->desc));
      else
        fprinttex(fd, "screen output : %T\n\n", noquote(opt->desc));
        
    }
  }
  /* Var arg */
  if (GET_NUMBER(vararglist) != 0) {
    Mwarg *opt;
    char buffer[BUFSIZ];
    c = GET_FIRST(vararglist);
    opt = (Mwarg *)GET_ELT(c);
    if (!(opt->d.a.t == SCALARARG && opt->d.a.rw == WRITE))
      fprinttex(fd, "{\\em %T} : %T\n\n", opt->texname, noquote(opt->desc));
    else
      fprinttex(fd, "screen output... : %T\n\n", noquote(opt->desc));
  }

  fprinttex(fd, "\\end{quotation}\n");
  fprinttex(fd, "\\Next\n");
  fprinttex(fd, "% --- ARGUMENTS : END --- \n\n");

  /* Summary (C function) */
  if (mwfuncdecl != NULL && mwfuncdecl->right != NULL &&
      mwfuncdecl->right->right != NULL &&
      mwfuncdecl->right->right->name == COMPOUND) 
    {
      fprinttex(fd, "\n% --- SUMMARY : BEGIN --- \n");
      /*fprinttex(fd, "\\samepage\n");*/
      fprinttex(fd, "\\Mark{\\Large\\bf  Function Summary} \\nopagebreak\\bigskip\n");
      fprinttex(fd, "\\nopagebreak\n\n");

      CmpdStmtTmp = mwfuncdecl->right->right;
      mwfuncdecl->right->right = NULL;
      texprintnode(fd, mwfuncdecl);
      mwfuncdecl->right->right = CmpdStmtTmp;

      fprinttex(fd, "\\Next\n");
      fprinttex(fd, "% --- SUMMARY : END --- \n\n");
    }

  else {
#ifdef DEBUG
    PRDBG("gentex : no MegaWave function description\n");
#endif
    INT_ERROR("gentex");
  }

  /* Description */
  fprinttex(fd, "\n% --- DESCRIPTION : BEGIN --- \n");
  /*fprinttex(fd, "\\samepage\n");*/
  fprinttex(fd, "\\Mark{\\Large\\bf  Description} \\nopagebreak\\bigskip\n");
  fprinttex(fd, "\\nopagebreak\n\n");
  fprinttex(fd, "\\nopagebreak\n");

  /* Include the module.doc file 
     Modified the 23/11/99  : no more group subdirectory in doc directory. 
  */
  
  /*
  if (group_name[0] == '?') 
    fprintf(fd,"Please set the group field in the module or insert here your text.\n\n");
  else
    {
      sprintf(User_DocFile,"%s/%s.tex",group_name,mwname->val.text);
      fprintf(fd,"\\input{%s}\n\n",User_DocFile);
    }
  */
  fprintf(fd,"\\input{src/%s.tex}\n\n",mwname->val.text);  
  
  fprinttex(fd, "\\Next\n");
  fprinttex(fd, "% --- DESCRIPTION : END --- \n\n");

  /* See also */
  sprintf(User_DepFile,"%s.dep",mwname->val.text);
  fprintf(fd,"\\input{obj/DEPENDENCIES/%s}\n\n",User_DepFile);

  /* Version */
  remove_dquote(version_num,mwversion->val.text);
  fprinttex(fd, "\\Version{%T}{%s}\n",version_num,lastmodif_date);

  /* Author */
  strcpy(An0,"1993");
  strncpy(An1,&lastmodif_date[strlen(lastmodif_date)-5],4);
  An1[4]='\0';

  if ((mwlabo != NULL) && (mwlabo->val.text != NULL))
    remove_dquote(labo_name,mwlabo->val.text);
  else
    strcpy(labo_name,
	   "CMLA, ENS Cachan, 94235 Cachan cedex, France");
  remove_dquote(author_name,mwauthor->val.text);
  if (strcmp(An0,An1) == 0)
    fprinttex(fd, "\\Author{%s}{%s}{%T}\n",author_name,An0,labo_name);
  else
    fprinttex(fd, "\\Author{%s}{%s-%s}{%T}\n",author_name,An0,An1,labo_name);

#ifdef DEBUG
    PRDBG("gentex : end\n");
#endif
}
