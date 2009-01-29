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
clean	: clean-bin clean-test clean-doc
distclean	: distclean-bin distclean-doc #distclean-test


SRCDIR	= src
SRC	= $(addprefix $(SRCDIR)/,				\
	tree.c parser.c instruction.c io.c header.c usage.c	\
	executable.c interface.c library.c documentation.c	\
	mwp.c)
CMDLINE	= $(SRCDIR)/mwp_cmdline.c

AUTOHDR	= $(SRC:.c=.h)
HDR	= $(AUTOHDR) $(CMDLINE:.c=.h)				\
	$(SRCDIR)/definitions.h $(SRCDIR)/config.h
OBJ	= $(SRC:.c=$(.O)) $(CMDLINE:.c=$(.O))
#DEP	= $(SRC:.c=.d)

BINNAME	= mwp

#
# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: autohdr cmdline man #dep

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
bin 	: $(BINNAME)
$(BINNAME) 	: $(OBJ)
	$(LDWRAP) $(LD) $(LDFLAGS) -o $@ $^

# FIXME: unused parameters and long string
$(CMDLINE:.c=$(.O))	: $(CMDLINE)
	$(CCWRAP) $(CC) $(CFLAGS_NOWARN) -c -o $@ $<

.PHONY	: lint
lint	: lint.flag
lint.flag	: $(SRC) $(HDR)
	$(LINT) $(LINTFLAGS) $?
	$(MAKE) $(addsuffix +charset, $?)
	$(MAKE) $(addsuffix +linelength, $?)
	@touch $@

# TODO: add `indent`
.PHONY	: beautify
beautify	: beautify.flag
beautify.flag	: $(SRC) $(HDR)
	$(MAKE) $(addsuffix +cleanup, $?)
	@touch $@

.PHONY	: clean-bin
clean-bin :
	@$(ECHO) '$(RM) $$(OBJ)'
	@$(RM) $(OBJ)
	$(RM) lint-flag beautify-flag

.PHONY	: distclean-bin
distclean-bin : clean-bin
	$(RM) $(SRCDIR)/*.ggo
	$(RM) $(SRCDIR)/mwp_cmdline.c
	$(RM) $(SRCDIR)/mwp_cmdline.h
	$(RM) $(BINNAME)

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

DOCDIR	= doc
MAN	= $(DOCDIR)/$(BINNAME).man
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
		echo "INPUT = $(SRCDIR)"; \
		echo "GENERATE_HTML = YES"; \
		echo "HTML_OUTPUT = $(DOCDIR)/html";) | $(DOXYGEN) -
$(DOCDIR)/latex/refman.tex	: $(SRC) $(HDR)
	$(MKDIR) $(DOCDIR)/latex
	(cat $(DOXYCONF); \
		echo "INPUT = $(SRCDIR)"; \
		echo "GENERATE_LATEX = YES"; \
		echo "LATEX_OUTPUT = $(DOCDIR)/latex";) | $(DOXYGEN) -
$(DOCDIR)/srcdoc.pdf : $(DOCDIR)/latex/refman.tex
	$(MAKE) -C $(DOCDIR)/latex refman.pdf
	mv $(DOCDIR)/latex/refman.pdf $(DOCDIR)/srcdoc.pdf

