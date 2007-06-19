/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   mwio.c
   
   Vers. 2.17
   Author : Jacques Froment
   Input/Output functions as a link between External and Internal Types.

   Version history
   2.17 (JF, march 2006) : 
     - I/O function for wpack2d added
     - data directory path search changed (see _search_filename_in_dir())

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
#include <ctype.h>

/* for stat */
#ifdef Linux
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#include <sys/stat.h>
#endif

/* for opendir(),readdir(),... */
#include <sys/types.h>
#include <dirent.h>

#include "mw.h"

/*===== Flip the image's buffer =====*/

#ifdef __STDC__
void _mw_flip_image(register unsigned char *ptr,
short size, short dx, short dy, char flip)
#else
void _mw_flip_image(ptr,size,dx,dy,flip)

register unsigned char *ptr;
short size,dx,dy;
char flip; /* TRUE if need fleeping, FALSE else */
#endif

{
  register unsigned short *ptr2;
  register unsigned long *ptr4;
  register long i;

  if (flip == TRUE)  /* Flip the image data (reverse byte order) */
    {
      switch(size)
	{
	case 2 : /* Bytes 01 -> 10 */
	  if (sizeof(*ptr2) != size)
	    mwerror(FATAL, 0,
    "Cannot flip %d-bytes data with this machine architecture\n",size);
	  for (i=0, ptr2 = (unsigned short *) ptr; 
	       i<dx*dy; i++,ptr2++)
	    _mw_in_flip_b2(*ptr2);
	  break;

	case 4 : /* Bytes 0123 -> 3210 */
	  if (sizeof(*ptr4) != size)
	    mwerror(FATAL, 0,
    "Cannot flip %d-bytes data with this machine architecture\n",size);
	  for (i=0, ptr4 = (unsigned long *) ptr;
		 i<dx*dy; i++,ptr4++)
	    _mw_in_flip_b4(*ptr4);
	  break;	  
	  
	case 8 : /* Bytes 01234567 -> 76543210 */ 
	  /* Not implemented ! */
	default:
	  mwerror(FATAL, 0,"Cannot flip %d-bytes data !\n",size);
	}
    }
}

/*===== Write header for MW2 binary file types =====*/

FILE *_mw_write_header_file(fname,type,IDvers)

char *fname;
char *type;
float IDvers; /*   ID file format version number for MW2 binary types. 
   Increment this value each time you change the format of a MW2 binary type,
   so that compatibility with former type can be preserved.
	      */

{
 FILE *fp;
 unsigned short first2bytes;    /* The first 2 bytes */
 char HID[SIZE_OF_MW2_BIN_TYPE_ID+1];
 /*  Header ID for MW2 binary types, of the form e.g. /1.00/ when IDvers=1. 
  */ 
   
 if ((strlen(type) <= 4) || (strncmp(type,"MW2_",4) != 0))
   mwerror(INTERNAL,1,"[_mw_write_header_file] Cannot create header for type %s\n",type);

 if (IDvers >= 10.0)
   mwerror(INTERNAL,1,"[_mw_write_header_file] IDvers must be less than 10 (or change SIZE_OF_MW2_BIN_TYPE_ID)\n");

 /* Write first 4 bytes (MW2_) so that flipping can be checked */
 fp = fopen(fname, "w");
 if (fp == NULL) return(fp);
 /* On Intel processors, char order will be inversed */
 first2bytes=0x4D57; /* MW */
 fwrite(&first2bytes,2,1,fp);
 /* Normal char order on every processors */
  fwrite(type+2,strlen(type)-2,1,fp);

  /* write the header id */
  sprintf(HID,"/%.2f/",IDvers);
  fwrite(HID,strlen(HID),1,fp);
 return(fp);
}     

/*===== Default paths for seeking input files =====*/


/* Get the file status of <fname> : return
   0 if some errors occur (file probably isn't readable),
   1 if file is a regular file or a link,
   2 if file is a directory,
   3 if file is not readable,
   4 if file is of other type.
*/

static int _get_file_status(fname)

char *fname;

{
  struct stat statbuf;

  if (stat(fname,&statbuf) != 0) return(0);
  if ((statbuf.st_mode & S_IFDIR) == S_IFDIR) 
    {
      mwerror(WARNING,1,"'%s' is a directory, trying another match...\n",fname);
      return(2); /* directory */
    }
  if ((statbuf.st_mode & S_IRUSR) != S_IRUSR) 
    {
      mwerror(WARNING,1,"no read permission on '%s', trying another match...\n",fname);
      return(3); /* not readable */
    }

  if ((statbuf.st_mode & S_IFREG) == S_IFREG) return(1); /* regular */
  if ((statbuf.st_mode & S_IFLNK) == S_IFLNK) return(1); /* link */

  mwerror(WARNING,1,"'%s' is not a regular nor a link file, trying another match...\n",
	  fname);
  return(4);
}


/* Search fname in searchdir, recursively in each subdirectories.
   Return 1 if found and the found directory in founddir, 0 elsewhere.
*/

static int _search_filename_in_dir(fname,searchdir,founddir)  

char *fname;
char *searchdir;
char *founddir;

