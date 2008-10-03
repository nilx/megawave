#!/usr/bin/make -f
#
# makefile for megawave 3, mwplight section
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)

COMMONDIR	= .
include $(COMMONDIR)/makefile.common

#
# TOP-LEVEL
#

.PHONY	: default all lint clean distclean
default	: lib modules
all	: lib modules doc test
clean	: clean-bin clean-doc
clean	: distclean-bin distclean-doc

#
# BIN
#

MODDIR	= compression curve image signal wave
SRC	:= $(sort $(shell find $(MODDIR) -type f	\
	-name "*.c" ! -name "*.lib.c" ! -name "*.exec.c"))

LIBSRC	= $(SRC:.c=.lib.c)
BINSRC	= $(SRC:.c=.exec.c)
LIBOBJ	= $(LIBSRC:.c=.o)
BINOBJ	= $(BINSRC:.c=.o)

MWPLIGHT	= ../kernel/mwplight/mwplight
LIBMW	= ../kernel/lib/libmw.so

LIBNAME	= libmw-modules
LIBMOD	= $(LIBNAME).so
MODULES	= $(BINOBJ:.o=)

LIBS 	+= m tiff X11 mw
LPATH	+= ../kernel/lib
# hacks
IPATH	+= ./include.tmp ../kernel/lib ../kernel/Wdevice
CFLAGS	+= -DM_PI=3 -DM_PI_2=9 -DM_PI_4=81

$(MWPLIGHT)	:
	$(MAKE) -C ../kernel/mwplight $(notdir $@)
$(LIBMW)	:
	$(MAKE) -C ../kernel/lib $(notdir $@)


.PHONY	: modules
modules	: $(MODULES)

.PHONY	: lib
lib 	: $(LIBMOD)
$(LIBMOD)	: $(LIBOBJ)
	$(LD) -shared $(LDFLAGS) -o $@ $^

# RULES

%.lib.c	: %.c $(MWPLIGHT)
	$(MWPLIGHT) --source $< --library $@
%.exec.c	: %.c $(MWPLIGHT)
	$(MWPLIGHT) --source $< --exec $@

%.lib.o	: %.lib.c
	$(CC) -c $(CFLAGS) -o $@ $<
%.exec.o	: %.exec.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.exec	: %.exec.o $(LIBMW)
	$(LD) $(LDFLAGS) -o $@ $<

.PHONY	: clean-bin
clean-bin :
	$(RM) $(LIBSRC)
	$(RM) $(BINSRC)
	$(RM) $(LIBOBJ)
	$(RM) $(BINOBJ)
	$(RM) $(DEP)

.PHONY	: distclean-bin
distclean-bin : clean-bin
	$(RM) $(LIBMOD)

# include the prerequisites files
# they will be built and re-read if they are missing
-include $(DEP)

#
# TEST
#

TEST	= test/checkmodules.sh
TESTMODDIR	= test/modules.tmp

.PHONY	: test
test	: $(MODULES)
	$(MKDIR) $(TESTMODDIR)
	cp $^ $(TESTMODDIR)
	env MODDIR=$(TESTMODDIR) sh $(TEST)
	$(RRM) $(TESTMODDIR)

#
# DOC
#


MAN	= $(DOCDIR)/$(LIBNAME).man

.PHONY	: doc
doc	: man

.PHONY	: manpage
manpage	: $(MAN)

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
	$(RM) $(MAN)
