#!/usr/bin/make -f
#
# global common makefile for megawave
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)

default :
.SUFFIXES	:

# TOOLS PATH

SH	= /bin/sh
CP	= /bin/cp -f
RM	= /bin/rm -f
RRM	= /bin/rm -r -f
MV	= /bin/mv -f
MKDIR	= /bin/mkdir -p
SED	= /bin/sed
CAT	= /bin/cat
ECHO	= /bin/echo

CC	= /usr/bin/gcc
CCWRAP	= 
LD	= $(CC)
LDWRAP	= 
AR	= /usr/bin/ar
MAKEDEP	= /usr/bin/gcc -MM
CPROTO	= /usr/bin/cproto
GENGETOPT	= /usr/bin/gengetopt

TOUCH	= /usr/bin/touch
STAT	= /usr/bin/stat
FIND	= /usr/bin/find
CSPLIT	= /usr/bin/csplit
INSTALL	= /usr/bin/install
LINT	= /usr/bin/splint
EXPAND	= /usr/bin/expand
DOXYGEN	= /usr/bin/doxygen
PANDOC	= /usr/bin/pandoc
PDFLATEX	= /usr/bin/pdflatex

KERNEL	:= $(shell uname -s)
# note : the kernel architecture and system architecture may differ;
# for example, 32bit system with a 64bit kernel; in this case, the
# system architecture must be used for binary compatibility
KERNEL_ARCH	:= $(shell uname -m)
# debian provides a clean way to get the system architecture from
# dpkg; for other distributons, we rely on a fragile hack
SYSTEM_ARCH	:= $(shell [ -x /usr/bin/dpkg ] \
			&& dpkg --print-architecture \
			|| ( file /bin/ls | cut -d: -f2 | cut -d, -f2 ))

# for specific targets, uncomment the relevant line
#include $(COMMONDIR)/makefile.osx
#include $(COMMONDIR)/makefile.cygwin
#include $(COMMONDIR)/makefile.debian3

# CC OPTIONS
# C language version
CSTD	?= -ansi
# compiler warning level
CWARN	?= -Wall
# compiler miscellaneous flags
CMISC	?=

CCVERSION	:= $(shell $(CC) --version | head -n 1 | cut -d ' ' -f 3)

CSTRICT_4_1	:= $(shell if [ "$(CCVERSION)" \> "4.1" ]; then   \
                             echo "-Wc++-compat -Wextra";         \
                           fi) # available from gcc 4.1
# code ckecking options
# CHECK can be "relax" or "strict"
CHECK	?= relax
# strict compilation switch
ifeq	($(CHECK), strict)
CWARN 	+= -pedantic -Werror $(CSTRICT_4_1)		\
	-Wfloat-equal -Wundef -Wshadow -Wpointer-arith	\
	-Wbad-function-cast -Wcast-qual -Wcast-align	\
	-Waggregate-return -Wstrict-prototypes		\
	-Wmissing-prototypes				\
	-Wmissing-declarations -Wnested-externs
endif

# compilation mode options
# MODE can be "normal", "opti", "debug" or "profile" 
MODE	?= normal

# debug
ifeq	($(MODE), debug)
CMISC	+= -g
PROF	= 0
endif

# profiling
ifeq	($(MODE), profile)
CMISC	+= -pg
CMISC	+= -O2 -funroll-loops -finline-functions	\
	-ffunction-sections -fdata-sections
endif

# optimisation
ifeq	($(MODE), opti)
CMISC	+= -O2 -funroll-loops -finline-functions	\
	-ffunction-sections -fdata-sections
endif

# compiler flags
IPATH	?= .
CFLAGS_NOWARN	?= $(CSTD) $(CMISC) $(addprefix -I, $(IPATH))
CFLAGS	?= -fPIC -ffunction-sections $(CWARN) $(CFLAGS_NOWARN)

# linking option
# LINK can be "dynamic" or "static"
LINK	?= dynamic

# include and link paths
LPATH	?= .
LPREFIX	?=
# libraries to link with
SHLIBS	?=

# file extentions (objects, static libs, shared libs)
.O	?= .o
.A	?= .a
.SO	?= .so

ifeq	($(LINK), dynamic)
LIB_EXT	?= $(.SO)
else
LIB_EXT	?= $(.A)
endif

# debug
ifeq	($(MODE), debug)
LPREFIX	?= debug/
endif

LDFLAGS	?= -Wl,--gc-sections $(LDDEBUG) $(CPROF)			\
	$(addprefix -L, $(LPATH)) $(addprefix -l$(LPREFIX), $(SHLIBS))
LDFLAGS_SHARED	?= -shared $(LDFLAGS)

# TOOLS OPTIONS

LINT_FLAGS	?= -posixlib -weak -ansi89limits
PANDOC_FLAGS	?= --standalone --reference-links --smart
DOXYCONF	?= $(COMMONDIR)/doxygen.conf
GENGETOPT_FLAGS	?= --func-name=mw_cmdline --arg-struct=mw_args_info	\
	--no-handle-error 						\
	--no-handle-help --no-help					\
	--no-handle-version --no-version
CPROTO_FLAGS	?= $(addprefix -I, $(IPATH)) -q

# GENERIC RULES

# build a prerequisites file
%.d	: %.c %.h
	@set -e; $(RM) $@; \
	    $(MAKEDEP) $(addprefix -I, $(IPATH)) $< > $@; \
	    $(SED) 's|\($*\)\.o[ :]*|\1.o $@ : |g' $@ > $@.$$$$; \
	    $(MV) $@.$$$$ $@;

# std C compilation
%$(.O)	: %.c
	$(CCWRAP) $(CC) $(CFLAGS) -c -o $@ $<

# make a C prototype header
# FIXME: cproto -q is buggy
%.h	:: %.c
	$(CPROTO) $(CPROTO_FLAGS) $< -o $@
	@HNAME=_`echo -n $(notdir $@) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_ \
		&& ($(ECHO) -e "/*\n * $(notdir $@)\n */\n"; \
			$(ECHO) -e "#ifndef $${HNAME}\n#define $${HNAME}\n"; \
			$(CAT) $@; \
			$(ECHO) -e "\n#endif /* !$${HNAME} */";) > $@.$$$$ \
		&& $(MV) $@.$$$$ $@

# markdown to manpage
%.man : %.man.mdwn
	-test -x $(PANDOC) && $(PANDOC) $(PANDOC_FLAGS) -t man -o $@ $<

# markdown to html
%.html : %.mdwn
	-test -x $(PANDOC) && $(PANDOC) $(PANDOC_FLAGS) -t html -o $@ $<


# UTILITIES

# gengetopt
%.c+cmdline	: %.c
	$(CSPLIT) $^ -f $*_split %GENGETOPT\ BEGIN%+1 /GENGETOPT\ END/
	$(SED) 's/^ *\* *//' $*_split00 > $*.ggo
	$(RM) $*_split0[01]
	cd $(dir $@) \
		&& $(GENGETOPT) ${GENGETOPT_FLAGS} \
			-i $(notdir $*).ggo \
			--file-name=$(notdir $*_cmdline)

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
