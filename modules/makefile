#!/usr/bin/make -f
#
# makefile for megawave 3, mwplight section
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)

BASEDIR	= ..
COMMONDIR	= $(BASEDIR)/common
include $(COMMONDIR)/makefile

#
# TOP-LEVEL
#

.PHONY	: default all lint clean distclean
default	: lib api modules
all	: lib api modules doc test
clean	: clean-bin clean-doc
distclean	: distclean-bin distclean-doc


# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: autohdr api dep man

.PHONY	: autohdr
#autohdr	: $(AUTOHDR)

.PHONY	: dep
dep	: $(DEP)

#
# BIN
#

SRCDIR	= ./src
SRC	:= $(sort $(shell find $(SRCDIR) -type f	\
	-name "*.c" ! -name "*.lib.c" ! -name "*.exec.c"))

LIBSRC	= $(SRC:.c=.lib.c)
LIBHDR	= $(LIBSRC:.c=.h)
BINSRC	= $(SRC:.c=.exec.c)
BINHDR	= $(BINSRC:.c=.h)
AUTOHDR	= $(LIBHDR) $(BINHDR)
LIBOBJ	= $(LIBSRC:.c=.o)
BINOBJ	= $(BINSRC:.c=.o)

# no need for cproto and makedepend
.PHONY	: $(AUTOHDR:.h=.d)

MWPLIGHT	= $(BASEDIR)/mwplight/mwplight
LIBMW	= $(BASEDIR)/libmw/libmw.so
LIBMW_WDEVICE	= $(BASEDIR)/libmw-wdevice/libmw-wdevice.so

MODULES	= $(BINOBJ:.o=)

LIBNAME	= mw-modules
LIBMW_MODULES	= lib$(LIBNAME).so
API	= $(LIBNAME).h
APITAG	:= _`echo -n $(LIBNAME) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_
APIINC	= <mw.h>


LIBS 	+= m tiff X11 mw
LPATH	+= $(BASEDIR)/libmw $(BASEDIR)/libmw-wdevice
IPATH	+= $(BASEDIR)/libmw $(BASEDIR)/libmw-wdevice

$(MWPLIGHT)	:
	$(MAKE) -C $(BASEDIR)/mwplight $(notdir $@)
$(LIBMW)	:
	$(MAKE) -C $(BASEDIR)/libmw $(notdir $@)

.PHONY	: modules
modules	: $(MODULES)

.PHONY	: lib
lib 	: $(LIBMW_MODULES)
$(LIBMW_MODULES)	: $(LIBOBJ)
	$(LD) -shared $(LDFLAGS) -o $@ $^

.PHONY	: api
api 	: $(API)
$(API) 	: $(LIBHDR)
	$(ECHO) -e "/*\n * $(LIBNAME) api header\n */\n" > $@
	$(ECHO) -e "#ifndef $(APITAG)\n#define $(APITAG)\n" >> $@
	$(ECHO) -e "$(addprefix \n#include , $(APIINC))\n" >> $@
	$(CAT) $^ >> $@
	$(ECHO) -e "\n#endif /* !$(APITAG) */" >> $@

# RULES

%.lib.c	: %.c $(MWPLIGHT)
	$(MWPLIGHT) --source $< --library $@
%.exec.c	: %.c $(MWPLIGHT)
	$(MWPLIGHT) --source $< --exec $@

%.lib.o	: %.lib.c
	$(CC) -c $(CFLAGS) -o $@ $<
%.exec.o	: %.exec.c
	$(CC) -c $(CFLAGS) -o $@ $<

#ifdef STATIC
#%.exec	: %.exec.c $(LIBMW) $(LIBMW_WDEVICE) $(LIBMW_MODULES)
#	$(CC) $(CCFLAGS) -static -I../libmw -I. -I../libmw-wdevice -L../libmw -o $@ $^
#else
%.exec	: %.exec.o $(LIBMW) $(LIBMW_WDEVICE) $(LIBMW_MODULES)
	$(LD) $(LDFLAGS) -lmw-wdevice -lmw-modules -o $@ $<
#endif

.PHONY	: clean-bin
clean-bin :
	$(RM) $(LIBSRC)
	$(RM) $(BINSRC)
	$(RM) $(LIBOBJ)
	$(RM) $(BINOBJ)
	$(RM) $(DEP)

.PHONY	: distclean-bin
distclean-bin : clean-bin
	$(RM) $(LIBMW_MODULES)
	$(RM) $(MODULES)

# include the prerequisites files
# they will be built and re-read if they are missing
-include $(DEP)

#
# TEST
#

TEST	= test/checkmodules.sh
TESTMODDIR	= /mnt/tmp/mwmodules
LIBPATH	= ./:$(BASEDIR)/libmw:$(BASEDIR)/libmw-wdevice

.PHONY	: test
test	: $(MODULES)
	$(MKDIR) $(TESTMODDIR)
	cp $(MODULES) $(TESTMODDIR)
	mmv "$(TESTMODDIR)/*.exec" "$(TESTMODDIR)/#1"
	env MODDIR=$(TESTMODDIR)			\
		DATA=$(BASEDIR)/data			\
		SAMPLES=$(BASEDIR)/data/PUBLIC		\
		PATH=$$PATH:$(TESTMODDIR)		\
	  LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:$(LIBPATH)	\
		/bin/sh $(TEST)
	$(RRM) $(TESTMODDIR)

#
# DOC
#

DOCDIR	= ./doc
MAN	= $(DOCDIR)/$(LIBNAME).man
MANHTML	= $(MAN).html

.PHONY	: doc
doc	: man

.PHONY	: man
man	: $(MAN) $(MANHTML)

#.PHONY	: srcdoc
#srcdoc	: $(DOXDIR)/html/index.html $(DOXDIR)/srcdoc.pdf
#$(DOXDIR)/html/index.html	: $(SRC) $(HDR)
#	$(MKDIR) $(DOXDIR)/html
#	(cat $(DOXYCONF); \
#		echo "INPUT = $(SRCDIR)"; \
#		echo "GENERATE_HTML = YES"; \
#		echo "HTML_OUTPUT = $(DOXDIR)/html";) | $(DOXYGEN) -
#$(DOXDIR)/latex/refman.tex	: $(SRC) $(HDR)
#	$(MKDIR) $(DOXDIR)/latex
#	(cat $(DOXYCONF); \
#		echo "INPUT = $(SRCDIR)"; \
#		echo "GENERATE_LATEX = YES"; \
#		echo "LATEX_OUTPUT = $(DOXDIR)/latex";) | $(DOXYGEN) -
#$(DOXDIR)/srcdoc.pdf : $(DOXDIR)/latex/refman.tex
#	$(MAKE) -C $(DOXDIR)/latex refman.pdf
#	mv $(DOXDIR)/latex/refman.pdf $(DOXDIR)/srcdoc.pdf

.PHONY	: clean-doc
clean-doc	:

.PHONY	: distclean-doc
distclean-doc	: clean-doc
	$(RM) $(MAN) $(MANHTML)
