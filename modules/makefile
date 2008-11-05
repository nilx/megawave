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

#
# BIN
#

SRCDIR	= ./src
SRC	:= $(sort $(shell find $(SRCDIR) -type f	\
	-name "*.c" ! -name "*.cmd.c"))

HDR	= $(SRC:.c=.h)
OBJ	= $(SRC:.c=.o)

CMDSRC	= $(SRC:.c=.cmd.c)
CMDOBJ	= $(CMDSRC:.c=.o)

LIBNAME	= mw-modules
API	= $(LIBNAME).h
APITAG	:= _`echo -n $(LIBNAME) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_

# no need for cproto and makedepend
#.PHONY	: $(AUTOHDR:.h=.d)

MODULES	= $(CMDOBJ:.cmd.o=.cmd)

BUILDDIR = build

LIBMW_MODULES	= lib$(LIBNAME)
LIBMW	= $(BASEDIR)/libmw/libmw
LIBMW_WDEVICE	= $(BASEDIR)/libmw-wdevice/libmw-wdevice
MWPLIGHT	= $(BASEDIR)/mwplight/mwplight

ifeq	($(STATIC), 0)
SHLIBS	= c m X11 tiff
STLIBS	=
else
SHLIBS	=
STLIBS	=				\
	tiff jpeg z			\
	X11 xcb xcb-xlib dl Xau Xdmcp	\
	m c
endif

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

.PHONY	: lib
ifeq	($(STATIC), 0)
lib 	: lib$(LIBNAME).so
else
lib 	: lib$(LIBNAME).a
endif
lib$(LIBNAME).a	: $(OBJ)
	@$(ECHO) '$(AR) ru $@ $$(OBJ)'
	@$(AR) ru $@ $?
lib$(LIBNAME).so	: $(OBJ)
	@$(ECHO) '$(LD) -fPIC -shared $(LDFLAGS) -o $@ $$(OBJ)'
	@$(LD) -fPIC -shared $(LDFLAGS) -o $@ $^

.PHONY	: api
api	: $(API)
$(API) 	: $(HDR)
	$(ECHO) -e "/*\n * $(LIBNAME) api header\n */\n" > $@
	$(ECHO) -e "#ifndef $(APITAG)\n#define $(APITAG)\n" >> $@
	@$(ECHO) '$(SED) "/#.*$$/d" $$(HDR) | $(SED) "/^$$/N;/\n$$/D" >> $@'
	@$(SED) '/#.*$$/d' $^ | $(SED) '/^$$/N;/\n$$/D' >> $@
	$(ECHO) -e "\n#endif /* !$(APITAG) */" >> $@

# RULES

%.cmd.c	: %.c $(MWPLIGHT)
	$(MWPLIGHT) --source $< --exec $@

ifeq	($(STATIC), 0)
%.cmd	: %.cmd.o $(LIBMW_MODULES).so $(LIBMW).so $(LIBMW_WDEVICE).so
	$(LD) $(LDFLAGS) -lmw -lmw-wdevice -lmw-modules -o $@ $<
	$(CP) $@ $(BUILDDIR)/$(basename $(notdir $@))
else
# order of the libraries matters
# see http://gcc.gnu.org/ml/gcc-help/2004-04/msg00106.html
%.cmd	: %.cmd.o $(LIBMW_MODULES).a $(LIBMW).a $(LIBMW_WDEVICE).a \
	$(addsuffix .a, $(addprefix /usr/lib/lib, $(STLIBS)))
	$(LD) $(LDFLAGS) -o $@ $^
	$(CP) $@ $(BUILDDIR)/$(basename $(notdir $@))
endif

.PHONY	: clean-bin
clean-bin :
	@echo '$(RM) $$(OBJ) $$(CMDOBJ)'
	@$(RM) $(OBJ) $(CMDOBJ)

.PHONY	: distclean-bin
distclean-bin : clean-bin
	@echo '$(RM) $$(MODULES)'
	@$(RM) $(MODULES)
	$(RM) lib$(LIBNAME).a
	$(RM) lib$(LIBNAME).so
	$(RM) $(BUILDDIR)/*
	$(RM) $(API)

# include the prerequisites files
# they will be built and re-read if they are missing
#-include $(DEP)

#
# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: $(CMDSRC) $(HDR) $(API)

#.PHONY	: dep
#dep	: $(DEP)

#
# TEST
#

TEST	= test/checkmodules.sh
LIBPATH	= ./:$(BASEDIR)/libmw:$(BASEDIR)/libmw-wdevice

TESTENV	= PATH=$(BUILDDIR):$$PATH	\
	DATA=$(BASEDIR)/data		\
	SAMPLES=$(BASEDIR)/data/PUBLIC	\
	SCRIPTS=$(BASEDIR)/scripts

ifeq	($(STATIC), 0)
TESTENV	+= LD_LIBRARY_PATH=$(LIBPATH)
endif

.PHONY	: test
test	: $(MODULES)
	env $(TESTENV) $(SH) $(TEST)

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
