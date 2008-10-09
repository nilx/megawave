/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  module.c
   
  Vers. 1.1
  Author : Jacques Froment
  Basic memory routines for the module & modules internal type

  Main changes :
  v1.1 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdlib.h>
#include <string.h>

#include "libmw-defs.h"
#include "utils.h"


#include "module.h"

/* --- Module --- */

/* Creates a new module structure */

Module mw_new_module(void)
{
     Module module;

     if(!(module = (Module) (malloc(sizeof(struct module)))))
     {
	  mwerror(ERROR, 0, "[mw_new_module] Not enough memory\n");
	  return(NULL);
     }

     module->next = module->previous = module->down = module->up = NULL;
     strcpy(module->name,"?");
     module->type='?';
     return(module);
}

/* Define the struct if it's not defined */

Module mw_change_module(Module module)
{
     if (module == NULL) module = mw_new_module();
     return(module);
}

/* desallocate the module structure into a chain of modules */
/* (delete only this one and not the chain of modules) */

void mw_delete_module(Module module)
{
     if (module == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_module] cannot delete : module structure is NULL\n");
	  return;
     }

     if (module->next) (module->next)->previous = NULL;
     if (module->previous) (module->previous)->next = NULL;
     if (module->up) (module->up)->down = NULL;
     if (module->down) (module->down)->up = NULL;
     module->next = module->previous = module->down = module->up = NULL;
     strcpy(module->name,"?");
     module->type='?';
     free(module);
     module=NULL;
}


/* --- Modules --- */

/* Creates a new modules structure */

Modules mw_new_modules(void)
{
     Modules modules;

     if(!(modules = (Modules) (malloc(sizeof(struct modules)))))
     {
	  mwerror(ERROR, 0, "[mw_new_modules] Not enough memory\n");
	  return(NULL);
     }

     strcpy(modules->cmt,"?");
     strcpy(modules->name,"?");
     return(modules);
}

/* Define the struct if it's not defined */

Modules mw_change_modules(Modules modules)
{
     if (modules == NULL) modules = mw_new_modules();
     return(modules);
}

/* desallocate all the modules of the chain <modules> */

void mw_delete_modules(Modules modules)
{
     Module module,module_up,module_previous;
  
     if (modules == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_modules] cannot delete chain of modules: modules structure is NULL\n");
	  return;
     }

     module = modules->first;
     while (module)
     { /* delete modules down to up */
	  while (module->down) module=module->down;
	  module_up = module->up;     
	  while (module)
	  {
	       while (module->next) module=module->next;        
	       module_previous = module->previous;
	       mw_delete_module(module);
	       module = module_previous;
	  }
	  module = module_up;
     }
     free(modules);
     modules=NULL;
}