{
  char dname[BUFSIZ];
  FILE *fp;
  DIR *D;
  struct dirent *dirbuf; 
  struct stat statbuf;
  char newdir[BUFSIZ];

  sprintf(dname,"%s/%s",searchdir,fname);

  if (_get_file_status(dname)==1)  /* searchdir/fname is found */
    {
      strcpy(founddir,searchdir);
      return(TRUE);
    }
  /* Not found : list the subdir in searchdir */
  if ((D=opendir(searchdir))==NULL) return(FALSE); /* cannot open searchdir */
  
  while ((dirbuf=readdir(D))!=NULL)
    {
      if ((strcmp(dirbuf->d_name,".")==0) ||
	  strcmp(dirbuf->d_name,"..")==0) continue;
      
      sprintf(newdir,"%s/%s",searchdir,dirbuf->d_name);	      
      if ((stat(newdir,&statbuf) == 0) &&
	  ((statbuf.st_mode & S_IFDIR) == S_IFDIR))
	/* dirbuf->d_name is a directory */
	if (_search_filename_in_dir(fname,newdir,founddir)==TRUE)
	  return(TRUE);
    }
  closedir(D);
  return(FALSE);
}	     

/* Search fname in this order, <mwgroup> being the module's group :
   1) fname
   2) fname in $MY_MEGAWAVE2/data/<mwgroup>/ (including subdirectories)
   3) fname in all $MY_MEGAWAVE2/data/ (including subdirectories)
   4) fname in $MEGAWAVE2/data/<mwgroup> (including subdirectories)
   5) fname in all $MEGAWAVE2/data/ (including subdirectories)
*/

int _search_filename(fname)  /* Return 1 if found, 0 else */

char *fname;

{
  FILE *fp;
  int i;
  char *path,*getenv(),searchdir[BUFSIZ],founddir[BUFSIZ],newfname[BUFSIZ];

  *founddir = '\0';
  if (_get_file_status(fname)==1) /* fname is found */
    return(TRUE);

  /* Not found : see if fname contains absolute pathname. 
     If yes, return not found.
  */
  if (fname[0]=='/') return(FALSE);

  /* Search in subdirs of $MY_MEGAWAVE2/data */
  if ((path = getenv("MY_MEGAWAVE2")) != NULL)
    {
      sprintf(searchdir,"%s/data/%s",path,mwgroup);  
      if (_search_filename_in_dir(fname,searchdir,founddir)==TRUE)
	{
	  /* founddir/fname found ! */
	  sprintf(newfname,"%s/%s",founddir,fname);  	  
	  strcpy(fname,newfname);
	  return(TRUE);
	}
      sprintf(searchdir,"%s/data",path);  
      if (_search_filename_in_dir(fname,searchdir,founddir)==TRUE)
	{
	  /* founddir/fname found ! */
	  sprintf(newfname,"%s/%s",founddir,fname);  	  
	  strcpy(fname,newfname);
	  return(TRUE);
	}
    }

  /* Search in subdirs of $MEGAWAVE2/data */
  if ((path = getenv("MEGAWAVE2")) != NULL)
    {
      sprintf(searchdir,"%s/data/%s",path,mwgroup);  
      if (_search_filename_in_dir(fname,searchdir,founddir)==TRUE)
	{
	  /* founddir/fname found ! */
	  sprintf(newfname,"%s/%s",founddir,fname);  	  
	  strcpy(fname,newfname);
	  return(TRUE);
	}
      sprintf(searchdir,"%s/data",path);  
      if (_search_filename_in_dir(fname,searchdir,founddir)==TRUE)
	{
	  /* founddir/fname found ! */
	  sprintf(newfname,"%s/%s",founddir,fname);  	  
	  strcpy(fname,newfname);
	  return(TRUE);
	}
      
    }
  return(FALSE);
}

static void search_filename(fname)  

char *fname;

{
  if (_search_filename(fname) != TRUE)
    mwerror(FATAL, 0,"File \"%s\" not found in default path\n",fname);
}

/* Format in <out> the filename given in <in> to be set in the structure. */
static void format_filename(out,in)

char *in,*out;
{
  char *i;

  /* Remove the dirname if any */
  for (i=in+strlen(in)-1; (*i!='/')&&(i!=in); i--);
  if (i!=in) i++;
  strncpy(out,i,mw_namesize);
}

/*
Search for the first string pattern <label> in the file pointed
to by <fp>. 
Return the current value of the file position, located at the next
byte after the found pattern. If the pattern is not found, return
a negative value.
*/

long _mw_find_pattern_in_file(fp,label)

FILE *fp;
char *label;

{
  char buf[BUFSIZ];
  int l;
  long p;

  l=strlen(label);
  p=ftell(fp);
  if (p<0) return(p);

  while (1)
    {
      if (fread(buf,l,1,fp)!=1) return(-1);
      if (strncmp(label,buf,l)==0) break;
      if (fseek(fp,1-l,SEEK_CUR)!=0) return(-1);
      p++;
    }
  return(p+l);
}

/* Return 1 if the byte ordering on the architecture this code is running
   is little endian, or 0 if it is big endian.
   Example of little endian architecture : ix86
   Example of big endian architecture : Sun Sparc
*/

int _mw_byte_ordering_is_little_endian()
{
  int b;
  char *p;

  b = 1;
  p = (char *) &b;
  if (*p==1) return(1); else return(0);
}


/*========== I/O functions ==========*/

/*===== Internal type : Cimage =====*/

short _mwload_cimage(name, type, comment, im)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Cimage *im;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *im = (Cimage) _mw_cimage_load_image(fname, type_in);
  if (*im == NULL) return(-1);  

  format_filename((*im)->name,fname);

  strcpy(comment_in,(*im)->cmt);
  _mw_make_type(type,type_in,"cimage");
  _mw_make_comment(comment,comment_in);
  return(0);
}

