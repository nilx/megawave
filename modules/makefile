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

SRCDIR	= ./src
SRC	:= $(sort $(shell find $(SRCDIR) -type f	\
	-name "*.c" ! -name "*.cmd.c"))

HDR	= $(SRC:.c=.h)
OBJ	= $(SRC:.c=$(.O))

CMDSRC	= $(SRC:.c=.cmd.c)
CMDOBJ	= $(CMDSRC:.c=$(.O))
CMDBIN	= $(CMDSRC:.c=)

LIBNAME	= mw-modules
API	= $(LIBNAME).h
APITAG	:= _`echo -n $(LIBNAME) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_

# no need for cproto and makedepend
#.PHONY	: $(AUTOHDR:.h=.d)

MODEXE	= $(SRC:.c=.cmd)
MODULES	= $(addprefix $(BINDIR)/, $(notdir $(SRC:.c=)))

LIBMW	= $(LIBDIR)/libmw
LIBMW_X11	= $(LIBDIR)/libmw-x11
LIBMW_CMDLINE	= $(LIBDIR)/libmw-cmdline
LIBMW_MODULES	= $(LIBDIR)/lib$(LIBNAME)
MWP	= $(BINDIR)/mwp

ifeq	($(LINK), static)
SHLIBS	= c m X11 tiff
STLIBS	=
# for "heavy" static:
#SHLIBS	= c m dl X11
#STLIBS	= tiff jpeg z
else
SHLIBS	= c m X11 tiff
STLIBS	=
LPATH	+= $(LIBDIR)
endif

IPATH	+= $(INCDIR)

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

.PHONY	: clean-bin
clean-bin :
	@echo '$(RM) $$(OBJ) $$(CMDOBJ)'
	@$(RM) $(OBJ) $(CMDOBJ)

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
	$(RM) makefile.dep

# include the prerequisites files
# they will be built and re-read if they are missing
#-include $(DEP)

#
# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: makefile.dep

#.PHONY	: dep
#dep	: $(DEP)

#
# TEST
#

# TODO: add profiling/coverage wrappers

TEST	= test/checkmodules.sh
LIBPATH	= $(LIBDIR)

TESTENV	= PATH=$(BINDIR):$$PATH			\
	UTILS=$(BASEDIR)/data/utils		\
	EXAMPLES=$(BASEDIR)/data/examples	\
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

