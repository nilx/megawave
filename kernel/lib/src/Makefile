# Warning : this file has been automatically generated from ./Makefile.in using macro mwconfig V.1.08 with arch = 'Linux ix86' option = 'jpeg shared '
#-----------------------------------------------------------------------------
# Makefile.in used to create Makefile using macro mw2config
# Version 2.1
# MegaWave2 system library : Copyright (c) Jacques Froment 1996-2002
#-----------------------------------------------------------------------------
# This file is part of the MegaWave2 system library 
# MegaWave2 is a "soft-publication" for the scientific community. It has
# been developed for research purposes and it comes without any warranty.
# The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
# CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
#       94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
#-----------------------------------------------------------------------------

#--- Optimized or debug mode 
CCMODE	= -O


MWINCDIR = ../include
DESTDIR  = $(PRIVATE_MEGAWAVE2)/sys
SYSDESTDIR =$(PUBLIC_MEGAWAVE2)/sys
LIBTARGET = $(PRIVATE_MEGAWAVE2)/kernel_obj/lib/lib/$(MW_MACHINETYPE)
OBJPATH  = $(PRIVATE_MEGAWAVE2)/kernel_obj/lib/obj/$(MW_MACHINETYPE)


# Library names - Static

# Library names - Shared
LIBslSYSMW   = $(LIBTARGET)/libsysmw.so
LIBslNOTIFF = $(LIBTARGET)/libnotiff.so
LIBslXMWCOM  = $(LIBTARGET)/libxmwcom.so

# --- C compiler Options ---

# JPEG flag
CJPEG	=
CJPEG   = -DJPEG 

# XMWP : Add code for communication with XMegaWave (+ compile the lib xmwcom)

#CFLAGS   = -I$(MWINCDIR) -I$(MW_INCLUDEX11) -D$(MW_SYSTEMTYPE) -D$(MW_MACHINETYPE) -DXMWP $(CJPEG)
CFLAGS   = -I$(MWINCDIR) -I$(MW_INCLUDEX11) -D$(MW_SYSTEMTYPE) -D$(MW_MACHINETYPE) $(CJPEG)

CFLAGSLIB  = $(CFLAGS) -c 

# Compiler version Sun Solaris (static or shared library)

#--- The C compiler to use, version "static library"


# Compiler version Sun OS 4.x, HP-UX 9.x, Linux and SGI IRIX

# Compiler Version HP-UX 10.x 

#--- The C compiler to use, version "shared library"


# Version HP-UX 10.x

# Version Sun OS 4.x 

# Version Silicon Graphics IRIX

# Version GCC (ex. : PC Linux)
CC = gcc $(CCMODE) -fpic

# Archiver
ARFLAGS  = rcv
AR       = ar $(ARFLAGS)

# Library object
SRC = mw.c mwio.c cimage.c \
      fimage.c cimage_io.c fimage_io.c window.c wpanel.c \
      cmovie.c cmovie_io.c polygon.c wave_io.c \
      polygon_io.c fsignal.c fsignal_io.c ascii_file.c file_type.c \
      wtrans2d.c wtrans2d_io.c basic_conv.c type_conv.c wmax2d.c wmax2d_io.c \
      wtrans1d.c wtrans1d_io.c pm_io.c gif_io.c curve.c curve_io.c \
      cfimage.c cfimage_io.c ccimage.c ccimage_io.c unix_bsd.c \
      module.c module_io.c fmovie.c fmovie_io.c ps_io.c epsf_io.c ccmovie.c \
      cfmovie.c fcurve.c fcurve_io.c fpolygon.c fpolygon_io.c tiff_io.c \
      pgm_io.c ppm_io.c dcurve.c dcurve_io.c rawdata.c rawdata_io.c jpeg_io.c bmp_io.c 