short _mwsave_cimage(name, type, type_force, comment, im)

char *name;
char type[]; /* Default image type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Cimage im;

{
  _mw_choose_type(type,type_force,"cimage");
  if (im->cmt[0] == '?') sprintf(im->cmt,"%s(%s)",mwname,comment);

  if (_mw_cimage_create_image(name,im,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Fimage =====*/

short _mwload_fimage(name, type, comment, im)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Fimage *im;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */

  /* Load a new image */
  search_filename(fname);
  *im = (Fimage) _mw_fimage_load_image(fname, type_in);
  if (*im == NULL) return(-1);  

  format_filename((*im)->name,fname);

  strcpy(comment_in,(*im)->cmt);
  _mw_make_type(type,type_in,"fimage");
  _mw_make_comment(comment,comment_in);

  return(0);
}

short _mwsave_fimage(name, type, type_force, comment, im)

char *name;
char type[]; /* Default image type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Fimage im;

{

  _mw_choose_type(type,type_force,"fimage");
  if (im->cmt[0] == '?') sprintf(im->cmt,"%s(%s)",mwname,comment);

  if (_mw_fimage_create_image(name,im,type) >= 0)
    return(0);
  else
    return(-1);
}


/*===== Internal type : Cmovie =====*/

short _mwload_cmovie(name, type, comment, movie)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Cmovie *movie;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname); 

  *movie = (Cmovie) _mw_cmovie_load_movie(fname, type_in);
  if (*movie == NULL) return(-1);  

  format_filename((*movie)->name,fname);

  strcpy(comment_in,(*movie)->cmt);
  _mw_make_type(type,type_in,"cimage");
  _mw_make_comment(comment,comment_in);
  return(0);
}

short _mwsave_cmovie(name, type, type_force, comment, movie)

char *name;
char type[]; /* Default movie type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Cmovie movie;

{
  _mw_choose_type(type,type_force,"cimage");
  if (movie->cmt[0] == '?') sprintf(movie->cmt,"%s(%s)",mwname,comment);

  if (_mw_cmovie_create_movie(name,movie,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Fmovie =====*/

short _mwload_fmovie(name, type, comment, movie)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Fmovie *movie;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];
  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname); 

  *movie = (Fmovie) _mw_fmovie_load_movie(fname, type_in);
  if (*movie == NULL) return(-1);  

  format_filename((*movie)->name,fname);

  strcpy(comment_in,(*movie)->cmt);
  _mw_make_type(type,type_in,"fimage");
  _mw_make_comment(comment,comment_in);
  return(0);
}

short _mwsave_fmovie(name, type, type_force, comment, movie)

char *name;
char type[]; /* Default movie type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Fmovie movie;

{
  _mw_choose_type(type,type_force,"fimage");
  if (movie->cmt[0] == '?') sprintf(movie->cmt,"%s(%s)",mwname,comment);

  if (_mw_fmovie_create_movie(name,movie,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Ccmovie =====*/

short _mwload_ccmovie(name, type, comment, movie)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Ccmovie *movie;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname); 

  *movie = (Ccmovie) _mw_ccmovie_load_movie(fname, type_in);
  if (*movie == NULL) return(-1);  

  format_filename((*movie)->name,fname);

  strcpy(comment_in,(*movie)->cmt);
  _mw_make_type(type,type_in,"ccimage");
  _mw_make_comment(comment,comment_in);
  return(0);
}

short _mwsave_ccmovie(name, type, type_force, comment, movie)

char *name;
char type[]; /* Default movie type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Ccmovie movie;

{
  /* ccimage and not ccmovie because a ccmovie is composed by images of
     ccimage type.
  */
  _mw_choose_type(type,type_force,"ccimage");

  if (movie->cmt[0] == '?') sprintf(movie->cmt,"%s(%s)",mwname,comment);

  if (_mw_ccmovie_create_movie(name,movie,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Cfmovie =====*/

short _mwload_cfmovie(name, type, comment, movie)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Cfmovie *movie;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname); 

  *movie = (Cfmovie) _mw_cfmovie_load_movie(fname, type_in);
  if (*movie == NULL) return(-1);  

  format_filename((*movie)->name,fname);

  strcpy(comment_in,(*movie)->cmt);
  _mw_make_type(type,type_in,"cfimage");
  _mw_make_comment(comment,comment_in);
  return(0);
}

short _mwsave_cfmovie(name, type, type_force, comment, movie)

char *name;
char type[]; /* Default movie type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Cfmovie movie;

{

  /* cfimage and not cfmovie because a cfmovie is composed by images of
     cfimage type.
  */
  _mw_choose_type(type,type_force,"cfimage");

  if (movie->cmt[0] == '?') sprintf(movie->cmt,"%s(%s)",mwname,comment);

  if (_mw_cfmovie_create_movie(name,movie,type) >= 0)
    return(0);
  else
    return(-1);
}


/*===== Internal type : Curve =====*/

short _mwload_curve(name, type, comment, cv)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Curve *cv;

{
  char type_in[mw_ftype_size];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *cv = (Curve) _mw_load_curve(fname,type_in);
  if (*cv == NULL) return(-1);  

  _mw_make_type(type,type_in,"curve");

  return(0);
}

short _mwsave_curve(name, type, type_force, comment, cv)

