/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  module_io.c
   
  Vers. 1.3
  Author : Jacques Froment
  Input/Output functions for the module & modules structures

  Main changes :
  v1.3 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
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

#include "libmw-defs.h"
#include "utils.h"

#include "module.h"
#include "ascii_file.h"

#include "module_io.h"

/* Read an entire line (until \n). Return 0 if EOF. */

static int readline(FILE *fp, char *string)
{
     register int i,c;
     int imax;

     i=0; imax = BUFSIZ-1;
     while (((c=getc(fp)) != EOF) && (((char) c) != '\n') && (i<imax)) 
	  string[i++]=(char) c;
     string[i] = '\0';
     if (c == EOF) return(0); else return(1);
}
  
static int no_information(char *string)
{
     int i;

     if ((string[0] == '%') || (string[0] == '\0')) return(1);
     i=0;
     while (string[i] != '\0') if ((string[i] != ' ') && (string[i] != '\t'))
				    return(0);
     return(1);
}
  
/* Read a entire line, ignoring the comments (beginning with %) and blank
   lines. Return 0 if EOF. 
   Do not read the line if the variable string is not empty.
*/

static int _mw_readline(FILE *fp, char *string)
{
     int i;

     if (string[0] != '\0') return(1);
     do i=readline(fp,string); while ((i!=0)&&(no_information(string)==1));
     return(i);
}

/* Convert group name :
   - Remove the path in a file list. Ex: group1/subgroup1 -> subgroup1
   - Change characters _ to <space>
*/

static void convert_groupname(char *string)
{
     char buff[BUFSIZ];
     register int i;

     /* Remove path */
     strcpy(buff,string);
     for (i=strlen(buff); (i >= 0) && (buff[i] != '/') ; i--);
     /* FIXME: wrong types, dirty temporary fix */
     if ((i>=0) && (i< (int) strlen(buff))) strcpy(string,&buff[i+1]);

     /* Replace '_' by space */
     for (i=0;i<= (int) strlen(string);i++) 
	  if (string[i] == '_') string[i] = ' ';
  
}

/* Load modules located at one directory level given in newm->name */

short _mw_load_submodules(FILE *fp, Module upm, char *line)
{
     Module oldm,newm;
     char dir[mw_namesize];  /* directory level name */

     strcpy(dir,upm->name);  

/*  
    printf("[_mw_load_submodules]: dir=%s  line=%s\n",dir,line);
*/
  
     oldm = NULL;
     while ((_mw_readline(fp,line)!=0)  &&  
	    ( (strncmp(line,"#group",6) != 0) || ((char *)strstr(line,dir) != NULL) ))
	  /* Detect a new group which is not a subgroup of dir */
     {      
	  if (strncmp(line,"#dir",4) == 0) {line[0]='\0'; continue;}
 
	  newm = mw_new_module();
	  if (!newm) return(1);
	  if (upm->down == NULL) upm->down = newm;
	  if (oldm != NULL) oldm->next = newm;
	  newm->previous = oldm;
	  newm->next = NULL;
	  newm->up = upm;
	  newm->down = NULL;
	  oldm = newm;

	  if (strncmp(line,"#group",6) == 0) /* A subgroup */
	  {
	       strcpy(newm->name,&line[7]);
	       convert_groupname(newm->name);
	       newm->type = 'g';
	       line[0] = '\0';
/*
  printf("[_mw_load_submodules] : call recursively _mw_load_submodules\n");
*/
	       if (_mw_load_submodules(fp,newm,line)!=0) 
	       {
/*
  printf("Fin de _mw_load_submodules avec != 0\n");
*/
		    return(1);                              
	       }
	  }
	  else  /* A module */
	  {
	       strcpy(newm->name,line);
	       newm->type = 'm';
	       line[0] = '\0';
	  }
     } 
     if ( (strncmp(line,"#group",6) != 0) || ((char *)strstr(line,dir) != NULL) )
	  /* NOT A New group which is not a subgroup of dir */
	  line[0] = '\0';

/*
  printf("Fin de _mw_load_submodules avec = 0\n");
*/
     return(0);
}