OBJ = 	$(OBJPATH)/mw.o \
	$(OBJPATH)/mwio.o \
	$(OBJPATH)/cimage.o \
	$(OBJPATH)/fimage.o \
	$(OBJPATH)/cimage_io.o \
	$(OBJPATH)/fimage_io.o \
	$(OBJPATH)/window.o \
	$(OBJPATH)/wpanel.o \
	$(OBJPATH)/cmovie.o \
	$(OBJPATH)/cmovie_io.o \
	$(OBJPATH)/fmovie.o \
	$(OBJPATH)/fmovie_io.o \
	$(OBJPATH)/ccmovie.o \
	$(OBJPATH)/ccmovie_io.o \
	$(OBJPATH)/cfmovie.o \
	$(OBJPATH)/cfmovie_io.o \
	$(OBJPATH)/polygon.o \
	$(OBJPATH)/fpolygon.o \
	$(OBJPATH)/polygon_io.o \
	$(OBJPATH)/fpolygon_io.o \
	$(OBJPATH)/fsignal.o \
	$(OBJPATH)/fsignal_io.o \
	$(OBJPATH)/wave_io.o \
	$(OBJPATH)/ascii_file.o \
	$(OBJPATH)/file_type.o \
	$(OBJPATH)/wtrans2d.o \
	$(OBJPATH)/wtrans2d_io.o \
	$(OBJPATH)/basic_conv.o \
	$(OBJPATH)/type_conv.o \
	$(OBJPATH)/wmax2d.o \
	$(OBJPATH)/wmax2d_io.o \
	$(OBJPATH)/wtrans1d.o \
	$(OBJPATH)/wtrans1d_io.o \
	$(OBJPATH)/pm_io.o \
	$(OBJPATH)/tiff_io.o \
	$(OBJPATH)/jpeg_io.o \
	$(OBJPATH)/pgm_io.o \
	$(OBJPATH)/ppm_io.o \
	$(OBJPATH)/gif_io.o \
	$(OBJPATH)/bmp_io.o \
	$(OBJPATH)/ps_io.o \
	$(OBJPATH)/epsf_io.o \
	$(OBJPATH)/curve.o \
	$(OBJPATH)/fcurve.o \
	$(OBJPATH)/dcurve.o \
	$(OBJPATH)/curve_io.o \
	$(OBJPATH)/fcurve_io.o \
	$(OBJPATH)/dcurve_io.o \
	$(OBJPATH)/mimage.o \
	$(OBJPATH)/mimage_io.o \
	$(OBJPATH)/cmimage.o \
	$(OBJPATH)/cmimage_io.o \
	$(OBJPATH)/cfimage.o \
	$(OBJPATH)/cfimage_io.o \
	$(OBJPATH)/ccimage.o \
	$(OBJPATH)/ccimage_io.o \
	$(OBJPATH)/shape.o \
	$(OBJPATH)/shape_io.o \
	$(OBJPATH)/rawdata.o \
	$(OBJPATH)/rawdata_io.o \
	$(OBJPATH)/list.o \
	$(OBJPATH)/list_io.o \
	$(OBJPATH)/unix_bsd.o \
	$(OBJPATH)/module.o \
	$(OBJPATH)/module_io.o

sysmw :	$(OBJPATH)/make_type_conv $(OBJ) $(OBJPATH)/mw_main.o $(OBJPATH)/make_type_conv
	@echo " "
	@echo "Create Archive (static) library..."	
	mv $(OBJPATH)/mw_main.o $(LIBTARGET)/.
	$(AR) $(LIBaSYSMW) $(OBJ)
	-ranlib $(LIBaSYSMW)

sysmwsl : $(OBJPATH)/make_type_conv $(OBJ) $(OBJPATH)/mw_main.o 
	@echo " "
	@echo "Create Shared (dynamic) library..."	
	mv $(OBJPATH)/mw_main.o $(LIBTARGET)/.
# Version HP-UX
# Version SunOS
# Version Sun Solaris
# Version Silicon Graphics IRIX et Linux PC
	ld -shared -soname libsysmw.so -o $(LIBslSYSMW) $(OBJ)

notiff : 	$(OBJPATH)/notiff.o
	@echo " "
	@echo "Create Archive (static) NOTIFF library..."		
	$(AR) $(LIBaNOTIFF) $(OBJPATH)/notiff.o
	-ranlib $(LIBaNOTIFF)

notiffsl : 	$(OBJPATH)/notiff.o
	@echo " "
	@echo "Create Shared (dynamic) NOTIFF library..."		
# Version HP-UX
# Version SunOS
# Version Sun Solaris
# Version Silicon Graphics IRIX et Linux PC
	ld -shared -soname libnotiff.so -o $(LIBslNOTIFF) $(OBJPATH)/notiff.o

xmwcom : 	$(OBJPATH)/xmw_com.o
	@echo " "
	@echo "Create Archive (static) XMWCOM library..."		
	$(AR) $(LIBaXMWCOM) $(OBJPATH)/xmw_com.o
	-ranlib $(LIBaXMWCOM)

