# Warning : this file has been automatically generated from ./Makefile.in using macro mwconfig V.1.08 with arch = 'Linux ix86' option = ''
#-----------------------------------------------------------------------------
# Makefile.in used to create Makefile using macro mw2config
# MegaWave2 preprocessor : Copyright (c) J.Froment and S.Parrino 1995-2001
#-----------------------------------------------------------------------------
# MAKEFILE for the MegaWave2 preprocesseur on Unix Systems 
# WARNING : the files follow K&R and not ANSI C. (should be changed).
#-----------------------------------------------------------------------------
# This file is part of the MegaWave2 preprocessor. 
# MegaWave2 is a "soft-publication" for the scientific community. It has
# been developed for research purposes and it comes without any warranty.
# The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
# CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
#       94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
#-----------------------------------------------------------------------------

#--- Optimized or debug mode 
CCMODE	= -O
# It has been reported that -O optimization generates bugged binaries on SunOS

BINDIR        = $(PRIVATE_MEGAWAVE2)/sys/bin/$(MW_MACHINETYPE)
SYSBINDIR     = $(PUBLIC_MEGAWAVE2)/sys/bin/$(MW_MACHINETYPE)
TARGETDIR      = $(PRIVATE_MEGAWAVE2)/kernel_obj/mwp
TARGET         = $(TARGETDIR)/$(MW_MACHINETYPE)/mwp
MODE           = 022
RELEASE        = 3
MEGAWAVEIOFILE = megawave2.io
OBJPATH	       = $(TARGETDIR)/obj/$(MW_MACHINETYPE)

# ----- Location where to find cpp -----
# Sur IRIX (et sans doute sur Dec Alpha), le /lib/cpp ou /usr/lib/acpp
# ne contient pas les bonnes definitions de base (#define) ce qui fait planter
# yyparse() dans des fichiers tels que /sys/types.h. 
# Solution trouvee: utiliser le preprocesseur du cc (code inscrit directement
# dans le fichier main.c). Dans ce cas, ne pas definir CPPCMD mais NOCPP.
CPP = CPPCMD=\"/lib/cpp\"

#---

# Variables globales
# Options du cc pour l'edition des liens
LOPTS = $(CCMODE)


LXOPTS = 
YOPTS = -dvt
#YOPTS = -dv

# Mettre -DXMWP dans la liste qui suit si on veut que mwp puisse generer du
#               XMegaWave2
COPTS_DEF = -DDEBUG -DLEXDEBUG -DYYDEBUG -DACCEPT_ANSI -DXMWP

# Default
COPTS = $(COPTS_DEF) -D$(MW_SYSTEMTYPE) $(CCMODE)

# Sur SunOS 5.5 (Solaris 5) : Option -Xs pour K&R
# Sur SunOS 5.6,5.7 (Solaris 6 et 7) : Option -Xa pour ANSI + K&R



# Chaine standard (compilateur natif sauf sous Linux)
YACC = yacc $(YOPTS)
LEX  = lex $(LXOPTS)
CC   = cc $(COPTS)
LD   = cc $(LOPTS)

# Chaine GNU (PC Linux)
YACC = bison $(YOPTS)
LEX  = flex $(LXOPTS)
CC   = gcc $(COPTS)
LD   = gcc $(LOPTS)
COPTS = $(COPTS_DEF) -D$(MW_SYSTEMTYPE) -DFLEX $(CCMODE)


SRC = main.c symbol.c mwp.y  mwp.l io.c bintree.c fillmwarg.c genmain.c \
      genxmain.c omwarg.c odbgmwarg.c genifile.c gentex.c\
      mw_present.c printnod.c data_io.c bintree.h genmain.h io.h symbol.h\
      token.h data_io.h $(MEGAWAVEIOFILE)


OBJ = $(OBJPATH)/main.o $(OBJPATH)/symbol.o $(OBJPATH)/y.tab.o \
	$(OBJPATH)/lex.yy.o $(OBJPATH)/io.o \
	$(OBJPATH)/bintree.o $(OBJPATH)/fillmwarg.o\
	$(OBJPATH)/omwarg.o $(OBJPATH)/odbgmwarg.o\
	$(OBJPATH)/genmain.o $(OBJPATH)/genxmain.o $(OBJPATH)/genifile.o\
	$(OBJPATH)/gentex.o $(OBJPATH)/mw_present.o \
	$(OBJPATH)/printnod.o $(OBJPATH)/data_io.o

all : $(TARGET)