Modules _mw_load_modules(char *fname)
{
     FILE    *fp;
     Modules modules;
     Module newm,oldm;
     char line[BUFSIZ];

     if (!(fp = fopen(fname, "r")))
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  return(NULL);
     }

     modules = mw_new_modules();
     if (modules == NULL) return(modules);
     oldm = NULL;
     line[0] = '\0';
  
     while (_mw_readline(fp,line)!=0)
     {
	  if (strncmp(line,"#dir",4) == 0) {line[0]='\0'; continue;}
      
	  if (strncmp(line,"#group",6) != 0) 
	  {
	       mwerror(ERROR,1,
		       "[_mw_load_modules]: Expecting #group field at the top level instead of \"%s\"\n",
		       line);
	       mw_delete_modules(modules);
	       fclose(fp);
	       return(NULL);
	  }
	  if ((char *)strpbrk(line,"/") != NULL)
	  {
	       mwerror(ERROR,1,
		       "[_mw_load_modules]: Subgroup \"%s\" not allowed at the top level\n",
		       line);
	       mw_delete_modules(modules);
	       fclose(fp);
	       return(NULL);        
	  }
	  newm = mw_new_module();
	  if (!newm) {
	       mw_delete_modules(modules);
	       fclose(fp);
	       return(NULL);
	  }
	  strcpy(newm->name,&line[7]);
	  convert_groupname(newm->name);
	  newm->type = 'G';
	  if (modules->first == NULL) modules->first = newm;
	  if (oldm != NULL) oldm->next = newm;
	  newm->previous = oldm;
	  newm->next = newm-> up = newm-> down = NULL;

	  line[0] = '\0';
	  if (_mw_load_submodules(fp,newm,line)!=0) 
	  {
	       mw_delete_modules(modules);
	       fclose(fp);
	       return(NULL);             
	  }

	  oldm = newm;
/*
  printf("[_mw_load_modules]: fin de boucle. line=%s\n",line);
*/
     } 
     fclose(fp);
     return(modules);
}

/* Write one level of directory */

void _mw_write_submodules(FILE *fp, Module levelm, char *groupid)
{
     int Group_nb;
     char subgroupid[BUFSIZ];
     Module module;

     if (!levelm)
	  mwerror(INTERNAL,1,"[_mw_write_submodules]: NULL levelm input (maybe a group without any modules) !\n");

     /* Step 1 */
     for (module=levelm,Group_nb=1; module; module=module->next)
     {
	  if (module->type == 'g')
	  {
	       if (!module->down)
	       {
		    mwerror(ERROR,1,"Group \"%s\" is empty !\n",module->name);
		    return;
	       }
	       sprintf(subgroupid,"%s_%d",groupid,Group_nb);
	       _mw_write_submodules(fp,module->down,subgroupid);
	       Group_nb++;
	  }
     }
        
     /* Step 2 : external declarations of the functions */
     for (module=levelm,Group_nb=1; module; module=module->next)  
	  if (module->type == 'm')
	       fprintf(fp,"extern _mw2_%s();\n",module->name);


     /* Step 3 */
     fprintf(fp,"static _MW_OPTION %s[] = {\n",groupid);
     for (module=levelm,Group_nb=1; module; module=module->next)
     {
	  if (module->type == 'g') 
	  {
	       fprintf(fp,"{_MW_MENU,\"%s\",%s_%d},\n",module->name,groupid,Group_nb);
	       Group_nb++;
	  }
	  if (module->type == 'm')
	  {
	       fprintf(fp,"{_MW_BUTTON,\"%s\",_mw2_%s},\n",module->name,module->name);
	       /* Uncomment this line to generate prototype in the output */
	       /* printf("_mw2_%s() {};\n",module->name); */
	  }
	  if (module->type == 'G') 
	  {
	       mwerror(INTERNAL,1,
		       "[_mw_write_submodules]: type G for %s is not allowed at bottom levels\n",
		       module->name);
	  }
     }
     fprintf(fp,"{0,NULL,NULL}\n   };\n");	

}

/* Write the modules structure in an ascii file which is not a MegaWave2 */
/* Data Ascii file ! It is part of the C source of the XMegaWave2 main program ! */

short _mw_create_modules(char *fname, Modules modules)
{
     FILE *fp;
     Module upm;
     int Group_nb;
     char groupid[BUFSIZ];

     if (modules == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_modules]: Cannot create file: Modules structure is NULL\n");

     if (modules->first == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_modules]: Cannot create file: No Module in the Modules structure\n");

     if (!(fp = fopen(fname, "w")))
     {
	  mwerror(ERROR, 0,"Cannot create the file \"%s\"\n",fname);
	  return(-1);
     }
    
     /* Step 1 */
     for (upm=modules->first,Group_nb=1; upm; upm=upm->next,Group_nb++)
     {
	  if (upm->type != 'G') 
	  {     
	       mwerror(ERROR,1,
		       "[_mw_create_modules]: type %c for %s is not allowed at the top level\n",
		       upm->type,upm->name);
	       return(-1);
	  }
      	
	  if (!upm->down)
	  {
	       mwerror(ERROR,1,"Group \"%s\" is empty !\n",upm->name);
	       return -1;
	  }
	  sprintf(groupid,"Group%d",Group_nb);
	  _mw_write_submodules(fp,upm->down,groupid);
     }
     
     /* Step 2 */
     fprintf(fp,"\n");
     for (upm=modules->first,Group_nb=1; upm; upm=upm->next,Group_nb++)
     {
	  if (upm == modules->first) 
	       fprintf(fp,"mwrunmode=2;\n_mw_InitXMW (argc, argv);\n");
	  fprintf(fp,"_mw_AddNewMenu(\"%s\",Group%d);\n",upm->name,Group_nb);
	  if (!upm->next) fprintf(fp,"_mw_EndXMW(argc,argv);\n");
     }
      
     fclose(fp);
     return(0);
}
