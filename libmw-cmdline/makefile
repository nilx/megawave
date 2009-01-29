#! make -f
#
# makefile for the megawave commandline library
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2009)

BASEDIR	= ..
COMMONDIR	= $(BASEDIR)/common
include	$(COMMONDIR)/makefile

#
# TOP-LEVEL
#

.PHONY	: default all clean distclean
default	: lib api
all	: beautify lint lib api test man
clean 	: clean-bin clean-doc
distclean	: distclean-bin distclean-doc


SRCDIR	= src
SRC	= $(addprefix $(SRCDIR)/, commandline.c)

AUTOHDR	= $(SRC:.c=.h)
HDR	= $(AUTOHDR) # $(SRCDIR)/definitions.h $(SRCDIR)/config.h
OBJ	= $(SRC:.c=$(.O))
DEP	= $(SRC:.c=.d)

LIBNAME	= mw-cmdline
API	= $(LIBNAME).h
APITAG	:= _`echo -n $(API) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_

IPATH	+= $(BASEDIR)/libmw

#
# BIN
#

.PHONY	: lib
lib 	: lib$(LIBNAME)$(.A) lib$(LIBNAME)$(.SO)

lib$(LIBNAME)$(.A)	: $(OBJ)
	@$(ECHO) '$(AR) ru $$(OBJ) $^'
	@$(AR) ru $@ $?
lib$(LIBNAME)$(.SO)	: $(OBJ)
	@$(ECHO) '$(LDWRAP) $(LD) -fPIC $(LDFLAGS_SHARED) -o $@ $$(OBJ)'
	@$(LDWRAP) $(LD) -fPIC $(LDFLAGS_SHARED) -o $@ $^

.PHONY	: api
api 	: $(API)
$(API) 	: $(HDR)
	$(ECHO) -e "/*\n * $(LIBNAME) api header\n */\n" > $@
	$(ECHO) -e "#ifndef $(APITAG)\n#define $(APITAG)\n" >> $@
#	$(CAT) $(filter %definitions.h, $^) >> $@
	$(CAT) $(filter-out %definitions.h, $^) >> $@
	$(ECHO) -e "\n#endif /* !$(APITAG) */" >> $@


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
	@echo '$(RM) $$(OBJ)'
	@$(RM) $(OBJ)
	$(RM) lint.flag beautify.flag

.PHONY	: distclean-bin
distclean-bin : clean-bin
	$(RM) lib$(LIBNAME)$(.A)
	$(RM) lib$(LIBNAME)$(.SO)

# include the prerequisites files
# they will be built and re-read if they are missing
#-include $(DEP)

#
# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: autohdr man api #dep

.PHONY	: autohdr
autohdr	: $(AUTOHDR)
	@echo $(AUTOHDR)

#.PHONY	: dep
#dep	: $(DEP)


#
# TEST
#

.PHONY	: test

#
# DOC
#


DOCDIR	= doc
MAN	= $(DOCDIR)/lib$(LIBNAME).man
MANHTML	= $(MAN).html

.PHONY	: man
man	: $(MAN) $(MANHTML)

.PHONY	: clean-doc
clean-doc	:
	$(RM) $(DOCDIR)/html/*.md5 $(DOCDIR)/html/*.dot
	$(RRM) $(DOCDIR)/latex

.PHONY	: distclean-doc
distclean-doc	: clean-doc
	$(RM) $(MAN) $(MANHTML)
	$(RRM) $(DOCDIR)/html
	$(RRM) $(DOCDIR)/srcdoc.pdf

.PHONY	: srcdoc
srcdoc	: prebuild $(DOCDIR)/html/index.html $(DOCDIR)/srcdoc.pdf
$(DOCDIR)/html/index.html	: $(SRC) $(HDR)
	$(MKDIR) $(DOCDIR)/html
	(cat $(DOXYCONF); \
		echo "INPUT = $(SRCDIR), $(IPATH)"; \
		echo "GENERATE_HTML = YES"; \
		echo "HTML_OUTPUT = $(DOCDIR)/html";) | $(DOXYGEN) -
$(DOCDIR)/latex/refman.tex	: $(SRC) $(HDR)
	$(MKDIR) $(DOCDIR)/latex
	(cat $(DOXYCONF); \
		echo "INPUT = $(SRCDIR), $(IPATH)"; \
		echo "GENERATE_LATEX = YES"; \
		echo "LATEX_OUTPUT = $(DOCDIR)/latex";) | $(DOXYGEN) -
$(DOCDIR)/srcdoc.pdf : $(DOCDIR)/latex/refman.tex
	$(MAKE) -C $(DOCDIR)/latex refman.pdf
	mv $(DOCDIR)/latex/refman.pdf $(DOCDIR)/srcdoc.pdf
