#! make -f
#
# makefile for the megawave mwplight parser
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)

COMMONDIR	= ../common
include	$(COMMONDIR)/makefile

#
# TOP-LEVEL
#

.PHONY	: default all clean distclean
default	: bin
all	: beautify lint bin test man
clean	: clean-bin clean-test clean-doc
distclean	: distclean-bin distclean-doc


#
# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: autohdr cmdline man

.PHONY	: cmdline
cmdline	: $(CMDLINE)

.PHONY	: autohdr
autohdr	: $(AUTOHDR)

.PHONY	: dep
dep	: $(DEP)

#
# BUILD
#

SRCDIR	= src

CMDLINE	= $(SRCDIR)/main_cmdline.c

SRC	= $(addprefix $(SRCDIR)/, \
	tree.c parse.c instruction.c io.c header.c afile.c ifile.c \
	mfile.c tfile.c present.c main.c)

AUTOHDR	= $(SRC:.c=.h)
HDR	= $(AUTOHDR:.c=.h) $(CMDLINE:.c=.h) $(SRCDIR)/mwpl.h
OBJ	= $(SRC:.c=.o) $(CMDLINE:.c=.o)
DEP	= $(SRC:.c=.d)

BINNAME	= mwplight

.PHONY	: bin
bin 	: $(SRCDIR)/$(BINNAME)
$(SRCDIR)/$(BINNAME) 	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^


# FIXME: unused parameters and long string
$(CMDLINE:.c=.o)	: $(CMDLINE)
	$(CC) -c -o $@ $<

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
	$(RM) lint-flag beautify-flag

.PHONY	: distclean-bin
distclean-bin : clean-bin
	$(RM) $(SRCDIR)/$(BINNAME)
	$(RM) $(SRCDIR)/*.ggo

# include the prerequisites files
# they will be built and re-read if they are missing
-include $(DEP)

#
# TEST
#

TEST_MODULES	= $(shell find ../../src -type f -name *.c)
TEST_ENV	= MWPLIGHT=$(SRCDIR)/$(BINNAME) \
	MODULES="$(TEST_MODULES)"

#TESTS	= $(addprefix test_, run library exec documentation interface name)
TESTS	= $(addprefix test_, run documentation name)

.PHONY	: test
test	: $(addsuffix .flag, $(TESTS))

test_%.flag	: $(SRCDIR)/$(BINNAME)
	@echo test_$*.sh 
	@env $(TEST_ENV) sh test/test_$*.sh
	@touch $@

.PHONY	: clean-test
clean-test :
	$(RM) $(addsuffix .flag, $(TESTS))

#
# DOC
#

DOCDIR	= doc
MAN	= $(DOCDIR)/$(BINNAME).man

.PHONY	: man
man	: $(DOCDIR)/$(BINNAME).man $(DOCDIR)/$(BINNAME).man.html

#.PHONY	: srcdoc
#srcdoc	: $(DOXDIR)/html/index.html $(DOXDIR)/srcdoc.pdf
#$(DOXDIR)/html/index.html	: $(SRC) $(HDR)
#	$(MKDIR) $(DOXDIR)/html
#	(cat $(DOXYCONF); \
#		echo "INPUT = $(SRCDIR)"; \
#		echo "GENERATE_HTML = YES"; \
#		echo "HTML_OUTPUT = $(DOXDIR)/html";) | $(DOXYGEN) -
#$(DOXDIR)/latex/refman.tex	: $(SRC) $(HDR)
#	$(MKDIR) $(DOXDIR)/latex
#	(cat $(DOXYCONF); \
#		echo "INPUT = $(SRCDIR)"; \
#		echo "GENERATE_LATEX = YES"; \
#		echo "LATEX_OUTPUT = $(DOXDIR)/latex";) | $(DOXYGEN) -
#$(DOXDIR)/srcdoc.pdf : $(DOXDIR)/latex/refman.tex
#	$(MAKE) -C $(DOXDIR)/latex refman.pdf
#	mv $(DOXDIR)/latex/refman.pdf $(DOXDIR)/srcdoc.pdf
#

.PHONY	: clean-doc
clean-doc	:
#	$(RM) $(DOXDIR)/html/*.md5 $(DOXDIR)/html/*.dot
#	$(RRM) $(DOXDIR)/latex

.PHONY	: distclean-doc
distclean-doc	: clean-doc
	$(RM) $(DOCDIR)/$(BINNAME).man $(DOCDIR)/$(BINNAME).man.html

