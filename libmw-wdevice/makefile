#! make -f
#
# makefile for the megawave wdevice library
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)

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
SRC	= $(addprefix $(SRCDIR)/, wdevice.c wdevice-misc.c)

AUTOHDR	= $(SRC:.c=.h)
HDR	= $(AUTOHDR) $(SRCDIR)/wdevice-defs.h $(SRCDIR)/wdevice-config.h
OBJ	= $(SRC:.c=.o)
DEP	= $(SRC:.c=.d)

LIBNAME	= mw-wdevice
API	= $(LIBNAME).h
APITAG	:= _`echo -n $(API) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_

#
# BIN
#

.PHONY	: lib
ifeq	($(LINK), static)
lib 	: lib$(LIBNAME).a
else
lib 	: lib$(LIBNAME).so
endif
lib$(LIBNAME).a	: $(OBJ)
	@$(ECHO) '$(AR) ru $$(OBJ) $^'
	@$(AR) ru $@ $?
lib$(LIBNAME).so	: $(OBJ)
	@$(ECHO) '$(LDWRAP) $(LD) -fPIC -shared $(LDFLAGS) -o $@ $$(OBJ)'
	@$(LDWRAP) $(LD) -fPIC -shared $(LDFLAGS) -o $@ $^

.PHONY	: api
api 	: $(API)
$(API) 	: $(HDR)
	$(ECHO) -e "/*\n * $(LIBNAME) api header\n */\n" > $@
	$(ECHO) -e "#ifndef $(APITAG)\n#define $(APITAG)\n" >> $@
	$(CAT) $(filter %-defs.h, $^) >> $@
	$(CAT) $(filter-out %-defs.h, $^) >> $@
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
	$(RM) lib$(LIBNAME).a
	$(RM) lib$(LIBNAME).so

# include the prerequisites files
# they will be built and re-read if they are missing
#-include $(DEP)

#
# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: autohdr man #dep

.PHONY	: autohdr
autohdr	: $(AUTOHDR)
	echo $(AUTOHDR)

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
#	$(RM) $(DOXDIR)/html/*.md5 $(DOXDIR)/html/*.dot
#	$(RRM) $(DOXDIR)/latex

.PHONY	: distclean-doc
distclean-doc	: clean-doc
	$(RM) $(MAN) $(MANHTML)

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