xmwcomsl : 	$(OBJPATH)/xmw_com.o
	@echo " "
	@echo "Create Shared (dynamic) XMWCOM library..."		
# Version HP-UX
# Version SunOS
# Version Sun Solaris
# Version Silicon Graphics IRIX et Linux PC
	ld -shared -soname libxmwcom.so -o $(LIBslXMWCOM) $(OBJPATH)/xmw_com.o

shared :	sysmwsl notiffsl xmwcomsl 

static : 	sysmw notiff xmwcom 

# --- DEPENDANCY ---

# notiff library ----------------------------------------

$(OBJPATH)/notiff.o : $(MWINCDIR)/tiff.h $(MWINCDIR)/tiffio.h notiff.c
	$(CC) $(CFLAGSLIB) -o $@ notiff.c

# xmwcom library ----------------------------------------

$(OBJPATH)/xmw_com.o : $(MWINCDIR)/mw.h $(MWINCDIR)/file_type.h $(MWINCDIR)/xmw_com.h \
	mwio.c basic_conv.c file_type.c xmw_com.c
	$(CC) $(CFLAGSLIB) -o $@ xmw_com.c

# sysmw library ----------------------------------------

$(OBJPATH)/mw.o : $(MWINCDIR)/mw.h mw.c
	$(CC) $(CFLAGSLIB) -o $@ mw.c

$(OBJPATH)/mw_main.o : $(MWINCDIR)/mw.h mw_main.c
	$(CC) $(CFLAGSLIB) -o $@ mw_main.c

$(OBJPATH)/mwio.o : $(MWINCDIR)/mw.h $(MWINCDIR)/file_type.h $(MWINCDIR)/cimage.h mwio.c cimage_io.c cmovie_io.c fmovie_io.c fimage_io.c polygon_io.c fsignal_io.c wtrans2d_io.c wtrans1d_io.c wmax2d_io.c
	$(CC) $(CFLAGSLIB) -o $@ mwio.c

$(OBJPATH)/window.o : $(MWINCDIR)/window.h $(MWINCDIR)/Wdevice.h $(MWINCDIR)/cimage.h window.c
	$(CC) $(CFLAGSLIB) -o $@ window.c

$(OBJPATH)/wpanel.o : $(MWINCDIR)/wpanel.h $(MWINCDIR)/Wdevice.h window.c wpanel.c
	$(CC) $(CFLAGSLIB) -o $@ wpanel.c

$(OBJPATH)/ascii_file.o : $(MWINCDIR)/ascii_file.h ascii_file.c
	$(CC) $(CFLAGSLIB) -o $@ ascii_file.c

$(OBJPATH)/file_type.o : $(MWINCDIR)/file_type.h $(MWINCDIR)/native_ftype.h $(MWINCDIR)/type_conv.h file_type.c
	$(CC) $(CFLAGSLIB) -o $@ file_type.c

$(OBJPATH)/unix_bsd.o : $(MWINCDIR)/mw.h unix_bsd.c
	$(CC) $(CFLAGSLIB) -o $@ unix_bsd.c

# --- INTERNAL TYPES ---

$(OBJPATH)/make_type_conv : $(MWINCDIR)/native_ftype.h make_type_conv.c basic_conv.c 
	$(CC) $(CFLAGS) -o $@ make_type_conv.c
	$(OBJPATH)/make_type_conv 			

$(OBJPATH)/cimage.o : $(MWINCDIR)/cimage.h cimage.c
	$(CC) $(CFLAGSLIB) -o $@ cimage.c

$(OBJPATH)/cimage_io.o : $(MWINCDIR)/cimage.h $(MWINCDIR)/mw.h cimage_io.c
	$(CC) $(CFLAGSLIB) -o $@ cimage_io.c

$(OBJPATH)/pm_io.o : $(MWINCDIR)/pm.h $(MWINCDIR)/mw.h pm_io.c
	$(CC) $(CFLAGSLIB) -o $@ pm_io.c

$(OBJPATH)/tiff_io.o : $(MWINCDIR)/tiff.h $(MWINCDIR)/tiffio.h $(MWINCDIR)/mw.h tiff_io.c
	$(CC) $(CFLAGSLIB) -o $@ tiff_io.c

$(OBJPATH)/jpeg_io.o : $(MWINCDIR)/mw.h jpeg_io.c
	$(CC) $(CFLAGSLIB) -o $@ jpeg_io.c

