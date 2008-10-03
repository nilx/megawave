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
LIB	= lib$(LIBNAME).so
API	= $(LIBNAME).h
APITAG	:= _`echo -n $(API) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_

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
lib 	: $(LIB)
$(LIB) 	: $(OBJ)
	$(LD) -shared $(LDFLAGS) -o $@ $^

.PHONY	: api
api 	: $(API)
$(API) 	: $(SRC) $(SRCDIR)/wdevice-defs.h $(SRCDIR)/wdevice-config.h
	$(ECHO) -e "/*\n * $(LIBNAME) api header\n */\n" > $@
	$(ECHO) -e "#ifndef $(APITAG)\n#define $(APITAG)\n" >> $@
	$(CAT) $(filter %.h, $^) >> $@
	$(CPROTO) $(CPROTO_FLAGS) $(filter %.c, $^) >> $@
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
	$(RM) $(OBJ) $(DEP)
	$(RM) lint.flag beautify.flag

.PHONY	: distclean-bin
distclean-bin : clean-bin
	$(RM) $(AUTOHDR)
	$(RM) $(LIB)
	$(RM) $(API)

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
MAN	= $(DOCDIR)/lib$(LIBNAME).man
MANHTML	= $(MAN).html

.PHONY	: man
man	: $(MAN) $(MANHTML)

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
	$(RM) $(MAN) $(MANHTML)

