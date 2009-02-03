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
clean	: clean-bin
distclean	: distclean-bin

#
# BIN
#

SRCDIR	= ./src
SRC	:= $(sort $(shell find $(SRCDIR) -type f	\
	-name "*.c" ! -name "*.cmd.c"))

HDR	= $(SRC:.c=.h)
OBJ	= $(SRC:.c=$(.O))

CMDSRC	= $(SRC:.c=.cmd.c)
CMDOBJ	= $(CMDSRC:.c=$(.O))

LIBNAME	= mw-modules
API	= $(LIBNAME).h
APITAG	:= _`echo -n $(LIBNAME) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_

# no need for cproto and makedepend
#.PHONY	: $(AUTOHDR:.h=.d)

MODEXE	= $(SRC:.c=.cmd)
BUILDDIR = build
MODULES	= $(addprefix $(BUILDDIR)/, $(notdir $(SRC:.c=)))

LIBMW_MODULES	= lib$(LIBNAME)
LIBMW	= $(BASEDIR)/libmw/libmw
LIBMW_X11	= $(BASEDIR)/libmw-x11/libmw-x11
LIBMW_CMDLINE	= $(BASEDIR)/libmw-cmdline/libmw-cmdline
MWP	= $(BASEDIR)/mwp/mwp

ifeq	($(LINK), static)
SHLIBS	= c m X11 tiff
STLIBS	=
# for "heavy" static:
#SHLIBS	= c m dl X11
#STLIBS	= tiff jpeg z
else
SHLIBS	= c m X11 tiff
STLIBS	=
LPATH	+= $(BASEDIR)/libmw $(BASEDIR)/libmw-x11 $(BASEDIR)/libmw-cmdline
endif

IPATH	+= $(BASEDIR)/libmw $(BASEDIR)/libmw-x11 $(BASEDIR)/libmw-cmdline

$(MWP)	:
	$(MAKE) -C $(BASEDIR)/mwp $(notdir $<)
$(LIBMW).%	:
	$(MAKE) -C $(BASEDIR)/libmw $(notdir $<)
$(LIBMW_X11).%	:
	$(MAKE) -C $(BASEDIR)/libmw-x11 $(notdir $<)
$(LIBMW_CMDLINE).%	:
	$(MAKE) -C $(BASEDIR)/libmw-cmdline $(notdir $<)

.PHONY	: modules
modules	: $(MODULES)

.PHONY	: lib
lib 	: lib$(LIBNAME)$(.A) lib$(LIBNAME)$(.SO)

lib$(LIBNAME)$(.A)	: $(OBJ)
	@$(ECHO) '$(AR) ru $@ $$(OBJ)'
	@$(AR) ru $@ $?
lib$(LIBNAME)$(.SO)	: $(OBJ)
	@$(ECHO) '$(LDWRAP) $(LD) -fPIC $(LDFLAGS_SHARED) -o $@ $$(OBJ)'
	@$(LDWRAP) $(LD) -fPIC $(LDFLAGS_SHARED) -o $@ $^

.PHONY	: api
api	: $(API)
$(API) 	: $(HDR)
	$(ECHO) -e "/*\n * $(LIBNAME) api header\n */\n" > $@
	$(ECHO) -e "#ifndef $(APITAG)\n#define $(APITAG)\n" >> $@
	$(ECHO) -e "/* TODO: drop, only required by fview() */" >> $@
	$(ECHO) -e "#include \"mw-x11.h\"\n" >> $@
	@$(ECHO) '$(SED) "/#.*$$/d" $$(HDR) | $(SED) "/^$$/N;/\n$$/D" >> $@'
	@$(SED) '/#.*$$/d' $^ | $(SED) '/^$$/N;/\n$$/D' >> $@
	$(ECHO) -e "\n#endif /* !$(APITAG) */" >> $@

# RULES

%.cmd.c	: %.c $(MWP)
	$(MWP) --source $< --exec $@

ifeq	($(LINK), static)
# order of the libraries matters
# see http://gcc.gnu.org/ml/gcc-help/2004-04/msg00106.html
%.cmd	: %.cmd.o $(LIBMW_MODULES)$(.A) \
	$(LIBMW)$(.A) $(LIBMW_X11)$(.A) $(LIBMW_CMDLINE)$(.A) \
	$(addsuffix $(.A), $(addprefix /usr/lib/lib, $(STLIBS)))
	$(LDWRAP) $(LD) $(LDFLAGS) -o $@ $^
else
%.cmd	: %.cmd.o $(LIBMW_MODULES)$(.SO) \
	$(LIBMW)$(.SO) $(LIBMW_X11)$(.SO) $(LIBMW_CMDLINE)$(.SO)
	$(LDWRAP) $(LD) $(LDFLAGS) \
	-lmw -lmw-x11 -lmw-cmdline -lmw-modules -o $@ $<
endif

makefile.dep	: $(SRC)
	$(RM) $@
	$(TOUCH) $@
	@echo 'for SRC in $$(SRC); do \
		echo "$(BUILDDIR)/`basename $$SRC .c`\t: $${SRC%.c}.cmd" \
		>> $@; \
	done'
	@for SRC in $^; do \
		echo "$(BUILDDIR)/`basename $$SRC .c`\t: $${SRC%.c}.cmd" \
		>> $@; \
	done

include makefile.dep
$(MODULES)	:
	$(CP) $< $@

.PHONY	: clean-bin
clean-bin :
	@echo '$(RM) $$(OBJ) $$(CMDOBJ)'
	@$(RM) $(OBJ) $(CMDOBJ)

# TODO fixme, remove the header and cmdline sources
.PHONY	: distclean-bin
distclean-bin : clean-bin
	@echo '$(RM) $$(MODULES)'
	@$(RM) $(MODULES)
	$(RM) lib$(LIBNAME)$(.A)
	$(RM) lib$(LIBNAME)$(.SO)
	$(RM) $(BUILDDIR)/*
	$(RM) $(API)
	$(RM) makefile.dep

# include the prerequisites files
# they will be built and re-read if they are missing
#-include $(DEP)

#
# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: $(HDR) $(API) makefile.dep

#.PHONY	: dep
#dep	: $(DEP)

#
# TEST
#

TEST	= test/checkmodules.sh
LIBPATH	= ./:$(BASEDIR)/libmw:$(BASEDIR)/libmw-x11:$(BASEDIR)/libmw-cmdline

TESTENV	= PATH=$(BUILDDIR):$$PATH	\
	DATA=$(BASEDIR)/data		\
	SAMPLES=$(BASEDIR)/data/PUBLIC	\
	SCRIPTS=$(BASEDIR)/scripts

ifeq	($(LINK), dynamic)
TESTENV	+= LD_LIBRARY_PATH=$(LIBPATH)
endif

.PHONY	: test
test	: $(MODULES)
	env $(TESTENV) $(SH) $(TEST)

#
# DOC
#

DOCNAME	= lib$(LIBNAME)
DOXY_OVERRIDE	= \
RECURSIVE        = YES\nSHOW_DIRECTORIES = YES\nSTRIP_FROM_PATH = $(SRCDIR)\n

include	$(COMMONDIR)/makefile.doc

