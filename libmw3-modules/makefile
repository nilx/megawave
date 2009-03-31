#!/usr/bin/make -f
#
# makefile for the megawave modules library and executables
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)

BASEDIR	= ..
COMMONDIR	= $(BASEDIR)/common
include $(COMMONDIR)/makefile

#
# TOP-LEVEL
#

.PHONY	: default all lint clean distclean
default	: lib api
all	: lib api modules doc test
clean	: clean-bin
distclean	: distclean-bin

SRCDIR	= ./src
SRC	:= $(sort $(shell find $(SRCDIR) -type f	\
	-name "*.c" ! -name "*.cmd.c"))

HDR	= $(SRC:.c=.h)
OBJ	= $(SRC:.c=$(.O))

CMDSRC	= $(SRC:.c=.cmd.c)
CMDOBJ	= $(CMDSRC:.c=$(.O))
CMDBIN	= $(CMDSRC:.c=)

LIBNAME	= mw3-modules
API	= $(LIBNAME).h
APITAG	:= _`echo -n $(LIBNAME) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_

# no need for cproto and makedepend
#.PHONY	: $(AUTOHDR:.h=.d)

MODEXE	= $(SRC:.c=.cmd)
MODULES	= $(addprefix $(BINDIR)/, $(notdir $(SRC:.c=)))

LIBMW3	= $(LIBDIR)/libmw3
LIBMW3_X11	= $(LIBDIR)/libmw3-x11
LIBMW3_CMDLINE	= $(LIBDIR)/libmw3-cmdline
LIBMW3_MODULES	= $(LIBDIR)/lib$(LIBNAME)
MWP	= $(BINDIR)/mwp

IPATH	+= $(INCDIR)
LPATH	+= $(LIBDIR)

ifeq	($(LINK), static)
SHLIBS	= c m X11 tiff
STLIBS	=
# for "heavy" static:
#SHLIBS	= c m dl X11
#STLIBS	= tiff jpeg z
else
SHLIBS	= c m X11 tiff
STLIBS	=
endif

# FIXME: use clean float tests
CWARN	+= -Wno-float-equal

#
# BIN
#

.PHONY	: modules
modules	: $(MODULES)

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
api	: $(INCDIR)/$(API)
$(INCDIR)/$(API) 	: $(SRCDIR)/$(API)
	$(MKDIR) $(@D)
	$(CP) $< $@

$(OBJ)	: $(INCDIR)/$(API)

# RULES

%.cmd.c	: %.c $(MWP)
	$(MWP) --source $< --exec $@
#	 $(CPP) -C -E -P -nostdinc $< | \
#		$(MWP) --module-name=$(basename $(notdir $<)) \
#		--source - --exec $@

ifeq	($(LINK), static)
# order of the libraries matters
# see http://gcc.gnu.org/ml/gcc-help/2004-04/msg00106.html
%.cmd	: %.cmd.o $(LIBMW3_MODULES)$(.A) \
	$(LIBMW3)$(.A) $(LIBMW3_X11)$(.A) $(LIBMW3_CMDLINE)$(.A) 
	$(addsuffix $(.A), $(addprefix /usr/lib/lib, $(STLIBS)))
	$(LDWRAP) $(LD) $(LDFLAGS) -o $@ $^
else
%.cmd	: %.cmd.o $(LIBMW3_MODULES)$(.SO) \
	$(LIBMW3)$(.SO) $(LIBMW3_X11)$(.SO) $(LIBMW3_CMDLINE)$(.SO) 
	$(LDWRAP) $(LD) $(LDFLAGS) \
	-lmw3 -lmw3-x11 -lmw3-cmdline -lmw3-modules -o $@ $<
endif

makefile.dep	: $(SRC)
	$(RM) $@
	$(TOUCH) $@
	@echo 'for SRC in $$(SRC); do \
		echo "$(BINDIR)/`basename $$SRC .c` : $${SRC%.c}.cmd" \
		>> $@; \
	done'
	@for SRC in $^; do \
		echo "$(BINDIR)/`basename $$SRC .c` : $${SRC%.c}.cmd" \
		>> $@; \
	done

-include makefile.dep
$(MODULES)	:
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
	@echo '$(RM) $$(OBJ) $$(CMDOBJ)'
	@$(RM) $(OBJ) $(CMDOBJ)
	$(RM) lint.flag charset.flag linelength.flag beautify.flag

# TODO fixme, remove the header and cmdline sources
.PHONY	: distclean-bin
distclean-bin : clean-bin
	@echo '$(RM) $$(MODULES)'
	@$(RM) $(MODULES)
	@echo '$(RM) $$(CMDBIN)'
	@$(RM) $(CMDBIN)
	@echo '$(RM) $$(CMDSRC)'
	@$(RM) $(CMDSRC)
	$(RM) $(LIBDIR)/lib$(LIBNAME)$(.A)
	$(RM) $(LIBDIR)/lib$(LIBNAME)$(.SO)
	$(RM) $(INCDIR)/$(API)

# include the prerequisites files
# they will be built and re-read if they are missing
#-include $(DEP)

#
# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: man makefile.dep

#.PHONY	: dep
#dep	: $(DEP)

#
# TEST
#

# TODO: add profiling/coverage wrappers

TEST	= $(BASEDIR)/test/checkmodules.sh
LIBPATH	= $(LIBDIR)

TESTENV	= PATH=$(BINDIR):$$PATH			\
	UTILS=$(BASEDIR)/data/utils		\
	EXAMPLES=$(BASEDIR)/data/examples	\
	IOEXAMPLES=$(BASEDIR)/test/iodata	\
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

DOXY_OVERRIDE	= \
	EXCLUDE_PATTERNS	= *.cmd.*\n\
	RECURSIVE		= YES\n\
	SHOW_DIRECTORIES	= YES\n
DOCNAME	= lib$(LIBNAME)
MANSECTION	= 3
include	$(COMMONDIR)/makefile.doc

