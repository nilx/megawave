/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ascii_file.c
   
   Vers. 1.0
   (C) 1993 Jacques Froment
   Basic functions to manage the MW DATA ASCII files

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include <sys/file.h>

#include "ascii_file.h"
#include "mw.h"

/*     Locate a string in a file */
/*     Characted ? in str may match any non-null character in the file */

int _mw_fascii_search_string(fp,str)  
     

FILE *fp;
char *str;

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
	if ( ( ((char) (c=getc(fp))) != str[i]) && (str[i] != '?')) nogood=1;

      if (c == EOF) return(EOF);
    }
  while (nogood == 1);  
  return(1);
}

/*     Put in *ptr the value of a field named field_name. str_control gives */
/*     the type of *ptr in the same way than in scanf.                      */

int _mw_fascii_get_field(fp,fname,field_name,str_control,ptr)

FILE *fp;
char *fname;
char *field_name;
char *str_control;
void *ptr;

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

FILE *_mw_open_data_ascii_file(fname)

char *fname;

{
  FILE    *fp;

  if (!(fp = fopen(fname, "r")))
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
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


FILE *_mw_create_data_ascii_file(fname)

char *fname;

{
  FILE    *fp;

  if (!(fp = fopen(fname, "w")))
    {
      mwerror(ERROR, 0,"Cannot create the file \"%s\"\n",fname);
      return(NULL);
    }

  fprintf(fp,"%%------------------------------------------------------------\n");
  fprintf(fp,_MW_DATA_ASCII_FILE_HEADER);
  fprintf(fp,"%%------------------------------------------------------------\n");

  fprintf(fp,"%%\n");

  return(fp);
}


