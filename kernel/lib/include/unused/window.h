/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   window.h
   
   Vers. 1.1
   (C) 1993-2001 Jacques Froment
   Interconnexion between the Wdevice Library and MegaWave2

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef window_flg
#define window_flg

#include "Wdevice.h"
#include "wpanel.h"

#ifdef __STDC__
Wframe *mw_get_window(Wframe *,int, int, int, int, char *);
void mw_window_notify(Wframe *, void *, int (*)());
void mw_window_main_loop(void);
#else
Wframe *mw_get_window();
void mw_window_notify();
void mw_window_main_loop();
#endif

#endif