$(OBJPATH)/gif_io.o :  $(MWINCDIR)/mw.h gif_io.c
	$(CC) $(CFLAGSLIB) -o $@ gif_io.c

$(OBJPATH)/bmp_io.o :  $(MWINCDIR)/mw.h bmp_io.c
	$(CC) $(CFLAGSLIB) -o $@ bmp_io.c

$(OBJPATH)/ps_io.o :  $(MWINCDIR)/mw.h ps_io.c
	$(CC) $(CFLAGSLIB) -o $@ ps_io.c

$(OBJPATH)/epsf_io.o :  $(MWINCDIR)/mw.h epsf_io.c
	$(CC) $(CFLAGSLIB) -o $@ epsf_io.c

$(OBJPATH)/pgm_io.o :  $(MWINCDIR)/mw.h pgm_io.c
	$(CC) $(CFLAGSLIB) -o $@ pgm_io.c

$(OBJPATH)/ppm_io.o :  $(MWINCDIR)/mw.h ppm_io.c basic_conv.c
	$(CC) $(CFLAGSLIB) -o $@ ppm_io.c

$(OBJPATH)/cmovie.o : $(MWINCDIR)/cmovie.h $(MWINCDIR)/cimage.h cmovie.c
	$(CC) $(CFLAGSLIB) -o $@ cmovie.c

$(OBJPATH)/cmovie_io.o : $(MWINCDIR)/cmovie.h $(MWINCDIR)/mw.h $(MWINCDIR)/cimage.h $(MWINCDIR)/ascii_file.h cmovie_io.c
	$(CC) $(CFLAGSLIB) -o $@ cmovie_io.c

$(OBJPATH)/fmovie.o : $(MWINCDIR)/fmovie.h $(MWINCDIR)/fimage.h fmovie.c
	$(CC) $(CFLAGSLIB) -o $@ fmovie.c

$(OBJPATH)/fmovie_io.o : $(MWINCDIR)/fmovie.h $(MWINCDIR)/fimage.h $(MWINCDIR)/mw.h $(MWINCDIR)/ascii_file.h fmovie_io.c
	$(CC) $(CFLAGSLIB) -o $@ fmovie_io.c

$(OBJPATH)/ccmovie.o : $(MWINCDIR)/ccmovie.h $(MWINCDIR)/ccimage.h ccmovie.c
	$(CC) $(CFLAGSLIB) -o $@ ccmovie.c

$(OBJPATH)/ccmovie_io.o : $(MWINCDIR)/ccmovie.h $(MWINCDIR)/mw.h $(MWINCDIR)/ccimage.h $(MWINCDIR)/ascii_file.h ccmovie_io.c
	$(CC) $(CFLAGSLIB) -o $@ ccmovie_io.c


$(OBJPATH)/cfmovie.o : $(MWINCDIR)/cfmovie.h $(MWINCDIR)/cfimage.h cfmovie.c
	$(CC) $(CFLAGSLIB) -o $@ cfmovie.c

$(OBJPATH)/cfmovie_io.o : $(MWINCDIR)/cfmovie.h $(MWINCDIR)/mw.h $(MWINCDIR)/cfimage.h $(MWINCDIR)/ascii_file.h cfmovie_io.c
	$(CC) $(CFLAGSLIB) -o $@ cfmovie_io.c


$(OBJPATH)/fimage.o : $(MWINCDIR)/fimage.h fimage.c
	$(CC) $(CFLAGSLIB) -o $@ fimage.c

$(OBJPATH)/fimage_io.o : $(MWINCDIR)/fimage.h $(MWINCDIR)/mw.h fimage_io.c
	$(CC) $(CFLAGSLIB) -o $@ fimage_io.c

$(OBJPATH)/curve.o : $(MWINCDIR)/curve.h curve.c
	$(CC) $(CFLAGSLIB) -o $@ curve.c

$(OBJPATH)/fcurve.o : $(MWINCDIR)/fcurve.h fcurve.c
	$(CC) $(CFLAGSLIB) -o $@ fcurve.c

$(OBJPATH)/dcurve.o : $(MWINCDIR)/dcurve.h dcurve.c
	$(CC) $(CFLAGSLIB) -o $@ dcurve.c

$(OBJPATH)/curve_io.o : $(MWINCDIR)/curve.h curve_io.c
	$(CC) $(CFLAGSLIB) -o $@ curve_io.c