char *name;
char type[]; /* Default curves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Curve cv;

{

  _mw_choose_type(type,type_force,"curve");
  if (_mw_create_curve(name,cv,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Curves =====*/

short _mwload_curves(name, type, comment, cv)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Curves *cv;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *cv = (Curves) _mw_load_curves(fname,type_in);
  if (*cv == NULL) return(-1);  

  format_filename((*cv)->name,fname);

  strcpy(comment_in,(*cv)->cmt);
  _mw_make_type(type,type_in,"curves");
  _mw_make_comment(comment,comment_in);

  return(0);
}

short _mwsave_curves(name, type, type_force, comment, cv)

char *name;
char type[]; /* Default curves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Curves cv;

{

  _mw_choose_type(type,type_force,"curves");
  if (cv->cmt[0] == '?') sprintf(cv->cmt,"%s(%s)",mwname,comment);

  if (_mw_create_curves(name,cv,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Polygon =====*/

short _mwload_polygon(name, type, comment, poly)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Polygon *poly;

{
  char type_in[mw_ftype_size];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *poly = (Polygon) _mw_load_polygon(fname,type_in);
  if (*poly == NULL) return(-1);  

  _mw_make_type(type,type_in,"polygon");

  return(0);
}

short _mwsave_polygon(name, type, type_force, comment, poly)

char *name;
char type[]; /* Default polygons type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Polygon poly;

{
  _mw_choose_type(type,type_force,"polygon");
  if (_mw_create_polygon(name,poly,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Polygons =====*/

short _mwload_polygons(name, type, comment, poly)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Polygons *poly;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *poly = (Polygons) _mw_load_polygons(fname,type_in);
  if (*poly == NULL) return(-1);  

  format_filename((*poly)->name,fname);

  strcpy(comment_in,(*poly)->cmt);
  _mw_make_type(type,type_in,"polygons");
  _mw_make_comment(comment,comment_in);

  return(0);
}

short _mwsave_polygons(name, type, type_force, comment, poly)

char *name;
char type[]; /* Default polygons type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Polygons poly;

{
  _mw_choose_type(type,type_force,"polygons");
  if (poly->cmt[0] == '?') sprintf(poly->cmt,"%s(%s)",mwname,comment);

  if (_mw_create_polygons(name,poly,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Fcurve =====*/

short _mwload_fcurve(name, type, comment, cv)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Fcurve *cv;

{
  char type_in[mw_ftype_size];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *cv = (Fcurve) _mw_load_fcurve(fname,type_in);
  if (*cv == NULL) return(-1);  

  _mw_make_type(type,type_in,"fcurve");

  return(0);
}

short _mwsave_fcurve(name, type, type_force, comment, cv)

char *name;
char type[]; /* Default fcurves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Fcurve cv;

{
  _mw_choose_type(type,type_force,"fcurve");
  if (_mw_create_fcurve(name,cv,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Fcurves =====*/

short _mwload_fcurves(name, type, comment, cv)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Fcurves *cv;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *cv = (Fcurves) _mw_load_fcurves(fname,type_in);
  if (*cv == NULL) return(-1);  

  format_filename((*cv)->name,fname);

  strcpy(comment_in,(*cv)->cmt);
  _mw_make_type(type,type_in,"fcurves");
  _mw_make_comment(comment,comment_in);

  return(0);
}

short _mwsave_fcurves(name, type, type_force, comment, cv)

char *name;
char type[]; /* Default fcurves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Fcurves cv;

{
  _mw_choose_type(type,type_force,"fcurves");
  if (cv->cmt[0] == '?') sprintf(cv->cmt,"%s(%s)",mwname,comment);

  if (_mw_create_fcurves(name,cv,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Fpolygon =====*/

short _mwload_fpolygon(name, type, comment, poly)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Fpolygon *poly;

{
  char type_in[mw_ftype_size];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *poly = (Fpolygon) _mw_load_fpolygon(fname,type_in);
  if (*poly == NULL) return(-1);  

  _mw_make_type(type,type_in,"fpolygon");

  return(0);
}

short _mwsave_fpolygon(name, type, type_force, comment, poly)

char *name;
char type[]; /* Default Fpolygons type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Fpolygon poly;

{
  _mw_choose_type(type,type_force,"fpolygon");
  if (_mw_create_fpolygon(name,poly,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Fpolygons =====*/

short _mwload_fpolygons(name, type, comment, poly)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Fpolygons *poly;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *poly = (Fpolygons) _mw_load_fpolygons(fname,type_in);
  if (*poly == NULL) return(-1);  

  format_filename((*poly)->name,fname);

  strcpy(comment_in,(*poly)->cmt);
  _mw_make_type(type,type_in,"fpolygons");
  _mw_make_comment(comment,comment_in);

  return(0);
}

short _mwsave_fpolygons(name, type, type_force, comment, poly)

char *name;
char type[]; /* Default fpolygons type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Fpolygons poly;

{
  _mw_choose_type(type,type_force,"fpolygons");
  if (poly->cmt[0] == '?') sprintf(poly->cmt,"%s(%s)",mwname,comment);

  if (_mw_create_fpolygons(name,poly,type) >= 0)
    return(0);
  else
    return(-1);
}


/*===== Internal type : Fsignal =====*/

short _mwload_fsignal(name, type, comment, signal)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Fsignal *signal;

