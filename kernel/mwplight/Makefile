# Warning : this file has been automatically generated from ./Makefile.in using macro mwconfig V.1.08 with arch = 'Linux ix86' option = ''
#-----------------------------------------------------------------------------
# Makefile.in used to create Makefile using macro mw2config
# MegaWave2 'light' preprocessor : Copyright (c) J.Froment 2005
#-----------------------------------------------------------------------------
# MAKEFILE for the MegaWave2 'light' preprocesseur on Unix Systems 
#-----------------------------------------------------------------------------
# This file is part of the MegaWave2 light preprocessor. 
# MegaWave2 is a "soft-publication" for the scientific community. It has
# been developed for research purposes and it comes without any warranty.
# The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
# CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
#       94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
#-----------------------------------------------------------------------------

#--- Optimized or debug mode 
CCMODE	= -O

#--- On Linux systems, assuming cc is gcc and using -aux-info (See CompileProto)
AUXINFO =
AUXINFO = -aux-info

CC = cc  $(CCMODE) 

BINDIR        = $(PRIVATE_MEGAWAVE2)/sys/bin/$(MW_MACHINETYPE)
SYSBINDIR     = $(PUBLIC_MEGAWAVE2)/sys/bin/$(MW_MACHINETYPE)
TARGETDIR      = $(PRIVATE_MEGAWAVE2)/kernel_obj/mwplight
TARGET         = $(TARGETDIR)/$(MW_MACHINETYPE)/mwplight
MODE           = 022
OBJPATH	       = $(TARGETDIR)/obj/$(MW_MACHINETYPE)

OBJ = $(OBJPATH)/mwpl_main.o $(OBJPATH)/mwpl_io.o $(OBJPATH)/mwpl_parse.o \
      $(OBJPATH)/mwpl_header.o $(OBJPATH)/mwpl_tree.o $(OBJPATH)/mwpl_instruction.o \
      $(OBJPATH)/mwpl_mfile.o $(OBJPATH)/mwpl_afile.o $(OBJPATH)/mwpl_tfile.o \
      $(OBJPATH)/mwpl_ifile.o $(OBJPATH)/mwpl_present.o

INFILES	= mwpl_main.h mwpl_tree.h mwpl_proto.h 

all : $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) 

$(OBJPATH)/mwpl_main.o : mwpl_main.c $(INFILES)
	$(CC) -c mwpl_main.c -o $@

$(OBJPATH)/mwpl_io.o : mwpl_io.c  $(INFILES)
	$(CC) -c mwpl_io.c $(AUXINFO) mwpl_io.aux -o $@

$(OBJPATH)/mwpl_parse.o : mwpl_parse.c $(INFILES)
	$(CC) -c mwpl_parse.c $(AUXINFO) mwpl_parse.aux -o $@

$(OBJPATH)/mwpl_header.o : mwpl_header.c  $(INFILES)
	$(CC) -c mwpl_header.c $(AUXINFO) mwpl_header.aux -o $@

$(OBJPATH)/mwpl_tree.o : mwpl_tree.c $(INFILES)
	$(CC) -c mwpl_tree.c  $(AUXINFO) mwpl_tree.aux -o $@

$(OBJPATH)/mwpl_instruction.o : mwpl_instruction.c $(INFILES)
	$(CC) -c mwpl_instruction.c  $(AUXINFO) mwpl_instruction.aux -o $@

$(OBJPATH)/mwpl_mfile.o : mwpl_mfile.c  $(INFILES)
	$(CC) -c mwpl_mfile.c $(AUXINFO) mwpl_mfile.aux -o $@

$(OBJPATH)/mwpl_afile.o : mwpl_afile.c  $(INFILES)
	$(CC) -c mwpl_afile.c $(AUXINFO) mwpl_afile.aux -o $@

$(OBJPATH)/mwpl_tfile.o : mwpl_tfile.c  $(INFILES)
	$(CC) -c mwpl_tfile.c $(AUXINFO) mwpl_tfile.aux -o $@

$(OBJPATH)/mwpl_ifile.o : mwpl_ifile.c  $(INFILES)
	$(CC) -c mwpl_ifile.c $(AUXINFO) mwpl_ifile.aux -o $@

$(OBJPATH)/mwpl_present.o : mwpl_present.c  $(INFILES)
	$(CC) -c mwpl_present.c $(AUXINFO) mwpl_present.aux -o $@

install :
	mkdir   -p $(BINDIR)
	cp $(TARGET) $(BINDIR)/`basename $(TARGET)`

sysinstall :
	-umask $(MODE);
	mkdir -m 755 -p $(SYSBINDIR)
	chmod 755 $(PUBLIC_MEGAWAVE2)/sys/bin
	cp $(TARGET) $(SYSBINDIR)/`basename $(TARGET)`
	chmod 755 $(SYSBINDIR)/*

clean :
	/bin/rm -f $(OBJ)

execlean: clean
	/bin/rm -f $(TARGET)


