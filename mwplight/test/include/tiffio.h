/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994 Sam Leffler
 * Copyright (c) 1991, 1992, 1993, 1994 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 * 
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/* Modified by JF 3/11/95 - Add functions declaration for non-Ansi compilers */

#ifndef _TIFFIO_
#define	_TIFFIO_

/*
 * TIFF I/O Library Definitions.
 */
#include "tiff.h"

/*
 * TIFF is defined as an incomplete type to hide the
 * library's internal data structures from clients.
 */
typedef	struct tiff TIFF;

/*
 * The following typedefs define the intrinsic size of
 * data types used in the *exported* interfaces.  These
 * definitions depend on the proper definition of types
 * in tiff.h.  Note also that the varargs interface used
 * pass tag types and values uses the types defined in
 * tiff.h directly.
 *
 * NB: ttag_t is unsigned int and not unsigned short because
 *     ANSI C requires that the type before the ellipsis be a
 *     promoted type (i.e. one of int, unsigned int, pointer,
 *     or double).
 * NB: tsize_t is int32 and not uint32 because some functions
 *     return -1.
 * NB: toff_t is not off_t for many reasons; TIFFs max out at
 *     32-bit file offsets being the most important
 */
typedef	unsigned int ttag_t;	/* directory tag */
typedef	uint16 tdir_t;		/* directory index */
typedef	uint16 tsample_t;	/* sample number */
typedef	uint32 tstrip_t;	/* strip number */
typedef uint32 ttile_t;		/* tile number */
typedef	int32 tsize_t;		/* i/o size in bytes */
typedef	void* tdata_t;		/* image data ref */
typedef	void* thandle_t;	/* client data handle */
typedef	int32 toff_t;		/* file offset */

/*
 * Flags to pass to TIFFPrintDirectory to control
 * printing of data structures that are potentially
 * very large.   Bit-or these flags to enable printing
 * multiple items.
 */
#define	TIFFPRINT_NONE		0x0		/* no extra info */
#define	TIFFPRINT_STRIPS	0x1		/* strips/tiles info */
#define	TIFFPRINT_CURVES	0x2		/* color/gray response curves */
#define	TIFFPRINT_COLORMAP	0x4		/* colormap */
#define	TIFFPRINT_JPEGQTABLES	0x100		/* JPEG Q matrices */
#define	TIFFPRINT_JPEGACTABLES	0x200		/* JPEG AC tables */
#define	TIFFPRINT_JPEGDCTABLES	0x200		/* JPEG DC tables */

/*
 * Macros for extracting components from the
 * packed ABGR form returned by TIFFReadRGBAImage.
 */
#define	TIFFGetR(abgr)	((abgr) & 0xff)
#define	TIFFGetG(abgr)	(((abgr) >> 8) & 0xff)
#define	TIFFGetB(abgr)	(((abgr) >> 16) & 0xff)
#define	TIFFGetA(abgr)	(((abgr) >> 24) & 0xff)


/*----------- Function prototypes for ANSI compilers -------------*/

#ifdef __STDC__

#include <stdarg.h>