$(OBJPATH)/fcurve_io.o : $(MWINCDIR)/fcurve.h fcurve_io.c
	$(CC) $(CFLAGSLIB) -o $@ fcurve_io.c

$(OBJPATH)/dcurve_io.o : $(MWINCDIR)/dcurve.h dcurve_io.c
	$(CC) $(CFLAGSLIB) -o $@ dcurve_io.c

$(OBJPATH)/polygon.o : $(MWINCDIR)/polygon.h $(MWINCDIR)/curve.h polygon.c curve.c
	$(CC) $(CFLAGSLIB) -o $@ polygon.c

$(OBJPATH)/fpolygon.o : $(MWINCDIR)/fpolygon.h $(MWINCDIR)/fcurve.h fpolygon.c fcurve.c
	$(CC) $(CFLAGSLIB) -o $@ fpolygon.c

$(OBJPATH)/polygon_io.o : $(MWINCDIR)/polygon.h ascii_file.c polygon_io.c 
	$(CC) $(CFLAGSLIB) -o $@  polygon_io.c

$(OBJPATH)/fpolygon_io.o : $(MWINCDIR)/fpolygon.h ascii_file.c fpolygon_io.c 
	$(CC) $(CFLAGSLIB) -o $@  fpolygon_io.c

$(OBJPATH)/mimage.o : $(MWINCDIR)/mimage.h $(MWINCDIR)/curve.h mimage.c
	$(CC) $(CFLAGSLIB) -o $@ mimage.c

$(OBJPATH)/mimage_io.o : $(MWINCDIR)/mimage.h $(MWINCDIR)/curve.h mimage.c mimage_io.c
	$(CC) $(CFLAGSLIB) -o $@ mimage_io.c

$(OBJPATH)/cmimage.o : $(MWINCDIR)/cmimage.h $(MWINCDIR)/mimage.h cmimage.c
	$(CC) $(CFLAGSLIB) -o $@ cmimage.c

$(OBJPATH)/cmimage_io.o : $(MWINCDIR)/cmimage.h $(MWINCDIR)/mimage.h cmimage.c cmimage_io.c
	$(CC) $(CFLAGSLIB) -o $@ cmimage_io.c

$(OBJPATH)/shape.o : $(MWINCDIR)/shape.h $(MWINCDIR)/curve.h shape.c
	$(CC) $(CFLAGSLIB) -o $@ shape.c

$(OBJPATH)/shape_io.o : $(MWINCDIR)/shape.h $(MWINCDIR)/curve.h shape.c shape_io.c
	$(CC) $(CFLAGSLIB) -o $@ shape_io.c

$(OBJPATH)/list.o : $(MWINCDIR)/list.h list.c
	$(CC) $(CFLAGSLIB) -o $@ list.c

$(OBJPATH)/list_io.o : $(MWINCDIR)/list.h list.c list_io.c
	$(CC) $(CFLAGSLIB) -o $@ list_io.c

$(OBJPATH)/fsignal.o : $(MWINCDIR)/fsignal.h fsignal.c
	$(CC) $(CFLAGSLIB) -o $@  fsignal.c

$(OBJPATH)/fsignal_io.o : $(MWINCDIR)/fsignal.h ascii_file.c fsignal_io.c
	$(CC) $(CFLAGSLIB) -o $@ fsignal_io.c

$(OBJPATH)/wave_io.o : $(MWINCDIR)/fsignal.h wave_io.c
	$(CC) $(CFLAGSLIB) -o $@ wave_io.c

$(OBJPATH)/ifp_io.o : $(MWINCDIR)/fsignal.h ifp_io.c
	$(CC) $(CFLAGSLIB) -o $@ ifp_io.c

$(OBJPATH)/wtrans2d.o : $(MWINCDIR)/wtrans2d.h wtrans2d.c
	$(CC) $(CFLAGSLIB) -o $@  wtrans2d.c

$(OBJPATH)/wtrans2d_io.o : $(MWINCDIR)/wtrans2d.h ascii_file.c wtrans2d_io.c
	$(CC) $(CFLAGSLIB) -o $@ wtrans2d_io.c

$(OBJPATH)/wmax2d.o : $(MWINCDIR)/wtrans2d.h $(MWINCDIR)/wmax2d.h wmax2d.c
	$(CC) $(CFLAGSLIB) -o $@ wmax2d.c

$(OBJPATH)/wmax2d_io.o : $(MWINCDIR)/wtrans2d.h $(MWINCDIR)/wmax2d.h wmax2d_io.c
	$(CC) $(CFLAGSLIB) -o $@  wmax2d_io.c

