# %Z%MegaWave %R%, %M% %I%, %G%
#-------------------------------------------
# Makefile for the MegaWave2 system macros
# (c)1995-99 Jacques Froment
#-------------------------------------------
SHELL = /bin/sh

# Megawave shell makefile

MWBINDIR = $(PRIVATE_MEGAWAVE2)/sys/shell
SYSDESTDIR  = $(PUBLIC_MEGAWAVE2)/sys/shell

# Files to install for all users

SYSTARGET =  .mw2_help_lg_com cmw2 cmw2_all mwvers mw2_macros.tex \
	smw2_foot.tex smw2_header.tex smw2latex mwnewuser mwechon \
	mwarch xmw2_header.c xmw2_foot.c cxmw2 mwdep\
        mw2_header.tex mw2_foot.tex mw2_index-header.tex make_mw2doc \
	cmw2macro smw2xdvi mwdoc mwwhere mwisupper mwmodlist

# Files to install for MegaWave2 Administrator only 

TARGET = $(SYSTARGET)  mwmodinstall mwcontrib \
	mwcontrib_all mwrnauthor mwrnauthor_all cxmw2debug \
	mwmodcheck mwconfig mwsetenv mwincvers mwinstall mwcleandistrib


SRC    = $(TARGET) 
MODE = 022
RELEASE = 4

install :
	mkdir -p $(MWBINDIR)
	for i in $(TARGET);\
	do DESTDIR=`echo $(PRIVATE_MEGAWAVE2)|sed '1,$$s/\//\\\\\\//g'`;\
	   sed "1,\$$s/%DESTDIR%/$$DESTDIR/g" $$i >$(MWBINDIR)/$$i;\
	   chmod u+x $(MWBINDIR)/$$i;\
	done

sysinstall :
	umask $(MODE)
	mkdir -m 755 -p $(SYSDESTDIR)
	for i in $(SYSTARGET);\
	do DESTDIR=`echo $(PUBLIC_MEGAWAVE2)|sed '1,$$s/\//\\\\\\//g'`;\
	   sed "1,\$$s/%DESTDIR%/$$DESTDIR/g" $$i >$(SYSDESTDIR)/$$i;\
	   chmod 755 $(SYSDESTDIR)/$$i;\
	done





