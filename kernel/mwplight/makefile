#! make -f
#
# makefile for megawave 3, mwplight section
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)

COMMONDIR	= ../common
include	$(COMMONDIR)/makefile

#
# TOP-LEVEL
#

.PHONY	: default all lint beautify clean distclean
default	: bin
all	: bin manpage test doc
lint	: lint-bin lint-doc
beautify	: beautify-bin beautify-doc
clean 	: clean-bin clean-doc
distclean	: distclean-bin distclean-doc

#
# BIN
#

SRCDIR	= src
SRC	= $(addprefix $(SRCDIR)/, \
	  tree.c parse.c instruction.c io.c header.c \
          afile.c ifile.c mfile.c tfile.c present.c \
          main.c) $(SRCDIR)/main_cmdline.c

HDR	= $(SRC:.c=.h)
OBJ	= $(SRC:.c=.o)
DEP	= $(SRC:.c=.d)

BINNAME	= mwplight

.PHONY	: bin
bin 	: $(SRCDIR)/$(BINNAME)
$(SRCDIR)/$(BINNAME) 	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

.PHONY	: lint-bin
lint-bin	: lint-bin-flag
lint-bin-flag	: $(SRC) $(HDR)
	$(LINT) $(LINTFLAGS) $?
	$(MAKE) $(addsuffix +charset, $?)
	$(MAKE) $(addsuffix +linelength, $?)
	touch lint-bin-flag

.PHONY	: beautify-bin
beautify-bin	: beautify-bin-flag
beautify-bin-flag	: $(SRC) $(HDR) $(EXTRASRC) $(EXTRAHDR)
	$(MAKE) $(addsuffix +cleanup, $?)
	touch beautify-bin-flag

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
-include $(DEP)

#
# DOC
#

DOCDIR	= doc
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