{ char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *signal = (Fsignal) _mw_load_fsignal(fname,type_in,NULL);
  if (*signal == NULL) return(-1);

  format_filename((*signal)->name,fname);

  strcpy(comment_in,(*signal)->cmt);
  _mw_make_type(type,type_in,"fsignal");
  _mw_make_comment(comment,comment_in);

  return(0);
}


short _mwsave_fsignal(name, type, type_force, comment, signal)

char *name;
char type[]; /* Default Fsignal type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Fsignal signal;

{
  _mw_choose_type(type,type_force,"fsignal");
  if (signal->cmt[0] == '?') sprintf(signal->cmt,"%s(%s)",mwname,comment);

  if (_mw_create_fsignal(name,signal,type) >= 0)
    return(0);
  else
    return(-1);
}


/*===== Internal type : Wtrans1d =====*/

short _mwload_wtrans1d(name, type, comment, wtrans)

char *name; /* Name of the generic wavelet decomposition */
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Wtrans1d *wtrans;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *wtrans = (Wtrans1d) _mw_wtrans1d_load_wtrans(fname, type_in);
  if (*wtrans == NULL) return(-1);  

  format_filename((*wtrans)->name,fname);

  strcpy(comment_in,(*wtrans)->cmt);
  _mw_make_type(type,type_in,"fsignal");
  _mw_make_comment(comment,comment_in);
  return(0);
}

short _mwsave_wtrans1d(name, type, type_force, comment, wtrans)

char *name; /* Name of the generic wavelet decomposition */
char type[]; /* Default wtrans type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Wtrans1d wtrans;

{
  _mw_choose_type(type,type_force,"fsignal");
  if (wtrans->cmt[0] == '?') sprintf(wtrans->cmt,"%s(%s)",mwname,comment);

  if (_mw_wtrans1d_create_wtrans(name,wtrans,type) >= 0)
    return(0);
  else
    return(-1);
}


/*===== Internal type : Wtrans2d =====*/

short _mwload_wtrans2d(name, type, comment, wtrans)

char *name; /* Name of the generic wavelet decomposition */
             /* The extension must be the Short Name of the Impulse files */
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Wtrans2d *wtrans;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *wtrans = (Wtrans2d) _mw_wtrans2d_load_wtrans(fname, type_in);
  if (*wtrans == NULL) return(-1);  

  format_filename((*wtrans)->name,fname);

  strcpy(comment_in,(*wtrans)->cmt);
  _mw_make_type(type,type_in,"fimage");
  _mw_make_comment(comment,comment_in);
  return(0);
}

short _mwsave_wtrans2d(name, type, type_force, comment, wtrans)

char *name; /* Name of the generic wavelet decomposition */
             /* The extension must be the Short Name of the Impulse files */
char type[]; /* Default wtrans type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Wtrans2d wtrans;

{
  _mw_choose_type(type,type_force,"fimage");
  if (wtrans->cmt[0] == '?') sprintf(wtrans->cmt,"%s(%s)",mwname,comment);

  if (_mw_wtrans2d_create_wtrans(name,wtrans,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Vchain_wmax =====*/

short _mwload_vchain_wmax(name, type, comment, vchain)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Vchain_wmax *vchain;

{
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *vchain = (Vchain_wmax) _mw_load_vchain_wmax(fname);
  if (*vchain == NULL) return(-1);  
  return(0);
}

short _mwsave_vchain_wmax(name, type, type_force, comment, vchain)

char *name;

char type[]; /* Default polygons type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Vchain_wmax vchain;

{
  if (type_force[0] != '?') strcpy(type,type_force);
  if (_mw_create_vchain_wmax(name,vchain) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Vchains_wmax (set of Vchain_wmax) =====*/

short _mwload_vchains_wmax(name, type, comment, vchains)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Vchains_wmax *vchains;

{
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *vchains = (Vchains_wmax) _mw_load_vchains_wmax(fname);
  if (*vchains == NULL) return(-1);  

  format_filename((*vchains)->name,fname);
  strcpy(comment,(*vchains)->cmt);

  return(0);
}

short _mwsave_vchains_wmax(name, type, type_force, comment, vchains)

char *name;
char type[]; /* Default polygons type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Vchains_wmax vchains;

{
  if (type_force[0] != '?') strcpy(type,type_force);
  if (vchains->cmt[0] == '?') sprintf(vchains->cmt,"%s(%s)",mwname,comment);

  if (_mw_create_vchains_wmax(name,vchains) >= 0)
    return(0);
  else
    return(-1);
}


/*===== Internal type : Ccimage =====*/

short _mwload_ccimage(name, type, comment, im)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Ccimage *im;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *im = (Ccimage) _mw_ccimage_load_image(fname, type_in);
  if (*im == NULL) return(-1);  

  format_filename((*im)->name,fname);

  strcpy(comment_in,(*im)->cmt);
  _mw_make_type(type,type_in,"ccimage");
  _mw_make_comment(comment,comment_in);
  return(0);
}

short _mwsave_ccimage(name, type, type_force, comment, im)

char *name;
char type[]; /* Default image type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Ccimage im;

{
  _mw_choose_type(type,type_force,"ccimage");
  if (im->cmt[0] == '?') sprintf(im->cmt,"%s(%s)",mwname,comment);

  if (_mw_ccimage_create_image(name,im,type) >= 0)
    return(0);
  else
    return(-1);
}


/*===== Internal type : Cfimage =====*/

