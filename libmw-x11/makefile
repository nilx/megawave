#! make -f
#
# makefile for the megawave X11 library
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
SRC	= $(addprefix $(SRCDIR)/, wpanel.c window.c wdevice.c wdevice-misc.c)

AUTOHDR	= $(SRC:.c=.h)
HDR	= $(AUTOHDR) $(SRCDIR)/definitions.h $(SRCDIR)/config.h
OBJ	= $(SRC:.c=$(.O))
DEP	= $(SRC:.c=.d)

LIBNAME	= mw-x11
API	= $(LIBNAME).h
APITAG	:= _`echo -n $(API) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_

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
	$(RM) $(INCDIR)/$(API)
	$(RM) $(LIBDIR)/lib$(LIBNAME)$(.A)
	$(RM) $(LIBDIR)/lib$(LIBNAME)$(.SO)

# include the prerequisites files
# they will be built and re-read if they are missing
#-include $(DEP)

#
# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: autohdr #dep

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