$(TARGET) : $(OBJ)
	$(LD) -o $(TARGET) $(OBJ) -lfl

$(OBJPATH)/main.o : io.h bintree.h symbol.h mwarg.h genmain.h genxmain.h\
                    genifile.h data_io.h main.c
	$(CC) -D$(CPP) -c main.c -o $@

$(OBJPATH)/symbol.o : symbol.h io.h symbol.c
	$(CC) -c symbol.c -o $@

$(OBJPATH)/y.tab.o : bintree.h symbol.h io.h token.h mwp.y
	$(YACC) mwp.y
	/bin/mv mwp.tab.c y.tab.c
	/bin/mv mwp.tab.h y.tab.h
	$(CC) -c y.tab.c -o $@

$(OBJPATH)/lex.yy.o : bintree.h symbol.h token.h io.h mwp.l mwp.y
	$(LEX) mwp.l
	$(CC) -c lex.yy.c -o $@

$(OBJPATH)/io.o : bintree.h io.h io.c
	$(CC) -c io.c -o $@

$(OBJPATH)/bintree.o : mwp.y bintree.h io.h bintree.c
	$(CC) -c bintree.c -o $@

$(OBJPATH)/fillmwarg.o : mwp.y genmain.h bintree.h symbol.h io.h fillmwarg.c
	$(CC) -c fillmwarg.c -o $@

$(OBJPATH)/omwarg.o : bintree.h symbol.h token.h mwarg.h data_io.h io.h mwp.y\
                      omwarg.h omwarg.c
	$(CC) -c omwarg.c -o $@

$(OBJPATH)/odbgmwarg.o : bintree.h symbol.h mwarg.h data_io.h odbgmwarg.h io.h\
                         odbgmwarg.c
	$(CC) -c odbgmwarg.c -o $@

$(OBJPATH)/genmain.o : bintree.h symbol.h token.h mwarg.h data_io.h omwarg.h\
                       genmain.h mwp.y genmain.c
	$(CC) -c  genmain.c -o $@

$(OBJPATH)/genxmain.o : mwarg.h omwarg.h genmain.h genxmain.h genxmain.c
	$(CC) -c genxmain.c -o $@

$(OBJPATH)/genifile.o : bintree.h symbol.h token.h mwarg.h genifile.h genifile.c
	$(CC) -c genifile.c -o $@

$(OBJPATH)/gentex.o : bintree.h token.h io.h symbol.h mwarg.h mwp.y gentex.c
	$(CC) -c gentex.c -o $@

$(OBJPATH)/printnod.o : mwp.y symbol.h bintree.h io.h printnod.c
	$(CC) -c printnod.c -o $@

$(OBJPATH)/mw_present.o : mw_present.c
	$(CC) -c mw_present.c -o $@

$(OBJPATH)/data_io.o : symbol.h data_io.h data_io.c
	$(CC) -c data_io.c -o $@

install :
	mkdir   -p $(BINDIR)
	cp $(TARGET) $(BINDIR)/`basename $(TARGET)`
	mkdir   -p $(PRIVATE_MEGAWAVE2)
	cp $(MEGAWAVEIOFILE) $(PRIVATE_MEGAWAVE2)/$(MEGAWAVEIOFILE)
	-rm -f $(BINDIR)/u`basename $(TARGET)`
	cp -p $(BINDIR)/`basename $(TARGET)` $(BINDIR)/u`basename $(TARGET)`

sysinstall :
	-umask $(MODE);
	mkdir -m 755 -p $(SYSBINDIR)
	chmod 755 $(PUBLIC_MEGAWAVE2)/sys/bin
	cp $(TARGET) $(SYSBINDIR)/`basename $(TARGET)`
	chmod 755 $(SYSBINDIR)/*
	umask $(MODE); cp  $(MEGAWAVEIOFILE) $(PUBLIC_MEGAWAVE2)/$(MEGAWAVEIOFILE)
	chmod 644 $(PUBLIC_MEGAWAVE2)/$(MEGAWAVEIOFILE)
	-rm -f $(SYSBINDIR)/u`basename $(TARGET)`
	cp -p $(SYSBINDIR)/`basename $(TARGET)` $(SYSBINDIR)/u`basename $(TARGET)`

clean :
	/bin/rm -f $(OBJ) $(OBJPATH)/lex.yy_.o lex.yy.c y.tab.c y.output y.tab.h mwp.output

execlean: clean
	/bin/rm -f $(TARGET)


