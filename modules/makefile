#!/usr/bin/make -f
#
# makefile for megawave 3, mwplight section
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)

include ./makefile.common

#
# TOP-LEVEL
#

.PHONY	: default lint beautify all \
          install install-all uninstall uninstall-all clean distclean
default	: bin
all	: bin doc
lint	: lint-bin lint-doc
beautify	: beautify-bin beautify-doc
clean 	: clean-bin clean-doc
distclean	: distclean-bin distclean-doc


#####################################################################
# BIN
#####################################################################

MODDIR	= compression curve examples image signal wave
MOD	:= $(sort $(shell $(FIND) $(MODDIR) -type f -name "*.c"))
SRC	= $(addprefix tmp/, $(MOD:.c=.a.c)) \
          $(addprefix tmp/, $(MOD:.c=.m.c))
MODOBJ	= $(addprefix tmp/, $(MOD:.c=.m.o))
HDR	=

MODULES = $(addprefix tmp2/, $(MOD:.c=))

MWPLIGHT	= ../kernel/mwplight/src/mwplight

BINNAME	= libmwmodules

IPATH	+= ../kernel/lib/include

.PHONY	: bin
bin 	: $(BINNAME).so
$(BINNAME).so 	: $(MODOBJ)
	$(LD) -shared $(LDFLAGS) -o $@ $^

# RULES
tmp/%.a.c	: %.c $(MWPLIGHT)
	$(MWPLIGHT) -a $@ $<
tmp/%.m.c	: %.c $(MWPLIGHT)
	$(MWPLIGHT) -m $@ $<

.PHONY	: modules
modules	: $(MODULES)
tmp2/%	: tmp/%.a.o
	$(CC) $(CFLAGS) -lX11 -lm \
	./libmwmodules.so ../kernel/Wdevice/src/libWdevice.so \
	../kernel/lib/src/libmw.so -o $@ $<

.PHONY	: clean-bin
clean-bin :
	$(RM) $(OBJ)
	$(RM) $(DEP)
	$(RM) lint-bin-flag beautify-bin-flag

.PHONY	: distclean-bin
distclean-bin : clean-bin
	$(RM) $(SRCDIR)/$(BINNAME)

# include the prerequisites files
# they will be built and re-read if they are missing
#-include $(DEP)

#####################################################################
# DOC
#####################################################################

MAN	= $(DOCDIR)/$(BINNAME).man

.PHONY	: doc
doc	: manpage srcdoc

.PHONY	: manpage
manpage	: $(MAN) $(MANHTML)

.PHONY	: srcdoc
srcdoc	: $(DOXDIR)/html/index.html $(DOXDIR)/srcdoc.pdf
$(DOXDIR)/html/index.html	: $(SRC) $(HDR)
	$(MKDIR) $(DOXDIR)/html
	(cat $(DOXYCONF); \
		echo "INPUT = $(SRCDIR)"; \
		echo "GENERATE_HTML = YES"; \
		echo "HTML_OUTPUT = $(DOXDIR)/html";) | $(DOXYGEN) -
$(DOXDIR)/latex/refman.tex	: $(SRC) $(HDR)
	$(MKDIR) $(DOXDIR)/latex
	(cat $(DOXYCONF); \
		echo "INPUT = $(SRCDIR)"; \
		echo "GENERATE_LATEX = YES"; \
		echo "LATEX_OUTPUT = $(DOXDIR)/latex";) | $(DOXYGEN) -
$(DOXDIR)/srcdoc.pdf : $(DOXDIR)/latex/refman.tex
	$(MAKE) -C $(DOXDIR)/latex refman.pdf
	mv $(DOXDIR)/latex/refman.pdf $(DOXDIR)/srcdoc.pdf

.PHONY	: clean-doc
clean-doc	:
	$(RM) $(DOXDIR)/html/*.md5 $(DOXDIR)/html/*.dot
	$(RRM) $(DOXDIR)/latex

.PHONY	: distclean-doc
distclean-doc	: clean-doc
	$(RM) $(MAN) $(MANHTML)
	$(RRM) $(DOXDIR)
