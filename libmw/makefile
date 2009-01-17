#! make -f
#
# makefile for the megawave library
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)

BASEDIR	= ..
COMMONDIR	= $(BASEDIR)/common
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
        utils.c         \
        mw_main.c       \
        mwio.c          \
        type_conv.c     \
        window.c        \
        wpanel.c        \
	)

AUTOHDR	= $(SRC:.c=.h)
HDR	= $(AUTOHDR) $(addprefix $(SRCDIR)/,	\
	tiff.h strtoul.h ppmr_io.h pm.h tiffio.h \
	wpanel-defs.h libmw-defs.h)
OBJ	= $(SRC:.c=.o)
#DEP	= $(SRC:.c=.d)

LIBNAME	= mw
API	= $(LIBNAME).h
APITAG	:= _`echo -n $(API) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_

IPATH	+= $(BASEDIR)/libmw-wdevice

# FIXME: hacks
CWARN	+= -Wno-float-equal

#
# BIN
#

.PHONY	: lib
lib 	: lib$(LIBNAME).$(LIB_EXT)

lib$(LIBNAME).$(STLIB_EXT)	: $(OBJ)
	@$(ECHO) '$(AR) ru $$(OBJ)'
	@$(AR) ru $@ $?
lib$(LIBNAME).$(DYLIB_EXT)	: $(OBJ)
	@$(ECHO) '$(LDWRAP) $(LD) -fPIC -shared $(LDFLAGS) -o $@ $$(OBJ)'
	@$(LDWRAP) $(LD) -fPIC -shared $(LDFLAGS) -o $@ $^

.PHONY	: api
api 	: $(API)
$(API) 	: $(AUTOHDR) $(addprefix $(SRCDIR)/, wpanel-defs.h libmw-defs.h)
	$(ECHO) -e "/*\n * $(LIBNAME) api header\n */\n" > $@
	$(ECHO) -e "#ifndef $(APITAG)\n#define $(APITAG)\n" >> $@
	$(ECHO) -e "\n#include <stdio.h> /* for FILE */\n" >> $@
	$(CAT) $(filter %-defs.h, $^) >> $@
	$(CAT) $(filter-out %-defs.h, $^) >> $@
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
	@echo '$(RM) $$(OBJ)'
	@$(RM) $(OBJ)
	$(RM) lint.flag beautify.flag

.PHONY	: distclean-bin
distclean-bin : clean-bin
	$(RM) lib$(LIBNAME).a
	$(RM) lib$(LIBNAME).so

# include the prerequisites files
# they will be built and re-read if they are missing
#-include $(DEP)


#
# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: autohdr man #dep

.PHONY	: autohdr
autohdr	: $(AUTOHDR)

#.PHONY	: dep
#dep	: $(DEP)


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
		echo "INPUT = $(SRCDIR), $(IPATH)"; \
		echo "GENERATE_HTML = YES"; \
		echo "HTML_OUTPUT = $(DOCDIR)/html";) | $(DOXYGEN) -
$(DOCDIR)/latex/refman.tex	: $(SRC) $(HDR)
	$(MKDIR) $(DOCDIR)/latex
	(cat $(DOXYCONF); \
		echo "INPUT = $(SRCDIR), $(IPATH)"; \
		echo "GENERATE_LATEX = YES"; \
		echo "LATEX_OUTPUT = $(DOCDIR)/latex";) | $(DOXYGEN) -
$(DOCDIR)/srcdoc.pdf : $(DOCDIR)/latex/refman.tex
	$(MAKE) -C $(DOCDIR)/latex refman.pdf
	mv $(DOCDIR)/latex/refman.pdf $(DOCDIR)/srcdoc.pdf

