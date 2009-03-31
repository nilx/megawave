#! make -f
#
# makefile for the megawave commandline library
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008-2009)

BASEDIR	= ..
COMMONDIR	= $(BASEDIR)/common
include	$(COMMONDIR)/makefile

#
# TOP-LEVEL
#

.PHONY	: default all clean distclean
default	: lib api
all	: beautify lint lib api test man
clean 	: clean-bin
distclean	: distclean-bin


SRCDIR	= src
SRC	= $(addprefix $(SRCDIR)/, commandline.c)

AUTOHDR	= $(SRC:.c=.h)
HDR	= $(AUTOHDR)
OBJ	= $(SRC:.c=$(.O))
DEP	= $(SRC:.c=.d)

LIBNAME	= mw3-cmdline
API	= $(LIBNAME).h
APITAG	:= _`echo -n $(API) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_

SHLIBS	= mw3

IPATH	+= $(INCDIR)
LPATH	+= $(LIBDIR)

#
# BIN
#

.PHONY	: lib
lib 	: $(LIBDIR)/lib$(LIBNAME)$(.A) $(LIBDIR)/lib$(LIBNAME)$(.SO)

$(LIBDIR)/lib$(LIBNAME)$(.A)	: $(OBJ)
	$(MKDIR) $(@D)
	@$(ECHO) '$(AR) ru $@ $$(OBJ)'
	@$(AR) ru $@ $?
$(LIBDIR)/lib$(LIBNAME)$(.SO)	: $(OBJ)
	$(MKDIR) $(@D)
	@$(ECHO) '$(LDWRAP) $(LD) -fPIC $(LDFLAGS_SHARED) -o $@ $$(OBJ)'
	@$(LDWRAP) $(LD) -fPIC $(LDFLAGS_SHARED) -o $@ $^

.PHONY	: api
api 	: $(INCDIR)/$(API)
$(INCDIR)/$(API) 	: $(SRCDIR)/$(API)
	$(MKDIR) $(@D)
	$(CP) $< $@

.PHONY	: lint
lint	: lint.flag charset.flag linelength.flag
lint.flag	: $(SRC) $(HDR)
#	$(LINT) $(LINT_FLAGS) $?
	@touch $@
charset.flag	: $(SRC) $(HDR)
	$(MAKE) $(addsuffix +charset, $?)
	@touch $@
linelength.flag	: $(SRC) $(HDR)
	$(MAKE) $(addsuffix +linelength, $?)
	@touch $@

.PHONY	: beautify
beautify	: beautify.flag
beautify.flag	: $(SRC) $(HDR) $(SRCDIR)/$(API)
	$(MAKE) $(addsuffix +style, $?)
	$(MAKE) $(addsuffix +cleanup, $?)
	@touch $@

.PHONY	: clean-bin
clean-bin :
	@echo '$(RM) $$(OBJ)'
	@$(RM) $(OBJ)
	$(RM) lint.flag charset.flag linelength.flag beautify.flag

.PHONY	: distclean-bin
distclean-bin : clean-bin
	$(RM) $(API)
	$(RM) lib$(LIBNAME)$(.A)
	$(RM) lib$(LIBNAME)$(.SO)

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

DOCNAME	= lib$(LIBNAME)
MANSECTION	= 3
include	$(COMMONDIR)/makefile.doc
