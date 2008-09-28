/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   module.h
   
   Vers. 1.1
   (C) 1994 Jacques Froment
   Internal Input/Output module & modules structures

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef module_flg
#define module_flg

#ifdef SunOS
#include <sys/types.h>
#endif

/* One MegaWave2 Module */
/* This structure may contain true module or a group name, organized in
   the hierarchy of sub-directories in the src directory. This structure
   is basically used to generates the menu panels of XMegaWave2. 
*/

typedef struct module {
  char name[mw_namesize]; /* Name of the module or of the group (node) */
  char type;              /* type of the node : */
  			  /* 'G' for main groups */
  			  /* 'g' for subgroups */
  			  /* 'm' for modules */
  
  struct module *next;    /* Pointer to the next node (may be NULL) */
  			  /* (same level in the directory) */
  struct module *previous;/* Pointer to the previous node (may be NULL) */
  struct module *down;    /* Pointer to the first node in the subdirectory */
  			  /* (may be NULL) */
  struct module *up;      /* Pointer to the first node in the parent dir. */
                          /* (may be NULL) */

} *Module;


typedef struct modules {
  char cmt[mw_cmtsize];   /* Comments */
  char name[mw_namesize]; /* Name of the file */
  struct module *first;   /* Pointer to the first module of the chain */
  } *Modules;

/* Functions definition */

#ifdef __STDC__

Module mw_new_module(void);
Module mw_change_module(Module);
void mw_delete_module(Module);
Modules mw_new_modules(void);
Modules mw_change_modules(Modules);
void mw_delete_modules(Modules);

#else

Module mw_new_module();
Module mw_change_module();
void mw_delete_module();
Modules mw_new_modules();
Modules mw_change_modules();
void mw_delete_modules();

#endif

#endif