#if defined(__cplusplus)
extern "C" {
#endif
typedef	void (*TIFFErrorHandler)(const char* module, const char* fmt, va_list);
typedef	tsize_t (*TIFFReadWriteProc)(thandle_t, tdata_t, tsize_t);
typedef	toff_t (*TIFFSeekProc)(thandle_t, toff_t, int);
typedef	int (*TIFFCloseProc)(thandle_t);
typedef	toff_t (*TIFFSizeProc)(thandle_t);
typedef	int (*TIFFMapFileProc)(thandle_t, tdata_t*, toff_t*);
typedef	void (*TIFFUnmapFileProc)(thandle_t, tdata_t, toff_t);

extern	const char* TIFFGetVersion(void);

extern	void TIFFClose(TIFF*);
extern	int TIFFFlush(TIFF*);
extern	int TIFFFlushData(TIFF*);
extern	int TIFFGetField(TIFF*, ttag_t, ...);
extern	int TIFFVGetField(TIFF*, ttag_t, va_list);
extern	int TIFFGetFieldDefaulted(TIFF*, ttag_t, ...);
extern	int TIFFVGetFieldDefaulted(TIFF*, ttag_t, va_list);
extern	int TIFFReadDirectory(TIFF*);
extern	tsize_t TIFFScanlineSize(TIFF*);
extern	tsize_t TIFFStripSize(TIFF*);
extern	tsize_t TIFFVStripSize(TIFF*, uint32);
extern	tsize_t TIFFTileRowSize(TIFF*);
extern	tsize_t TIFFTileSize(TIFF*);
extern	tsize_t TIFFVTileSize(TIFF*, uint32);
extern	int TIFFFileno(TIFF*);
extern	int TIFFGetMode(TIFF*);
extern	int TIFFIsTiled(TIFF*);
extern	int TIFFIsByteSwapped(TIFF*);
extern	uint32 TIFFCurrentRow(TIFF*);
extern	tdir_t TIFFCurrentDirectory(TIFF*);
extern	tstrip_t TIFFCurrentStrip(TIFF*);
extern	ttile_t TIFFCurrentTile(TIFF*);
extern	int TIFFReadBufferSetup(TIFF*, tdata_t, tsize_t);
extern	int TIFFLastDirectory(TIFF*);
extern	int TIFFSetDirectory(TIFF*, tdir_t);
extern	int TIFFSetSubDirectory(TIFF*, uint32);
extern	int TIFFUnlinkDirectory(TIFF*, tdir_t);
extern	int TIFFSetField(TIFF*, ttag_t, ...);
extern	int TIFFVSetField(TIFF*, ttag_t, va_list);
extern	int TIFFWriteDirectory(TIFF *);
#if defined(c_plusplus) || defined(__cplusplus)
extern	void TIFFPrintDirectory(TIFF*, FILE*, long = 0);
extern	int TIFFReadScanline(TIFF*, tdata_t, uint32, tsample_t = 0);
extern	int TIFFWriteScanline(TIFF*, tdata_t, uint32, tsample_t = 0);
extern	int TIFFReadRGBAImage(TIFF*, uint32, uint32, uint32*, int stop = 0);
#else
extern	void TIFFPrintDirectory(TIFF*, FILE*, long);
extern	int TIFFReadScanline(TIFF*, tdata_t, uint32, tsample_t);
extern	int TIFFWriteScanline(TIFF*, tdata_t, uint32, tsample_t);
extern	int TIFFReadRGBAImage(TIFF*, uint32, uint32, uint32*, int stop);
#endif
extern	TIFF* TIFFOpen(const char*, const char*);
extern	TIFF* TIFFFdOpen(int, const char*, const char*);
extern	TIFF* TIFFClientOpen(const char* name, const char* mode,
	    thandle_t clientdata,
	    TIFFReadWriteProc readproc, TIFFReadWriteProc writeproc,
	    TIFFSeekProc seekproc, TIFFCloseProc closeproc,
	    TIFFSizeProc sizeproc,
	    TIFFMapFileProc mapproc, TIFFUnmapFileProc unmapproc);
extern	const char* TIFFFileName(TIFF*);
extern	void TIFFError(const char*, const char*, ...);
extern	void TIFFWarning(const char*, const char*, ...);
extern	TIFFErrorHandler TIFFSetErrorHandler(TIFFErrorHandler);
extern	TIFFErrorHandler TIFFSetWarningHandler(TIFFErrorHandler);
extern	ttile_t TIFFComputeTile(TIFF*, uint32, uint32, uint32, tsample_t);
extern	int TIFFCheckTile(TIFF*, uint32, uint32, uint32, tsample_t);
extern	ttile_t TIFFNumberOfTiles(TIFF*);
extern	tsize_t TIFFReadTile(TIFF*,
	    tdata_t, uint32, uint32, uint32, tsample_t);
extern	tsize_t TIFFWriteTile(TIFF*,
	    tdata_t, uint32, uint32, uint32, tsample_t);
extern	tstrip_t TIFFComputeStrip(TIFF*, uint32, tsample_t);
extern	tstrip_t TIFFNumberOfStrips(TIFF*);
extern	tsize_t TIFFReadEncodedStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFReadRawStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFReadEncodedTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	tsize_t TIFFReadRawTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteEncodedStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteRawStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteEncodedTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteRawTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	void TIFFSetWriteOffset(TIFF*, toff_t);
extern	void TIFFSwabShort(uint16 *);
extern	void TIFFSwabLong(uint32 *);
extern	void TIFFSwabArrayOfShort(uint16 *, unsigned long);
extern	void TIFFSwabArrayOfLong(uint32 *, unsigned long);
extern	void TIFFReverseBits(unsigned char *, unsigned long);
extern	const unsigned char* TIFFGetBitRevTable(int);

extern	void TIFFModeCCITTFax3(TIFF* tif, int isClassF);	/* XXX */
#if defined(__cplusplus)
}
#endif

