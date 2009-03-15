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
lint	: lint-bin lint-test
clean 	: clean-bin
distclean	: distclean-bin

# TODO: drop unused or unwanted (module, unix_bsd, ...)
# TODO: split libmw/libmw-cmdline
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
        error.c         \
	\
        rawdata.c       \
        rawdata_io.c    \
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
        ascii_file.c    \
        basic_conv.c    \
        file_type.c     \
        libmw.c         \
        mwio.c          \
        type_conv.c     \
	)

AUTOHDR	= $(SRC:.c=.h)
HDR	= $(AUTOHDR) $(addprefix $(SRCDIR)/,	\
	tiff.h strtoul.h ppmr_io.h pm.h tiffio.h \
	libmw-defs.h)
OBJ	= $(SRC:.c=$(.O))
#DEP	= $(SRC:.c=.d)

LIBNAME	= mw
API	= $(LIBNAME).h
APITAG	:= _`echo -n $(API) | tr "a-z" "A-Z" | tr -c "A-Z0-9" "_"`_

IPATH	+= $(BASEDIR)/libmw-x11

# FIXME: use clean float tests
CWARN	+= -Wno-float-equal

#
# BIN
#

.PHONY	: lib
lib 	: lib$(LIBNAME)$(.A) lib$(LIBNAME)$(.SO)

lib$(LIBNAME)$(.A)	: $(OBJ)
	@$(ECHO) '$(AR) ru $$(OBJ)'
	@$(AR) ru $@ $?
lib$(LIBNAME)$(.SO)	: $(OBJ)
	@$(ECHO) '$(LDWRAP) $(LD) -fPIC $(LDFLAGS_SHARED) -o $@ $$(OBJ)'
	@$(LDWRAP) $(LD) -fPIC $(LDFLAGS_SHARED) -o $@ $^

.PHONY	: api
api 	: $(API)
$(API) 	: $(SRCDIR)/$(API)
	$(CP) $< $@

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
	$(RM) $(API)
	$(RM) lib$(LIBNAME)$(.A)
	$(RM) lib$(LIBNAME)$(.SO)

# include the prerequisites files
# they will be built and re-read if they are missing
#-include $(DEP)


#
# PREBUILD AND UTILS
#

.PHONY	: prebuild
prebuild	: autohdr man api #dep

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

DOCNAME	= lib$(LIBNAME)
include	$(COMMONDIR)/makefile.doc
