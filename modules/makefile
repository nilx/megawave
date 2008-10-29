#!/usr/bin/make -f
#
# makefile for the megawave modules
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
prebuild	: autohdr api man #dep

.PHONY	: autohdr
autohdr	: $(AUTOHDR)

#.PHONY	: dep
#dep	: $(DEP)

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
#.PHONY	: $(AUTOHDR:.h=.d)

MODULES	= $(BINOBJ:.o=)

BUILDDIR = build

LIBNAME	= mw-modules
API	= $(LIBNAME).h
APITAG	:= _`echo -n $(LIBNAME) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_
APIINC	= <mw.h>

LIBMW_MODULES	= lib$(LIBNAME)
LIBMW	= $(BASEDIR)/libmw/libmw
LIBMW_WDEVICE	= $(BASEDIR)/libmw-wdevice/libmw-wdevice
MWPLIGHT	= $(BASEDIR)/mwplight/mwplight


LIBS 	+= m tiff X11
LPATH	+= $(BASEDIR)/libmw $(BASEDIR)/libmw-wdevice
IPATH	+= $(BASEDIR)/libmw $(BASEDIR)/libmw-wdevice

$(MWPLIGHT)	:
	$(MAKE) -C $(BASEDIR)/mwplight $(notdir $<)
$(LIBMW).%	:
	$(MAKE) -C $(BASEDIR)/libmw $(notdir $<)
$(LIBMW_WDEVICE).%	:
	$(MAKE) -C $(BASEDIR)/libmw-wdevice $(notdir $<)

.PHONY	: modules
modules	: $(MODULES)
	$(MKDIR) $(BUILDDIR)
	for MODNAME in $(MODULES); do	\
		cp $$MODNAME $(BUILDDIR)/`basename $$MODNAME .exec`; \
	done;

.PHONY	: lib
lib 	: lib$(LIBNAME).so lib$(LIBNAME).a
lib$(LIBNAME).a	: $(LIBOBJ)
	$(AR) ru $@ $^
lib$(LIBNAME).so	: $(LIBOBJ)
	$(LD) -fPIC -shared $(LDFLAGS) -o $@ $^

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
	$(TOUCH) -m -d "`$(STAT) -c %y $<`" $@
%.exec.c	: %.c $(MWPLIGHT)
	$(MWPLIGHT) --source $< --exec $@
	$(TOUCH) -m -d "`$(STAT) -c %y $<`" $@

%.lib.o	: %.lib.c
	$(CC) -c $(CFLAGS) -o $@ $<
%.exec.o	: %.exec.c
	$(CC) -c $(CFLAGS) -o $@ $<

ifeq	($(STATIC), 1)
%.exec	: %.exec.o $(LIBMW).a $(LIBMW_WDEVICE).a $(LIBMW_MODULES).a
	$(LD) $(LDFLAGS) -o $@ $^
else
%.exec	: %.exec.o $(LIBMW).so $(LIBMW_WDEVICE).so $(LIBMW_MODULES).so
	$(LD) $(LDFLAGS) -lmw -lmw-wdevice -lmw-modules -o $@ $<
endif

.PHONY	: clean-bin
clean-bin :
	$(RM) $(LIBSRC)
	$(RM) $(BINSRC)
	$(RM) $(LIBOBJ)
	$(RM) $(BINOBJ)
	$(RM) $(DEP)

.PHONY	: distclean-bin
distclean-bin : clean-bin
	$(RM) $(MODULES)
	$(RM) lib$(LIBNAME).a
	$(RM) lib$(LIBNAME).so
	$(RRM) $(BUILDDIR)

# include the prerequisites files
# they will be built and re-read if they are missing
#-include $(DEP)

#
# TEST
#

TEST	= test/checkmodules.sh
TESTMODDIR	= /mnt/tmp/mwmodules
LIBPATH	= ./:$(BASEDIR)/libmw:$(BASEDIR)/libmw-wdevice

TESTENV	= MODDIR=$(TESTMODDIR)			\
	BASEDIR=$(BASEDIR)			\
	DATA=$(BASEDIR)/data			\
	SAMPLES=$(BASEDIR)/data/PUBLIC		\
	SCRIPTS=$(BASEDIR)/scripts		\
	PATH=$$PATH:$(TESTMODDIR)

ifeq	($(STATIC), 0)
TESTENV	+= LD_LIBRARY_PATH=$(LIBPATH)
endif

.PHONY	: test
test	: $(MODULES)
	$(RRM) $(TESTMODDIR)
	$(MKDIR) $(TESTMODDIR)
	cp $(MODULES) $(TESTMODDIR)
	mmv "$(TESTMODDIR)/*.exec" "$(TESTMODDIR)/#1"
	env $(TESTENV) /bin/sh $(TEST)
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

.PHONY	: srcdoc
srcdoc	: $(DOCDIR)/html/index.html $(DOCDIR)/srcdoc.pdf
$(DOCDIR)/html/index.html	: $(SRC)
	$(MKDIR) $(DOCDIR)/html
	(cat $(DOXYCONF); \
		echo "INPUT         = $(SRCDIR)"; \
		echo "FILE_PATTERNS = *.lib.c"; \
		echo "RECURSIVE     = YES"; \
		echo "GENERATE_HTML = YES"; \
		echo "HTML_OUTPUT   = $(DOCDIR)/html";) | $(DOXYGEN) -
$(DOCDIR)/latex/refman.tex	: $(SRC)
	$(MKDIR) $(DOCDIR)/latex
	(cat $(DOXYCONF); \
		echo "INPUT          = $(SRCDIR)"; \
		echo "FILE_PATTERNS = *.lib.c"; \
		echo "RECURSIVE      = YES"; \
		echo "GENERATE_LATEX = YES"; \
		echo "LATEX_OUTPUT   = $(DOCDIR)/latex";) | $(DOXYGEN) -
$(DOCDIR)/srcdoc.pdf : $(DOCDIR)/latex/refman.tex
	$(MAKE) -C $(DOCDIR)/latex refman.pdf
	mv $(DOCDIR)/latex/refman.pdf $(DOCDIR)/srcdoc.pdf

.PHONY	: clean-doc
clean-doc	:

.PHONY	: distclean-doc
distclean-doc	: clean-doc
	$(RM) $(MAN) $(MANHTML)
	$(RM) $(DOCDIR)/srcdoc.pdf
	$(RRM) $(DOCDIR)/html $(DOCDIR)/latex

