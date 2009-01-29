/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ascii_file.c
   
  Vers. 1.4
  Author : Jacques Froment
  Basic functions to manage the MW DATA ASCII files
   
  Versions history :
  V 1.4 (JF, April 2007) : added function _mw_dirbasename()
  V 1.3 (JF, March 2006) : 
  - added functions _mw_remove_first_spaces() and _mw_basename()
  - bug in _mw_open_data_ascii_file() corrected

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <string.h>
#include <sys/file.h>

#include "libmw-defs.h"
#include "error.h"


#include "ascii_file.h"

/*     Locate a string in a file */
/*     Characted ? in str may match any non-null character in the file */

int _mw_fascii_search_string(FILE *fp, char *str)
{
     int i,c,nogood;

     if (str[0] == '\0')
     {
	  mwerror(INTERNAL, 1,"[_mw_fascii_search_string]: Null input string\n");
	  return(0);
     }

     do
     {
	  /* Search the first caracter */
	  while ( ((char) (c=getc(fp))) != str[0]) if (c == EOF) return(EOF);

	  nogood=0;
	  /* Search the others */
	  for (i=1; (str[i] != '\0') && (c != EOF) && (nogood == 0); i++) 
	       if ( ((((char) (c=getc(fp))) != str[i]) && (str[i] != '?')) 
		    /* The following line useful in case of DOS input : CR LF to
		       be interpreted as LF alone :
		    */
		    && !((str[i]=='\n')&&(c=='\r')&&((c=getc(fp))=='\n'))
		    )
		    nogood=1;
      
	  if (c == EOF) return(EOF);
     }
     while (nogood == 1);  
     return(1);
}

/*    Remove first blanck spaces in string s, if any */

void _mw_remove_first_spaces(char *s)
{
     int i,l;

     l=strlen(s);
     while (s[0]==' ')
     {
	  for (i=0;i<l;i++)
	       s[i]=s[i+1];
	  l--;
     }
}

/*    Put in <bname> the basename of <s> (i.e. <s> without the dirname).
      Example : s="/tmp/titi/toto.c" -> bname="toto.c"
      <bname> should be allocated by at least the size of <s>.
*/
void _mw_basename(char *s, char *bname)
{
     int i,l;

     l=strlen(s);
     for (i=l-1; (i>0)&&(s[i]!='/'); i--);
     if (s[i]=='/') i++;
     strcpy(bname,&s[i]);
}

/*    Put in <dname> the dirname of <s> and in <bname> the basename of <s> 
      (i.e. <s> without the dirname).
      Example : s="/tmp/titi/toto.c" -> dname="/tmp/titi/" bname="toto.c"
      <dname> and <bname> should be allocated by at least the size of <s>.
*/
void _mw_dirbasename(char *s, char *dname, char *bname)
{
     int i,l;

     l=strlen(s);
     for (i=l-1; (i>0)&&(s[i]!='/'); i--);
     if (s[i]=='/') i++;
     strcpy(bname,&s[i]);
     if (i>0) strncpy(dname,s,i); 
     dname[i]='\0';
}

/*     Put in *ptr the value of a field named field_name. str_control gives */
/*     the type of *ptr in the same way than in scanf.                      */

int _mw_fascii_get_field(FILE *fp, char *fname, char *field_name, char *str_control, void *ptr)
{
     if (_mw_fascii_search_string(fp,field_name) != 1)
     {
	  mwerror(ERROR, 0,"Cannot find the field \"%s\" in the MegaWave2 Data Ascii file \"%s\"\n",field_name,fname);	  
	  return(0);
     }

     if (fscanf(fp,str_control,ptr) != 1)
     {
	  mwerror(ERROR, 0,"Error at or before the field \"%s\" in the MegaWave2 Data Ascii file \"%s\"\n",field_name,fname);	  
	  return(0);
     }
     return(1);
}


/*     Same as _mw_fascii_get_field(), but do not generate error if the
       field is not found (this allows to read fields added in an extended format). 
*/

int _mw_fascii_get_optional_field(FILE *fp, char *fname, 
				  char *field_name, char *str_control, 
				  void *ptr)
{
     if (_mw_fascii_search_string(fp,field_name) != 1) return(0); /* Not found */
     if (fscanf(fp,str_control,ptr) != 1)
     {
	  mwerror(WARNING, 0,"Error at or before the field \"%s\" in the MegaWave2 Data Ascii file \"%s\" (field ignored).\n",field_name,fname);	  
	  return(0);
     }
     return(1);
}


FILE *_mw_open_data_ascii_file(char *fname)
{
     FILE    *fp;

     if (!(fp = fopen(fname, "r")))
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  return(NULL);
     }


     if (_mw_fascii_search_string(fp,_MW_DATA_ASCII_FILE_HEADER) == EOF)
     {
	  mwerror(ERROR, 0,
		  "File \"%s\" is not in the MegaWave2 DATA ASCII format\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     return(fp);
}


FILE *_mw_create_data_ascii_file(char *fname)
{
     FILE    *fp;

     if (!(fp = fopen(fname, "w")))
     {
	  mwerror(ERROR, 0,"Cannot create the file \"%s\"\n",fname);
	  return(NULL);
     }

     fprintf(fp,"#------------------------------------------------------------\n");
     fprintf(fp,"#");
     fprintf(fp,_MW_DATA_ASCII_FILE_HEADER);
     fprintf(fp,"#------------------------------------------------------------\n");
     fprintf(fp,"#\n");

     return(fp);
}