$(OBJPATH)/wtrans1d.o : $(MWINCDIR)/wtrans1d.h wtrans1d.c
	$(CC) $(CFLAGSLIB) -o $@  wtrans1d.c

$(OBJPATH)/wtrans1d_io.o : $(MWINCDIR)/wtrans1d.h ascii_file.c wtrans1d_io.c
	$(CC) $(CFLAGSLIB) -o $@  wtrans1d_io.c

$(OBJPATH)/basic_conv.o : basic_conv.c
	$(CC) $(CFLAGSLIB) -o $@ basic_conv.c

$(OBJPATH)/type_conv.o : type_conv.c $(OBJPATH)/make_type_conv
	$(CC) $(CFLAGSLIB) -o $@ type_conv.c

$(OBJPATH)/cfimage.o : $(MWINCDIR)/cfimage.h cfimage.c
	$(CC) $(CFLAGSLIB) -o $@ cfimage.c

$(OBJPATH)/cfimage_io.o : $(MWINCDIR)/cfimage.h cfimage_io.c
	$(CC) $(CFLAGSLIB) -o $@  cfimage_io.c

$(OBJPATH)/ccimage.o : $(MWINCDIR)/ccimage.h ccimage.c
	$(CC) $(CFLAGSLIB) -o $@  ccimage.c

$(OBJPATH)/ccimage_io.o : $(MWINCDIR)/ccimage.h ccimage_io.c
	$(CC) $(CFLAGSLIB) -o $@ ccimage_io.c

$(OBJPATH)/rawdata.o : $(MWINCDIR)/rawdata.h rawdata.c
	$(CC) $(CFLAGSLIB) -o $@  rawdata.c

$(OBJPATH)/rawdata_io.o : $(MWINCDIR)/rawdata.h rawdata_io.c
	$(CC) $(CFLAGSLIB) -o $@  rawdata_io.c

$(OBJPATH)/module.o : $(MWINCDIR)/module.h module.c
	$(CC) $(CFLAGSLIB) -o $@ module.c

$(OBJPATH)/module_io.o : $(MWINCDIR)/module.h module_io.c
	$(CC) $(CFLAGSLIB) -o $@ module_io.c


# --------------------

install :
	@echo "===== Install the library on $(DESTDIR)"
	mkdir -p $(DESTDIR)/lib/$(MW_MACHINETYPE)
	cp $(LIBTARGET)/mw_main.o $(DESTDIR)/lib/$(MW_MACHINETYPE)
	cp $(LIBslSYSMW) $(DESTDIR)/lib/$(MW_MACHINETYPE)
	cp $(LIBslNOTIFF) $(DESTDIR)/lib/$(MW_MACHINETYPE)
	cp $(LIBslXMWCOM) $(DESTDIR)/lib/$(MW_MACHINETYPE)

sysinstall :
	@echo "===== Install the library on $(SYSDESTDIR)"
	mkdir -m 755 -p $(SYSDESTDIR)/lib/$(MW_MACHINETYPE)
	cp $(LIBTARGET)/mw_main.o $(SYSDESTDIR)/lib/$(MW_MACHINETYPE)
	chmod 755 $(SYSDESTDIR) $(SYSDESTDIR)/lib $(SYSDESTDIR)/lib/$(MW_MACHINETYPE)
	cp $(LIBslSYSMW) $(SYSDESTDIR)/lib/$(MW_MACHINETYPE)
	cp $(LIBslNOTIFF) $(SYSDESTDIR)/lib/$(MW_MACHINETYPE)
	cp $(LIBslXMWCOM) $(SYSDESTDIR)/lib/$(MW_MACHINETYPE)
	chmod 644 $(SYSDESTDIR)/lib/$(MW_MACHINETYPE)/*
# On HP-UX
# On SunOS 4.x, Solaris, IRIX and PC Linux
	chmod 755 $(SYSDESTDIR)/lib/$(MW_MACHINETYPE)/*.so

clean :
	/bin/rm -f type_conv.c $(OBJ) $(OBJPATH)/make_type_conv $(OBJPATH)/notiff.o $(OBJPATH)/xmw_com.o $(LIBaSYSMW) $(LIBaNOTIFF) $(LIBaXMWCOM) $(LIBslSYSMW) $(LIBslNOTIFF) $(LIBslXMWCOM)




