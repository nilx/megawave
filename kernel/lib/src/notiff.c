/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  notiff.c

  Vers. 1.0
  (C) 1995 Jacques Froment
  Function Definitions of the TIFF library, so users who do not have
  libtiff can still compile MegaWave2 modules.

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "mw.h"
#include "tiffio.h"

/* TIFFOpen function must be the first TIFF function called by the MegaWave2 
   library. If it returns NULL, no other TIFF functions should be called.
*/

TIFF* TIFFOpen(const char * a, const char * b)
{
     mwerror(WARNING,0,"Cannot use TIFF format: libtiff has not been loaded\n");
     return(NULL);
}

void TIFFClose(TIFF* a) {};
int TIFFGetField(TIFF* a, ttag_t b, ...)  {};
int TIFFGetFieldDefaulted(TIFF* a, ttag_t b , ...) {};
tsize_t TIFFScanlineSize(TIFF* a) {};
tsize_t TIFFStripSize(TIFF* a) {};
int TIFFIsTiled(TIFF* a) {};
void TIFFError(const char* a, const char* b, ...) {};
void TIFFWarning(const char* a, const char* b, ...){};
tstrip_t TIFFComputeStrip(TIFF* a, uint32 b, tsample_t c){};
tsize_t TIFFReadEncodedStrip(TIFF* a, tstrip_t b, tdata_t c, tsize_t d){};
tsize_t TIFFWriteEncodedStrip(TIFF* a, tstrip_t b, tdata_t c, tsize_t d){};
const char* TIFFFileName(TIFF* a){};
tsize_t TIFFTileSize(TIFF* a){};
int TIFFSetField(TIFF* a, ttag_t b, ...){};
TIFFErrorHandler TIFFSetErrorHandler(TIFFErrorHandler a){};
TIFFErrorHandler TIFFSetWarningHandler(TIFFErrorHandler b ){};
tsize_t TIFFReadTile(TIFF* a,
		     tdata_t b, uint32 c, uint32 d, uint32 e, tsample_t f){};
