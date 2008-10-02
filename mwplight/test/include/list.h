/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   list.h
   
   Vers. 1.1
   (C) 2001 Lionel Moisan
   Definition of the Flist(s)/Dlist(s) internal types
   Modified by JF : errors handling and alloc_size -> max_size.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef list_flg
#define list_flg

#ifdef SunOS
#include <sys/types.h>
#endif

/* Memory growth rate for reallocations */
#define MW_LIST_ENLARGE_FACTOR 1.5  


/* Conventions:
- a valid F/Dlist(s) should not be NULL
- an emty F/Dlist(s) has size=0
*/

/* Float List and Lists */

typedef struct flist {

  int size;          /* size (number of elements) */
  int max_size;      /* currently allocated size (number of ELEMENTS) */
  int dim;           /* dimension (number of components per element) */

  float *values;     /* values = size * dim array
			  nth element = values[n*dim+i], i=0..dim-1 */

  int data_size;     /* size of data[] in bytes */
  void* data;        /* User defined field (saved). A pointer to something */

} *Flist;


typedef struct flists {

  char cmt[mw_cmtsize];     /* Comments */
  char name[mw_namesize];   /* Name */

  int size;          /* size (number of elements) */
  int max_size;      /* currently allocated size (number of ELEMENTS) */

  Flist *list;       /* array of Flist */

  int data_size;     /* size of data[] in bytes */
  void* data;        /* User defined field (saved). A pointer to something */

} *Flists;


/* Double List and Lists */

typedef struct dlist {

  int size;          /* size (number of elements) */
  int max_size;      /* currently allocated size (number of ELEMENTS) */
  int dim;           /* dimension (number of components per element) */

  double *values;    /* values = size * dim array
			  nth element = values[n*dim+i], i=0..dim-1 */

  int data_size;     /* size of data[] in bytes */
  void* data;        /* User defined field (saved). A pointer to something */

} *Dlist;


typedef struct dlists {

  char cmt[mw_cmtsize];     /* Comments */
  char name[mw_namesize];   /* Name */

  int size;          /* size (number of elements) */
  int max_size;      /* currently allocated size (number of ELEMENTS) */

  Dlist *list;       /* array of Dlist */

  int data_size;     /* size of data[] in bytes */
  void* data;        /* User defined field (saved). A pointer to something */

} *Dlists;


/* Functions definition */

#ifdef __STDC__
 
Flist mw_new_flist(void);
Flist mw_realloc_flist(Flist,int);
Flist mw_enlarge_flist(Flist);
Flist mw_change_flist(Flist,int,int,int);
void mw_delete_flist(Flist);
void mw_clear_flist(Flist,float);
Flist mw_copy_flist(Flist,Flist);

Flists mw_new_flists();
Flists mw_realloc_flists(Flists,int);
Flists mw_enlarge_flists(Flists);
Flists mw_change_flists(Flists,int,int);
void mw_delete_flists(Flists);
Flists mw_copy_flists(Flists,Flists);

Dlist mw_new_dlist(void);
Dlist mw_realloc_dlist(Dlist,int);
Dlist mw_enlarge_dlist(Dlist);
Dlist mw_change_dlist(Dlist,int,int,int);
void mw_delete_dlist(Dlist);
void mw_clear_dlist(Dlist,double);
Dlist mw_copy_dlist(Dlist,Dlist);

Dlists mw_new_dlists();
Dlists mw_realloc_dlists(Dlists,int);
Dlists mw_enlarge_dlists(Dlists);
Dlists mw_change_dlists(Dlists,int,int);
void mw_delete_dlists(Dlists);
Dlists mw_copy_dlists(Dlists,Dlists);

#else

Flist mw_new_flist();
Flist mw_realloc_flist();
Flist mw_enlarge_flist();
Flist mw_change_flist();
void mw_delete_flist();
void mw_clear_flist();
Flist mw_copy_flist();

Flists mw_new_flists();
Flists mw_realloc_flists();
Flists mw_enlarge_flists();
Flists mw_change_flists();
void mw_delete_flists();
Flists mw_copy_flists();

Dlist mw_new_dlist();
Dlist mw_realloc_dlist();
Dlist mw_enlarge_dlist();
Dlist mw_change_dlist();
void mw_delete_dlist();
void mw_clear_dlist();
Dlist mw_copy_dlist();

Dlists mw_new_dlists();
Dlists mw_realloc_dlists();
Dlists mw_enlarge_dlists();
Dlists mw_change_dlists();
void mw_delete_dlists();
Dlists mw_copy_dlists();

#endif

#endif
