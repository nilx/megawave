#! make -f
#
# makefile for the megawave parser
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)

BASEDIR	= ..
COMMONDIR	= $(BASEDIR)/common
include	$(COMMONDIR)/makefile

#
# TOP-LEVEL
#

.PHONY	: default all clean distclean
default	: bin
all	: beautify lint bin test man
clean	: clean-bin
distclean	: distclean-bin


SRCDIR	= src
SRC	= $(addprefix $(SRCDIR)/,				\
	tree.c parser.c instruction.c io.c header.c usage.c	\
	executable.c library.c documentation.c mwp.c)
CMDLINE	= $(SRCDIR)/mwp_cmdline.c

AUTOHDR	= $(SRC:.c=.h)
HDR	= $(AUTOHDR) 						\
	$(SRCDIR)/definitions.h $(SRCDIR)/config.h
OBJ	= $(SRC:.c=$(.O)) $(CMDLINE:.c=$(.O))
#DEP	= $(SRC:.c=.d)

BINNAME	= mwp

#
# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: autohdr cmdline #dep

.PHONY	: cmdline
cmdline	: $(SRCDIR)/mwp.c+cmdline

.PHONY	: autohdr
autohdr	: $(AUTOHDR)

#.PHONY	: dep
#dep	: $(DEP)

#
# BUILD
#

.PHONY	: bin
bin 	: $(BINDIR)/$(BINNAME)
$(BINDIR)/$(BINNAME) 	: $(OBJ)
	$(MKDIR) $(@D)
	$(LDWRAP) $(LD) $(LDFLAGS) -o $@ $^

# FIXME: unused parameters and long string
$(CMDLINE:.c=$(.O))	: $(CMDLINE)
	$(CCWRAP) $(CC) $(CFLAGS_NOWARN) -c -o $@ $<

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
beautify.flag	: $(SRC) $(HDR)
	$(MAKE) $(addsuffix +style, $?)
	$(MAKE) $(addsuffix +cleanup, $?)
	@touch $@

.PHONY	: clean-bin
clean-bin :
	@$(ECHO) '$(RM) $$(OBJ)'
	@$(RM) $(OBJ)
	$(RM) lint.flag charset.flag linelength.flag beautify.flag

.PHONY	: distclean-bin
distclean-bin : clean-bin
	$(RM) $(SRCDIR)/*.ggo
	$(RM) $(BINDIR)/$(BINNAME)

# include the prerequisites files
# they will be built and re-read if they are missing
#-include $(DEP)

#
# TEST
#

#TEST_MODULES	= $(shell find $(BASEDIR)/modules/src -type f -name *.c)
#TEST_ENV	= MWPLIGHT=$(BINNAME) 	\
#	MODULES="$(TEST_MODULES)" 			\
#	LIBMWDIR=../libmw				\
#	LIBMW_WDEVICEDIR=../libmw-wdevice		\
#	TESTDIR=test
#
# TODO: test_interface
#TESTS	= $(addprefix test_, run library executable documentation name)
#
#.PHONY	: test
#test	: $(addsuffix .flag, $(TESTS))
#
#test_%.flag	: $(BINNAME)
#	@echo test_$*.sh 
#	@env $(TEST_ENV) sh test/test_$*.sh
#	@touch $@
#
#.PHONY	: clean-test
#clean-test :
#	$(RM) $(addsuffix .flag, $(TESTS))
#
#.PHONY	: distclean-test
#distclean-test : clean-test

#
# DOC
#

DOCNAME	= $(BINNAME)
MANSECTION	= 1
include	$(COMMONDIR)/makefile.doc