short _mwload_cfimage(name, type, comment, im)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Cfimage *im;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */

  /* Load a new image */
  search_filename(fname);
  *im = (Cfimage) _mw_cfimage_load_image(fname, type_in);
  if (*im == NULL) return(-1);  

  format_filename((*im)->name,fname);

  strcpy(comment_in,(*im)->cmt);
  _mw_make_type(type,type_in,"cfimage");
  _mw_make_comment(comment,comment_in);

  return(0);
}

short _mwsave_cfimage(name, type, type_force, comment, im)

char *name;
char type[]; /* Default image type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Cfimage im;

{
  _mw_choose_type(type,type_force,"cfimage");
  if (im->cmt[0] == '?') sprintf(im->cmt,"%s(%s)",mwname,comment);

  if (_mw_cfimage_create_image(name,im,type) >= 0)
    return(0);
  else
    return(-1);
}


/*===== Internal type : Modules =====*/

#include "module.h"

short _mwload_modules(name, type, comment, modules)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Modules *modules;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */

  /* Load a new image */
  search_filename(fname);
  *modules = (Modules) _mw_load_modules(fname);
  if (*modules == NULL) return(-1);  

  format_filename((*modules)->name,fname);

  strcpy(comment_in,(*modules)->cmt);
  _mw_make_comment(comment,comment_in);

  return(0);
}

short _mwsave_modules(name, type, type_force, comment, modules)

char *name;
char type[]; /* Default image type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Modules modules;

{
  if (type_force[0] != '?') strcpy(type,type_force);
  if (modules->cmt[0] == '?') sprintf(modules->cmt,"%s(%s)",mwname,comment);

  if (_mw_create_modules(name,modules) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Morpho_line  =====*/

short _mwload_morpho_line(name, type, comment, ll)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Morpho_line *ll;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *ll = (Morpho_line) _mw_load_morpho_line(fname,type_in);
  if (*ll == NULL) return(-1);  

  _mw_make_type(type,type_in,"morpho_line");

  return(0);
}

short _mwsave_morpho_line(name, type, type_force, comment, ll)

char *name;
char type[]; /* Default curves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Morpho_line ll;

{

  _mw_choose_type(type,type_force,"morpho_line");
  if (_mw_create_morpho_line(name,ll,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Fmorpho_line  =====*/

short _mwload_fmorpho_line(name, type, comment, fll)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Fmorpho_line *fll;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *fll = (Fmorpho_line) _mw_load_fmorpho_line(fname,type_in);
  if (*fll == NULL) return(-1);  

  _mw_make_type(type,type_in,"fmorpho_line");

  return(0);
}

short _mwsave_fmorpho_line(name, type, type_force, comment, fll)

char *name;
char type[]; /* Default curves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Fmorpho_line fll;

{
  _mw_choose_type(type,type_force,"fmorpho_line");
  if (_mw_create_fmorpho_line(name,fll,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : morpho_set  =====*/

short _mwload_morpho_set(name, type, comment, morpho_set)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Morpho_set *morpho_set;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *morpho_set = (Morpho_set) _mw_load_morpho_set(fname,type_in);
  if (*morpho_set == NULL) return(-1);  

  _mw_make_type(type,type_in,"morpho_set");

  return(0);
}

short _mwsave_morpho_set(name, type, type_force, comment, morpho_set)

char *name;
char type[]; /* Default curves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Morpho_set morpho_set;

{
  _mw_choose_type(type,type_force,"morpho_set");
  if (_mw_create_morpho_set(name,morpho_set,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : morpho_sets  =====*/

short _mwload_morpho_sets(name, type, comment, morpho_sets)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Morpho_sets *morpho_sets;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *morpho_sets = (Morpho_sets) _mw_load_morpho_sets(fname,type_in);
  if (*morpho_sets == NULL) return(-1);  

  _mw_make_type(type,type_in,"morpho_sets");

  return(0);
}

short _mwsave_morpho_sets(name, type, type_force, comment, morpho_sets)

char *name;
char type[]; /* Default curves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Morpho_sets morpho_sets;

{
  _mw_choose_type(type,type_force,"morpho_sets");
  if (_mw_create_morpho_sets(name,morpho_sets,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Mimage  =====*/

short _mwload_mimage(name, type, comment, mimage)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Mimage *mimage;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *mimage = (Mimage) _mw_load_mimage(fname,type_in);
  if (*mimage == NULL) return(-1);  

  format_filename((*mimage)->name,fname);

  strcpy(comment_in,(*mimage)->cmt);
  _mw_make_type(type,type_in,"mimage");
  _mw_make_comment(comment,comment_in);

  return(0);
}

short _mwsave_mimage(name, type, type_force, comment, mimage)

char *name;
char type[]; /* Default curves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Mimage mimage;

{
  _mw_choose_type(type,type_force,"mimage");
  if (mimage->cmt[0] == '?') sprintf(mimage->cmt,"%s(%s)",mwname,comment);

  if (_mw_create_mimage(name,mimage,type) >= 0)
    return(0);
  else
    return(-1);
}


/*===== Internal type : Cmorpho_line  =====*/

short _mwload_cmorpho_line(name, type, comment, ll)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Cmorpho_line *ll;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *ll = (Cmorpho_line) _mw_load_cmorpho_line(fname,type_in);
  if (*ll == NULL) return(-1);  

  _mw_make_type(type,type_in,"cmorpho_line");

  return(0);
}

short _mwsave_cmorpho_line(name, type, type_force, comment, ll)

