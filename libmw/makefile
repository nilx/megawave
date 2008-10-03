#! make -f
#
# makefile for the megawave library
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)

BASEDIR	= ..
COMMONDIR	= $(BASEDIR)/kernel/common
include	$(COMMONDIR)/makefile

#
# TOP-LEVEL
#

.PHONY	: default all clean distclean
default	: lib api
all	: beautify lint lib api test man
lint	: lint-bin lint-test lint-doc
clean 	: clean-bin clean-doc
distclean	: distclean-bin distclean-doc

# TODO: drop unused or unwanted (module, unix_bsd, ...)
# TODO: libmw/libmw-cmdline/libmw-wpanel(with libmw-wdevice?)
SRCDIR	= src
SRC	:= $(addprefix $(SRCDIR)/, \
        ccimage.c       \
        ccimage_io.c    \
        cfimage.c       \
        cfimage_io.c    \
        cimage.c        \
        cimage_io.c     \
        fimage.c        \
        fimage_io.c     \
        \
        ccmovie.c       \
        ccmovie_io.c    \
        cfmovie.c       \
        cfmovie_io.c    \
        cmovie.c        \
        cmovie_io.c     \
        fmovie.c        \
        fmovie_io.c     \
        \
        cmimage.c       \
        cmimage_io.c    \
        mimage.c        \
        mimage_io.c     \
        \
        curve.c         \
        curve_io.c      \
        dcurve.c        \
        dcurve_io.c     \
        fcurve.c        \
        fcurve_io.c     \
        fpolygon.c      \
        fpolygon_io.c   \
        polygon.c       \
        polygon_io.c    \
        shape.c         \
        shape_io.c      \
        \
        fsignal.c       \
        fsignal_io.c    \
	\
        list.c          \
        list_io.c       \
	\
        wave_io.c       \
        wmax2d.c        \
        wmax2d_io.c     \
        wpack2d.c       \
        wpack2d_io.c    \
        wtrans1d.c      \
        wtrans1d_io.c   \
        wtrans2d.c      \
        wtrans2d_io.c   \
        \
        module.c        \
        module_io.c     \
	\
        rawdata.c       \
        rawdata_io.c    \
        \
        ascii_file.c    \
        basic_conv.c    \
        file_type.c     \
        \
        bmp_io.c        \
        epsf_io.c       \
        gif_io.c        \
        jpeg_io.c       \
        pgm_io.c        \
        pm_io.c         \
        ppm_io.c        \
        ps_io.c         \
        tiff_io.c       \
        \
        libmw.c         \
        mw.c            \
        mw_main.c       \
        mwio.c          \
        type_conv.c     \
        window.c        \
        wpanel.c        \
	)

AUTOHDR	= $(SRC:.c=.h)
HDR	= $(AUTOHDR) $(addprefix $(SRCDIR)/,	\
	tiff.h strtoul.h ppmr_io.h pm.h tiffio.h \
	wpanel-defs.h libwp-defs.h)
OBJ	= $(SRC:.c=.o)
DEP	= $(SRC:.c=.d)

LIBNAME	= mw
LIBMW	= lib$(LIBNAME).so
API	= $(LIBNAME).h
APITAG	:= _`echo -n $(API) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_
APIINC	= <stdio.h>

IPATH	+= $(BASEDIR)/libmw-wdevice

# FIXME: hacks
CWARN	+= -Wno-float-equal

# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: autohdr dep man

.PHONY	: autohdr
autohdr	: $(AUTOHDR)

.PHONY	: dep
dep	: $(DEP)

#
# BIN
#

.PHONY	: lib
lib 	: $(LIBMW)
$(LIBMW) 	: $(OBJ)
	$(LD) -shared $(LDFLAGS) -o $@ $^

.PHONY	: api
api 	: $(API)
$(API) 	: $(SRC) $(SRCDIR)/wpanel-defs.h $(SRCDIR)/libmw-defs.h
	$(ECHO) -e "/*\n * $(LIBNAME) api header\n */\n" > $@
	$(ECHO) -e "#ifndef $(APITAG)\n#define $(APITAG)\n" >> $@
	$(ECHO) -e "$(addprefix \n#include , $(APIINC))\n" >> $@
	$(CAT) $(filter %.h, $^) >> $@
	$(CPROTO) $(CPROTO_FLAGS) $(filter %.c, $^) >> $@
	$(ECHO) -e "\n#endif /* !$(APITAG) */" >> $@

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
	$(RM) lint.flag beautify.flag

.PHONY	: distclean-bin
distclean-bin : clean-bin
	$(RM) $(LIBMW)
	$(RM) $(API)
	$(RM) $(AUTOHDR)

# include the prerequisites files
# they will be built and re-read if they are missing
-include $(DEP)


#
# TEST
#

.PHONY	: test


#
# DOC
#

DOCDIR	= doc
MAN	= $(DOCDIR)/lib$(LIBNAME).man
MANHTML	= $(DOCDIR)/lib$(LIBNAME).man

.PHONY	: man
man	: $(MAN) $(MANHTML)

# .PHONY	: srcdoc
# srcdoc	: $(DOXDIR)/html/index.html $(DOXDIR)/srcdoc.pdf
# $(DOXDIR)/html/index.html	: $(SRC) $(HDR)
# 	$(MKDIR) $(DOXDIR)/html
# 	(cat $(DOXYCONF); \
# 		echo "INPUT = $(SRCDIR), $(IPATH)"; \
# 		echo "GENERATE_HTML = YES"; \
# 		echo "HTML_OUTPUT = $(DOXDIR)/html";) | $(DOXYGEN) -
# $(DOXDIR)/latex/refman.tex	: $(SRC) $(HDR)
# 	$(MKDIR) $(DOXDIR)/latex
# 	(cat $(DOXYCONF); \
# 		echo "INPUT = $(SRCDIR), $(IPATH)"; \
# 		echo "GENERATE_LATEX = YES"; \
# 		echo "LATEX_OUTPUT = $(DOXDIR)/latex";) | $(DOXYGEN) -
# $(DOXDIR)/srcdoc.pdf : $(DOXDIR)/latex/refman.tex
# 	$(MAKE) -C $(DOXDIR)/latex refman.pdf
# 	mv $(DOXDIR)/latex/refman.pdf $(DOXDIR)/srcdoc.pdf

.PHONY	: clean-doc
clean-doc	:
#	$(RM) $(DOXDIR)/html/*.md5 $(DOXDIR)/html/*.dot
#	$(RRM) $(DOXDIR)/latex

.PHONY	: distclean-doc
distclean-doc	: clean-doc
	$(RM) $(MAN) $(MANHTML)

