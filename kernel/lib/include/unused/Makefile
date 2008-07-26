# Megawave library include files
# Put here the include files needed to compile MegaWave2 modules.
# Do not put those used to compile the library only (as tiff.h,...).
#-----------------------------------------------------------------------------
# This file is part of the MegaWave2 system library 
# MegaWave2 is a "soft-publication" for the scientific community. It has
# been developed for research purposes and it comes without any warranty.
# The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
# CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
#       94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
#-----------------------------------------------------------------------------


DESTDIR  = $(PRIVATE_MEGAWAVE2)/sys/include
DESTLINUXDIR = $(PRIVATE_MEGAWAVE2)/sys/include/Linux
DESTLINUXX11DIR = $(PRIVATE_MEGAWAVE2)/sys/include/Linux/X11
DESTSOLARISDIR = $(PRIVATE_MEGAWAVE2)/sys/include/Solaris
SYSDESTDIR  = $(PUBLIC_MEGAWAVE2)/sys/include
SYSDESTLINUXDIR  = $(PUBLIC_MEGAWAVE2)/sys/include/Linux
SYSDESTLINUXX11DIR  = $(PUBLIC_MEGAWAVE2)/sys/include/Linux/X11
SYSDESTSOLARISDIR  = $(PUBLIC_MEGAWAVE2)/sys/include/Solaris

SRC =   mw.h cimage.h fimage.h fsignal.h \
	window.h Wdevice.h cmovie.h fmovie.h mwi.h file_type.h \
	polygon.h ascii_file.h wtrans2d.h string_size.h wmax2d.h wtrans1d.h \
        curve.h cfimage.h ccimage.h ccmovie.h cfmovie.h unix_bsd.h module.h \
	fcurve.h dcurve.h fpolygon.h mimage.h cmimage.h shape.h \
	basic_conv.h rawdata.h type_conv.h native_ftype.h \
	wpanel.h list.h wpack2d.h

LINUXSRC = Linux/README Linux/libio.h Linux/libio_2.1.h Linux/libio_2.2.h \
	Linux/libio_2.3.3.h Linux/stdio.h Linux/stdio_2.1.h Linux/stdio_2.2.h \
	Linux/stdio_2.3.h Linux/stdio_2.3.3.h  \
	Linux/stdarg.h Linux/stdarg_2.1.h \
	Linux/stdarg_2.2.h Linux/stdarg_2.3.h

LINUXX11SRC = Linux/X11/Xlib.h Linux/X11/Xlib_2.1.h Linux/X11/Xlib_v1.5.h 

SOLARISSRC = Solaris/README Solaris/stdio_impl.h Solaris/stdio_tag.h

install :
	mkdir -p $(DESTDIR)
	cp $(SRC) $(DESTDIR)
	mkdir -p $(DESTLINUXDIR)
	cp $(LINUXSRC) $(DESTLINUXDIR)
	mkdir -p $(DESTLINUXX11DIR)
	cp $(LINUXX11SRC) $(DESTLINUXX11DIR)
	mkdir -p $(DESTSOLARISDIR)
	cp $(SOLARISSRC) $(DESTSOLARISDIR)

sysinstall:
	mkdir -m 755 -p $(SYSDESTDIR)
	cp $(SRC) $(SYSDESTDIR)
	chmod 644 $(SYSDESTDIR)/*
	mkdir -p $(SYSDESTLINUXDIR)
	chmod 755 $(SYSDESTLINUXDIR)
	cp $(LINUXSRC) $(SYSDESTLINUXDIR)
	chmod 644 $(SYSDESTLINUXDIR)/*
	mkdir -p $(SYSDESTLINUXX11DIR)
	chmod 755 $(SYSDESTLINUXX11DIR)
	cp $(LINUXX11SRC) $(SYSDESTLINUXX11DIR)
	chmod 644 $(SYSDESTLINUXX11DIR)/*
	mkdir -p $(SYSDESTSOLARISDIR)
	chmod 755 $(SYSDESTSOLARISDIR) 
	cp $(SOLARISSRC) $(SYSDESTSOLARISDIR)
	chmod 644 $(SYSDESTSOLARISDIR)/*

