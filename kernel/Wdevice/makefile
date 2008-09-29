#! make -f
#
# makefile for the megawave wdevice library
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)

COMMONDIR	= ../common
include	$(COMMONDIR)/makefile

#
# TOP-LEVEL
#

.PHONY	: default all clean distclean
default	: lib
all	: beautify lint lib test man
clean 	: clean-bin clean-doc
distclean	: distclean-bin distclean-doc


SRCDIR	= src

SRC	= $(addprefix $(SRCDIR)/, 				\
	mw-wdevice.c mw-wdevice_misc.c mw-wdevice_var.c)

AUTOHDR	= $(SRC:.c=.h)
HDR	= $(AUTOHDR) $(SRCDIR)/libmw-wdevice.h
OBJ	= $(SRC:.c=.o)
DEP	= $(SRC:.c=.d)

LIBNAME	= libmw-wdevice

#
# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: autohdr dep man

.PHONY	: autohdr
autohdr	: $(AUTOHDR)
	echo $(AUTOHDR)

.PHONY	: dep
dep	: $(DEP)

#
# BIN
#

.PHONY	: lib
lib 	: $(LIBNAME).so
$(LIBNAME).so 	: $(OBJ)
	$(LD) -shared $(LDFLAGS) -o $@ $^

.PHONY	: lint
lint	: lint.flag
lint.flag	: $(SRC) $(HDR)
	$(LINT) $(LINTFLAGS) $?
	$(MAKE) $(addsuffix +charset, $?)
	$(MAKE) $(addsuffix +linelength, $?)
	@touch $@

.PHONY	: beautify
beautify	: beautify.flag
beautify.flag	: $(SRC) $(HDR)
	$(MAKE) $(addsuffix +cleanup, $?)
	@touch $@

.PHONY	: clean-bin
clean-bin :
	$(RM) $(OBJ) $(DEP)
	$(RM) lint-flag beautify-flag

.PHONY	: distclean-bin
distclean-bin : clean-bin
	$(RM) $(LIBNAME).so

# include the prerequisites files
# they will be built and re-read if they are missing
-include $(DEP)


#
# TEST
#

.PHONY	: test

#
# DOC
#


DOCDIR	= doc
MAN	= $(DOCDIR)/$(LIBNAME).man

.PHONY	: man
man	: $(DOCDIR)/$(LIBNAME).man $(DOCDIR)/$(LIBNAME).man.html

# .PHONY	: srcdoc
# srcdoc	: $(DOXDIR)/html/index.html $(DOXDIR)/srcdoc.pdf
# $(DOXDIR)/html/index.html	: $(SRC) $(HDR)
# 	$(MKDIR) $(DOXDIR)/html
# 	(cat $(DOXYCONF); \
# 		echo "INPUT = $(SRCDIR), $(IPATH)"; \
# 		echo "GENERATE_HTML = YES"; \
# 		echo "HTML_OUTPUT = $(DOXDIR)/html";) | $(DOXYGEN) -
# $(DOXDIR)/latex/refman.tex	: $(SRC) $(HDR)
# 	$(MKDIR) $(DOXDIR)/latex
# 	(cat $(DOXYCONF); \
# 		echo "INPUT = $(SRCDIR), $(IPATH)"; \
# 		echo "GENERATE_LATEX = YES"; \
# 		echo "LATEX_OUTPUT = $(DOXDIR)/latex";) | $(DOXYGEN) -
# $(DOXDIR)/srcdoc.pdf : $(DOXDIR)/latex/refman.tex
# 	$(MAKE) -C $(DOXDIR)/latex refman.pdf
# 	mv $(DOXDIR)/latex/refman.pdf $(DOXDIR)/srcdoc.pdf

.PHONY	: clean-doc
clean-doc	:
#	$(RM) $(DOXDIR)/html/*.md5 $(DOXDIR)/html/*.dot
#	$(RRM) $(DOXDIR)/latex

.PHONY	: distclean-doc
distclean-doc	: clean-doc
	$(RM) $(DOCDIR)/$(LIBNAME).man $(DOCDIR)/$(LIBNAME).man.html

