#!/usr/bin/make -f
#
# global common makefile for megawave
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)

default :

# TOOLS PATH

CP	= /bin/cp -f
RM	= /bin/rm -f
RRM	= /bin/rm -r -f
MV	= /bin/mv -f
MKDIR	= /bin/mkdir -p
SED	= /bin/sed
CAT	= /bin/cat
ECHO	= /bin/echo

CC	= /usr/bin/gcc
LD	= /usr/bin/gcc
AR	= /usr/bin/ar
MAKEDEP	= /usr/bin/gcc -MM
CPROTO	= /usr/bin/cproto
GENGETOPT	= /usr/bin/gengetopt

TOUCH	= /usr/bin/touch
STAT	= /usr/bin/stat
FIND	= /usr/bin/find
CSPLIT	= /usr/bin/csplit
TOUCH	= /usr/bin/touch
INSTALL	= /usr/bin/install
LINT	= /usr/bin/splint
EXPAND	= /usr/bin/expand
DOXYGEN	= /usr/bin/doxygen
PANDOC	= /usr/bin/pandoc
PDFLATEX	= /usr/bin/pdflatex

# CC OPTIONS

CCVERSION	:= $(shell  $(CC) --version | head -n 1 | cut -d ' ' -f 3)
CSTRICT_4_1	:= $(shell if [ "$(CCVERSION)" \> "4.1" ]; then   \
                             echo "-Wc++-compat -Wextra";         \
                           fi) # available from gcc 4.1

# C language version
CSTD	= -ansi
# compiler warning level
CWARN	= -Wall
# compiler miscellaneous flags
CMISC	=

# strict compilation switch
STRICT	?= 0
ifeq	($(STRICT), 1)
CWARN 	+= -pedantic -Werror $(CSTRICT_4_1)		\
	-Wfloat-equal -Wundef -Wshadow -Wpointer-arith	\
	-Wbad-function-cast -Wcast-qual -Wcast-align	\
	-Waggregate-return -Wstrict-prototypes		\
	-Wmissing-prototypes				\
	-Wmissing-declarations -Wnested-externs
endif

# compilation switchs
DEBUG	?= 0
PROF	?= 0
OPTI	?= 0
STATIC	?= 0

# debug
ifeq	($(DEBUG), 1)
CMISC	+= -g
PROF	= 0
OPTI	= 0
endif

# profiling
ifeq	($(PROF), 1)
CMISC	+= -pg
OPTI	= 1
endif

# optimisation
ifeq	($(OPTI), 1)
CMISC	+= -O2 -funroll-loops -finline-functions	\
	-ffunction-sections -fdata-sections
endif

# include and link paths
IPATH	= .
LPATH	= .
# libraries to link with
LIBS	=

# compiler flags
CFLAGS	= -fPIC -ffunction-sections				\
	$(CSTD) $(CWARN) $(CMISC) $(addprefix -I, $(IPATH))
CFLAGS_NOWARN	= $(CSTD) $(CMISC) $(addprefix -I, $(IPATH))

# linker flags
# FIXME: gcc syntax
LDFLAGS	= -Wl,--gc-sections					\
	$(LDDEBUG) $(CPROF)					\
	$(addprefix -L, $(LPATH)) $(addprefix -l, $(LIBS))

# TOOLS OPTIONS

LINT_FLAGS	= -posixlib -weak -ansi89limits
PANDOC_FLAGS	= --standalone --reference-links --smart
DOXYCONF	= $(COMMONDIR)/doxygen.conf
GENGETOPT_FLAGS	= --func-name=mw_cmdline --arg-struct=mw_args_info	\
	--no-handle-error 						\
	--no-handle-help --no-help					\
	--no-handle-version --no-version
CPROTO_FLAGS	= $(addprefix -I, $(IPATH)) -q

# GENERIC RULES

# build a prerequisites file
%.d	: %.c %.h
	@set -e; $(RM) $@; \
	    $(MAKEDEP) $(addprefix -I, $(IPATH)) $< > $@; \
	    $(SED) 's|\($*\)\.o[ :]*|\1.o $@ : |g' $@ > $@.$$$$; \
	    $(MV) $@.$$$$ $@;

# std C compilation
%.o	: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# make a C prototype header
# FIXME: cproto -q is buggy
%.h	: %.c
	$(CPROTO) $(CPROTO_FLAGS) $< -o $@
	@HNAME=_`echo -n $(notdir $@) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_ \
		&& ($(ECHO) -e "/*\n * $(notdir $@)\n */\n"; \
			$(ECHO) -e "#ifndef $${HNAME}\n#define $${HNAME}\n"; \
			$(CAT) $@; \
			$(ECHO) -e "\n#endif /* !$${HNAME} */";) > $@.$$$$ \
		&& $(MV) $@.$$$$ $@
	$(TOUCH) -m -d "`$(STAT) -c %y $<`" $@

# extract the command-line description
%.ggo	: %.c
	$(CSPLIT) $^ -f $*_split %GENGETOPT\ BEGIN%+1 /GENGETOPT\ END/
	$(SED) 's/^ *\* *//' $*_split00 > $@
	$(RM) $*_split0[01]

# TODO: one pass
%_cmdline.c	: %.ggo
	cd $(dir $@) \
		&& $(GENGETOPT) ${GENGETOPT_FLAGS} \
			-i $(notdir $^) --file-name=$(notdir $*_cmdline)
%_cmdline.h	: %.ggo
	cd $(dir $@) \
		&& $(GENGETOPT) ${GENGETOPT_FLAGS} \
			-i $(notdir $^) --file-name=$(notdir $*_cmdline)

# markdown to manpage
%.man : %.man.mdwn
	-test -x $(PANDOC) && $(PANDOC) $(PANDOC_FLAGS) -t man -o $@ $<

# markdown to html
%.html : %.mdwn
	-test -x $(PANDOC) && $(PANDOC) $(PANDOC_FLAGS) -t html -o $@ $<


# UTILITIES

# TODO : empty blank lines at the end of the file
%+cleanup : %
	$(EXPAND) $< | $(SED) 's/[ \t]*$$//' > $<.$$$$ \
	    && $(MV) $<.$$$$ $<;

CHARSET	= us-ascii
%+charset : %
	test "`file -b -i $< | cut -d '=' -f 2`" = "$(CHARSET)"

MAXLENGTH	= 80
%+linelength : %
	test `wc -L $< | cut -d ' ' -f 1` -le $(MAXLENGTH)