#else

/*----------- Function prototypes for non-ANSI compilers -------------*/

typedef	void (*TIFFErrorHandler)();
typedef	tsize_t (*TIFFReadWriteProc)();
typedef	toff_t (*TIFFSeekProc)();
typedef	int (*TIFFCloseProc)();
typedef	toff_t (*TIFFSizeProc)();
typedef	int (*TIFFMapFileProc)();
typedef	void (*TIFFUnmapFileProc)();

extern	char* TIFFGetVersion();

extern	void TIFFClose();
extern	int TIFFFlush();
extern	int TIFFFlushData();
extern	int TIFFGetField();
extern	int TIFFVGetField();
extern	int TIFFGetFieldDefaulted();
extern	int TIFFVGetFieldDefaulted();
extern	int TIFFReadDirectory();
extern	tsize_t TIFFScanlineSize();
extern	tsize_t TIFFStripSize();
extern	tsize_t TIFFVStripSize();
extern	tsize_t TIFFTileRowSize();
extern	tsize_t TIFFTileSize();
extern	tsize_t TIFFVTileSize();
extern	int TIFFFileno();
extern	int TIFFGetMode();
extern	int TIFFIsTiled();
extern	int TIFFIsByteSwapped();
extern	uint32 TIFFCurrentRow();
extern	tdir_t TIFFCurrentDirectory();
extern	tstrip_t TIFFCurrentStrip();
extern	ttile_t TIFFCurrentTile();
extern	int TIFFReadBufferSetup();
extern	int TIFFLastDirectory();
extern	int TIFFSetDirectory();
extern	int TIFFSetSubDirectory();
extern	int TIFFUnlinkDirectory();
extern	int TIFFSetField();
extern	int TIFFVSetField();
extern	int TIFFWriteDirectory();

extern	void TIFFPrintDirectory();
extern	int TIFFReadScanline();
extern	int TIFFWriteScanline();
extern	int TIFFReadRGBAImage();

extern	TIFF* TIFFOpen();
extern	TIFF* TIFFFdOpen();
extern	TIFF* TIFFClientOpen();
extern	char* TIFFFileName();
extern	void TIFFError();
extern	void TIFFWarning();
extern	TIFFErrorHandler TIFFSetErrorHandler();
extern	TIFFErrorHandler TIFFSetWarningHandler();
extern	ttile_t TIFFComputeTile();
extern	int TIFFCheckTile();
extern	ttile_t TIFFNumberOfTiles();
extern	tsize_t TIFFReadTile();
extern	tsize_t TIFFWriteTile();
extern	tstrip_t TIFFComputeStrip();
extern	tstrip_t TIFFNumberOfStrips();
extern	tsize_t TIFFReadEncodedStrip();
extern	tsize_t TIFFReadRawStrip();
extern	tsize_t TIFFReadEncodedTile();
extern	tsize_t TIFFReadRawTile();
extern	tsize_t TIFFWriteEncodedStrip();
extern	tsize_t TIFFWriteRawStrip();
extern	tsize_t TIFFWriteEncodedTile();
extern	tsize_t TIFFWriteRawTile();
extern	void TIFFSetWriteOffset();
extern	void TIFFSwabShort();
extern	void TIFFSwabLong();
extern	void TIFFSwabArrayOfShort();
extern	void TIFFSwabArrayOfLong();
extern	void TIFFReverseBits();
extern	unsigned char* TIFFGetBitRevTable();

extern	void TIFFModeCCITTFax3();	/* XXX */

#endif

#endif /* _TIFFIO_ */
