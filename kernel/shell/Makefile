# %Z%MegaWave %R%, %M% %I%, %G%
#-------------------------------------------
# Makefile for the MegaWave2 system macros
# (c)1995-2004 Jacques Froment
#-----------------------------------------------------------------------------
# This file is part of the MegaWave2 system library 
# MegaWave2 is a "soft-publication" for the scientific community. It has
# been developed for research purposes and it comes without any warranty.
# The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
# CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
#       94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
#-----------------------------------------------------------------------------

SHELL = /bin/sh

# Megawave shell makefile

DESTDIR = $(PRIVATE_MEGAWAVE2)/sys/shell
DESTDATADIR = $(PRIVATE_MEGAWAVE2)/sys/shell/data
SYSDESTDIR  = $(PUBLIC_MEGAWAVE2)/sys/shell
SYSDESTDATADIR  = $(PUBLIC_MEGAWAVE2)/sys/shell/data

# Files to install for all users

SYSTARGET =  .mw2_help_lg_com cmw2 cmw2_all mwvers mwdoclatex mwnewuser mwecho \
	mwarch cxmw2 mwdep mwmakedoc cmw2macro mwdocxdvi mwdoc mwwhere mwisupper \
	mwmodlist mwsysmaclist mwrm mwrnwordmod mwmodsearch mwmodbibtex

SYSDATATARGET = data/mwmakedoc_foot.tex data/mwmakedoc_foot_index.tex data/mwmakedoc_macros.tex \
		data/mwmakedoc_macros_html.tex data/mwmakedoc_body.tex data/mwmakedoc_body_html.tex\
		data/mwmakedoc_header_latex2.09.tex data/mwmakedoc_header_latex2e.tex data/mwmakedoc_list_html.tex\
		data/mwmakedoc_list.tex data/mwmakedoc_reference.tex data/mwmakedoc_reference_html.tex\
		data/mwmakedoc_bib.tex	data/mwdoclatex_header.tex data/mwdoclatex_bib.tex data/mwdoclatex_foot.tex \
		data/mwrnwordmod.data data/cxmw2_header.c data/cxmw2_foot.c

# Files to install for MegaWave2 Administrator only 

TARGET = $(SYSTARGET)  mwmodinstall mwcontrib \
	mwcontrib_all mwrnauthor mwrnauthor_all cxmw2debug \
	mwmodcheck mwconfig mwsetenv mwincvers mwinstall mwcleandistrib \
	mwcmwcheck


SRC    = $(TARGET) 
MODE = 022
RELEASE = 4

install :
	mkdir -p $(DESTDIR)
	for i in $(TARGET);\
	do DESTDIR=`echo $(PRIVATE_MEGAWAVE2)|sed '1,$$s/\//\\\\\\//g'`;\
	   sed "1,\$$s/%DESTDIR%/$$DESTDIR/g" $$i >$(DESTDIR)/$$i;\
	   chmod u+x $(DESTDIR)/$$i;\
	done
	mkdir -p  $(DESTDATADIR)
	cp $(SYSDATATARGET) $(DESTDATADIR)

sysinstall :
	umask $(MODE)
	mkdir -m 755 -p $(SYSDESTDIR)
	for i in $(SYSTARGET);\
	do DESTDIR=`echo $(PUBLIC_MEGAWAVE2)|sed '1,$$s/\//\\\\\\//g'`;\
	   sed "1,\$$s/%DESTDIR%/$$DESTDIR/g" $$i >$(SYSDESTDIR)/$$i;\
	   chmod 755 $(SYSDESTDIR)/$$i;\
	done
	mkdir -m 755 -p  $(SYSDESTDATADIR)
	cp $(SYSDATATARGET) $(SYSDESTDATADIR)
	chmod 644 $(SYSDESTDATADIR)/*