char *name;
char type[]; /* Default curves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Cmorpho_line ll;

{
  _mw_choose_type(type,type_force,"cmorpho_line");
  if (_mw_create_cmorpho_line(name,ll,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Cfmorpho_line  =====*/

short _mwload_cfmorpho_line(name, type, comment, fll)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Cfmorpho_line *fll;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *fll = (Cfmorpho_line) _mw_load_cfmorpho_line(fname,type_in);
  if (*fll == NULL) return(-1);  

  _mw_make_type(type,type_in,"cfmorpho_line");

  return(0);
}

short _mwsave_cfmorpho_line(name, type, type_force, comment, fll)

char *name;
char type[]; /* Default curves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Cfmorpho_line fll;

{
  _mw_choose_type(type,type_force,"cfmorpho_line");
  if (_mw_create_cfmorpho_line(name,fll,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : cmorpho_set  =====*/

short _mwload_cmorpho_set(name, type, comment, cmorpho_set)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Cmorpho_set *cmorpho_set;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *cmorpho_set = (Cmorpho_set) _mw_load_cmorpho_set(fname,type_in);
  if (*cmorpho_set == NULL) return(-1);  

  _mw_make_type(type,type_in,"cmorpho_set");

  return(0);
}

short _mwsave_cmorpho_set(name, type, type_force, comment, cmorpho_set)

char *name;
char type[]; /* Default curves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Cmorpho_set cmorpho_set;

{
  _mw_choose_type(type,type_force,"cmorpho_set");
  if (_mw_create_cmorpho_set(name,cmorpho_set,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : cmorpho_sets  =====*/

short _mwload_cmorpho_sets(name, type, comment, cmorpho_sets)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Cmorpho_sets *cmorpho_sets;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *cmorpho_sets = (Cmorpho_sets) _mw_load_cmorpho_sets(fname,type_in);
  if (*cmorpho_sets == NULL) return(-1);  

  _mw_make_type(type,type_in,"cmorpho_sets");

  return(0);
}

short _mwsave_cmorpho_sets(name, type, type_force, comment, cmorpho_sets)

char *name;
char type[]; /* Default curves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Cmorpho_sets cmorpho_sets;

{
  _mw_choose_type(type,type_force,"cmorpho_sets");
  if (_mw_create_cmorpho_sets(name,cmorpho_sets,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Cmimage  =====*/

short _mwload_cmimage(name, type, comment, cmimage)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Cmimage *cmimage;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *cmimage = (Cmimage) _mw_load_cmimage(fname,type_in);
  if (*cmimage == NULL) return(-1);  

  format_filename((*cmimage)->name,fname);

  strcpy(comment_in,(*cmimage)->cmt);
  _mw_make_type(type,type_in,"cmimage");
  _mw_make_comment(comment,comment_in);

  return(0);
}

short _mwsave_cmimage(name, type, type_force, comment, cmimage)

char *name;
char type[]; /* Default type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Cmimage cmimage;

{
  _mw_choose_type(type,type_force,"cmimage");
  if (cmimage->cmt[0] == '?') sprintf(cmimage->cmt,"%s(%s)",mwname,comment);

  if (_mw_create_cmimage(name,cmimage,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Shape  =====*/

short _mwload_shape(name, type, comment, shape)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Shape *shape;

{
  char type_in[mw_ftype_size];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *shape = (Shape) _mw_load_shape(fname,type_in);
  if (*shape == NULL) return(-1);  

  _mw_make_type(type,type_in,"shape");
  return(0);
}

short _mwsave_shape(name, type, type_force, comment, shape)

char *name;
char type[]; /* Default type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Shape shape;

{
  _mw_choose_type(type,type_force,"shape");

  if (_mw_create_shape(name,shape,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Shapes  =====*/

short _mwload_shapes(name, type, comment, shapes)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Shapes *shapes;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *shapes = (Shapes) _mw_load_shapes(fname,type_in);
  if (*shapes == NULL) return(-1);  

  format_filename((*shapes)->name,fname);

  strcpy(comment_in,(*shapes)->cmt);
  _mw_make_type(type,type_in,"shapes");
  _mw_make_comment(comment,comment_in);

  return(0);
}

short _mwsave_shapes(name, type, type_force, comment, shapes)

char *name;
char type[]; /* Default type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Shapes shapes;

{
  _mw_choose_type(type,type_force,"shapes");
  if (shapes->cmt[0] == '?') sprintf(shapes->cmt,"%s(%s)",mwname,comment);

  if (_mw_create_shapes(name,shapes,type) >= 0)
    return(0);
  else
    return(-1);
}


/*===== Internal type : Dcurve =====*/

short _mwload_dcurve(name, type, comment, cv)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Dcurve *cv;

{
  char type_in[mw_ftype_size];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *cv = (Dcurve) _mw_load_dcurve(fname,type_in);
  if (*cv == NULL) return(-1);  

  _mw_make_type(type,type_in,"dcurve");

  return(0);
}

short _mwsave_dcurve(name, type, type_force, comment, cv)

char *name;
char type[]; /* Default dcurves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Dcurve cv;

{
  _mw_choose_type(type,type_force,"dcurve");
  if (_mw_create_dcurve(name,cv,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Dcurves =====*/

short _mwload_dcurves(name, type, comment, cv)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Dcurves *cv;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *cv = (Dcurves) _mw_load_dcurves(fname,type_in);
  if (*cv == NULL) return(-1);  

  format_filename((*cv)->name,fname);

  strcpy(comment_in,(*cv)->cmt);
  _mw_make_type(type,type_in,"dcurves");
  _mw_make_comment(comment,comment_in);

  return(0);
}

