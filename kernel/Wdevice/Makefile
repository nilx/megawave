# Warning : this file has been automatically generated from ./Makefile.in using macro mwconfig V.1.07 with arch = 'Linux ix86' option = 'shared '
#-----------------------------------------------------------------------------
# Makefile.in used to create Makefile using macro mw2config
# Wdevice library : Copyright (c) Jacques Froment 1991-2003
#-----------------------------------------------------------------------------
# This file is part of the Wdevice library.
# Wdevice is a "soft-publication" for the scientific community. It has
# been developed for research purposes and it comes without any warranty.
# The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
# CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
#       94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
#-----------------------------------------------------------------------------

#--- Optimized or debug mode 
CCMODE	= -O

#--- The C compiler to use, version "static library"


# Compiler Version HP-UX 10.x 

# Compiler version Sun OS 4.x, HP-UX 9.x, Linux and SGI IRIX "static library" 

# Compiler version Sun Solaris

#--- The C compiler to use, version "shared library"


# Version HP-UX 10.x "shared library" 

# Version Silicon Graphics IRIX "shared library"

# Version GCC "shared library"	 (ex. : PC Linux)
CC = gcc $(CCMODE) -fpic

# Compiler version Sun Solaris


# --- various Path ---

# Path for source files
SRCPATH = src

# Path for include files
INCLUDEPATH = include

# Path for object files
OBJPATH = $(PRIVATE_MEGAWAVE2)/kernel_obj/Wdevice/obj/$(MW_MACHINETYPE)

# Path for the librairie
LIBPATH = $(PRIVATE_MEGAWAVE2)/kernel_obj/Wdevice/lib/$(MW_MACHINETYPE)
LIBDESTDIR  = $(PRIVATE_MEGAWAVE2)/sys/lib/$(MW_MACHINETYPE)
SYSLIBDESTDIR  = $(PUBLIC_MEGAWAVE2)/sys/lib/$(MW_MACHINETYPE)

# Where to copy Wdevice.h to be used by MegaWave2 system library
INCLUDESYSLIB = $(PRIVATE_MEGAWAVE2)/lib/include

# --- Libraries ---

LIB_WDEVICE_X11 = -lX11

# --- Flags for cc ---

# -c to avoid linking 
# -DW_DEBUG_ON to enable built-in debugging

CFLAGS = -I$(INCLUDEPATH) -DX11R4 -c 

# --- Wdevice ---

WDEVICE_X11 = $(OBJPATH)/W_X11R4.o

# ---- OBJ CODES GENERATION ----

$(OBJPATH)/W_X11R4.o : $(SRCPATH)/W_X11R4.c
	$(CC) $(CFLAGS) -o $@ $(SRCPATH)/W_X11R4.c

# --- Dependencies ---

$(OBJPATH)/W_X11R4.o : $(INCLUDEPATH)/W_X11R4.h $(SRCPATH)/W_X11R4_var.c \
	$(SRCPATH)/W_X11R4_misc.c $(SRCPATH)/W_X11R4.Icon 

# --- programs to compile ---

static:	$(WDEVICE_X11)
	@echo "Create Archive (static) W_X11R4 library..."	
	ar ru $(LIBPATH)/libW_X11R4.a $(WDEVICE_X11)
	-ranlib $(LIBPATH)/libW_X11R4.a

shared:	$(WDEVICE_X11)
	@echo "Create Shared (dynamic) W_X11R4 library..."		

# Version HP-UX
# Version SunOS
# Version Sun Solaris
# Version Silicon Graphics IRIX et Linux PC
	ld -shared -o $(LIBPATH)/libW_X11R4.so $(WDEVICE_X11)

install:
	@echo "===== Install the library on $(LIBDESTDIR)"      
	cp $(INCLUDEPATH)/Wdevice.h ../lib/include/.
	mkdir -p $(INCLUDESYSLIB)
	cp $(INCLUDEPATH)/Wdevice.h $(INCLUDESYSLIB)/.
	mkdir -p $(LIBDESTDIR)
# On SunOS 4.x, Solaris, IRIX and PC Linux
	cp $(LIBPATH)/libW_X11R4.so $(LIBDESTDIR)/
	mkdir -p $(PRIVATE_MEGAWAVE2)/sys/include
	cp $(INCLUDEPATH)/Wdevice.h $(PRIVATE_MEGAWAVE2)/sys/include

sysinstall:
	@echo "===== Install the library on $(SYSLIBDESTDIR)"
	mkdir -m 755 -p $(SYSLIBDESTDIR)
# On SunOS 4.x, Solaris, IRIX and PC Linux
	cp $(LIBPATH)/libW_X11R4.so $(SYSLIBDESTDIR)
	chmod 755 $(SYSLIBDESTDIR)/*.so
	mkdir -m 755 -p $(PUBLIC_MEGAWAVE2)/sys/include
	cp $(INCLUDEPATH)/Wdevice.h $(PUBLIC_MEGAWAVE2)/sys/include
	chmod 644 $(PUBLIC_MEGAWAVE2)/sys/include/Wdevice.h

clean :	
	rm -f $(OBJPATH)/* $(LIBPATH)/*