short _mwsave_dcurves(name, type, type_force, comment, cv)

char *name;
char type[]; /* Default dcurves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Dcurves cv;

{
  _mw_choose_type(type,type_force,"dcurves");
  if (cv->cmt[0] == '?') sprintf(cv->cmt,"%s(%s)",mwname,comment);

  if (_mw_create_dcurves(name,cv,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Rawdata =====*/

short _mwload_rawdata(name, type, comment, rd)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Rawdata *rd;

{
  char type_in[mw_ftype_size];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *rd = (Rawdata) _mw_load_rawdata(fname);
  if (*rd == NULL) return(-1);  
  return(0);
}

short _mwsave_rawdata(name, type, type_force, comment, rd)

char *name;
char type[]; /* Default rawdatas type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Rawdata rd;

{
  if (_mw_create_rawdata(name,rd) >= 0)
    return(0);
  else
    return(-1);
}



/*===== Internal type : Flist =====*/

short _mwload_flist(name, type, comment, lst)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Flist *lst;

{
  char type_in[mw_ftype_size];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *lst = (Flist) _mw_load_flist(fname,type_in);
  if (*lst == NULL) return(-1);  

  _mw_make_type(type,type_in,"flist");

  return(0);
}

short _mwsave_flist(name, type, type_force, comment, lst)

char *name;
char type[]; /* Default dcurves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Flist lst;

{
  _mw_choose_type(type,type_force,"flist");
  if (_mw_create_flist(name,lst,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Flists =====*/

short _mwload_flists(name, type, comment, lsts)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Flists *lsts;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *lsts = (Flists) _mw_load_flists(fname,type_in);
  if (*lsts == NULL) return(-1);  

  format_filename((*lsts)->name,fname);

  strcpy(comment_in,(*lsts)->cmt);
  _mw_make_type(type,type_in,"flists");
  _mw_make_comment(comment,comment_in);

  return(0);
}

short _mwsave_flists(name, type, type_force, comment, lsts)

char *name;
char type[]; /* Default dcurves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Flists lsts;

{
  _mw_choose_type(type,type_force,"flists");
  if (lsts->cmt[0] == '?') sprintf(lsts->cmt,"%s(%s)",mwname,comment);

  if (_mw_create_flists(name,lsts,type) >= 0)
    return(0);
  else
    return(-1);
}


/*===== Internal type : Dlist =====*/

short _mwload_dlist(name, type, comment, lst)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Dlist *lst;

{
  char type_in[mw_ftype_size];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *lst = (Dlist) _mw_load_dlist(fname,type_in);
  if (*lst == NULL) return(-1);  

  _mw_make_type(type,type_in,"dlist");

  return(0);
}

short _mwsave_dlist(name, type, type_force, comment, lst)

char *name;
char type[]; /* Default dcurves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Dlist lst;

{
  _mw_choose_type(type,type_force,"dlist");
  if (_mw_create_dlist(name,lst,type) >= 0)
    return(0);
  else
    return(-1);
}

/*===== Internal type : Dlists =====*/

short _mwload_dlists(name, type, comment, lsts)

char *name;
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Dlists *lsts;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *lsts = (Dlists) _mw_load_dlists(fname,type_in);
  if (*lsts == NULL) return(-1);  

  format_filename((*lsts)->name,fname);

  strcpy(comment_in,(*lsts)->cmt);
  _mw_make_type(type,type_in,"dlists");
  _mw_make_comment(comment,comment_in);

  return(0);
}

short _mwsave_dlists(name, type, type_force, comment, lsts)

char *name;
char type[]; /* Default dcurves type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Dlists lsts;

{
  _mw_choose_type(type,type_force,"dlists");
  if (lsts->cmt[0] == '?') sprintf(lsts->cmt,"%s(%s)",mwname,comment);

  if (_mw_create_dlists(name,lsts,type) >= 0)
    return(0);
  else
    return(-1);
}


/*===== Internal type : Wpack2d =====*/

short _mwload_wpack2d(name, type, comment, pack)

char *name; /* Name of the 2D wavelet packet decomposition */
char type[]; /* Input: old default type; Output: new default type */
char comment[]; /* Input: old comments; Output: new comments */
Wpack2d *pack;

{
  char type_in[mw_ftype_size];
  char comment_in[BUFSIZ];
  char fname[BUFSIZ];

  strcpy(fname,name);      /* Do Not Change the value of name */
  search_filename(fname);
  *pack = (Wpack2d) _mw_load_wpack2d(fname, type_in);
  if (*pack == NULL) return(-1);  

  format_filename((*pack)->name,fname);

  strcpy(comment_in,(*pack)->cmt);
  _mw_make_type(type,type_in,"wpack2d");
  _mw_make_comment(comment,comment_in);
  return(0);
}

short _mwsave_wpack2d(name, type, type_force, comment, pack)

char *name; /* Name of the 2D wavelet packet decomposition */
char type[]; /* Default pack type for the output file */
char type_force[]; /* Overwrite the default type file, if not '?' */
char comment[];
Wpack2d pack;

{
  _mw_choose_type(type,type_force,"wpack2d");
  if (pack->cmt[0] == '?') sprintf(pack->cmt,"%s(%s)",mwname,comment);

  if (_mw_create_wpack2d(name,pack,type) >= 0)
    return(0);
  else
    return(-1);
}

