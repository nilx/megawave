#!/usr/bin/make -f
#
# makefile for megawave 3, lib section
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)

include ../makefile.common

#FIXME : Wfloat-equal tolerance
CSTRICT	+= -Wno-float-equal

#
# TOP-LEVEL
#

.PHONY	: default lint beautify all \
          install install-all uninstall uninstall-all clean distclean
default	: bin
all	: bin test doc
lint	: lint-bin lint-test lint-doc
beautify	: beautify-bin beautify-test beautify-doc
clean 	: clean-bin clean-test clean-doc
distclean	: distclean-bin distclean-test distclean-doc

#
# BIN
#

SRC	:= $(addprefix $(SRCDIR)/, \
		ascii_file.c \
		basic_conv.c \
		bmp_io.c \
		ccimage.c \
		ccimage_io.c \
		ccmovie.c \
		ccmovie_io.c \
		cfimage.c \
		cfimage_io.c \
		cfmovie.c \
		cfmovie_io.c \
		cimage.c \
		cimage_io.c \
		cmimage.c \
		cmimage_io.c \
		cmovie.c \
		cmovie_io.c \
		curve.c \
		curve_io.c \
		dcurve.c \
		dcurve_io.c \
		epsf_io.c \
		fcurve.c \
		fcurve_io.c \
		file_type.c \
		fimage.c \
		fimage_io.c \
		fmovie.c \
		fmovie_io.c \
		fpolygon.c \
		fpolygon_io.c \
		fsignal.c \
		fsignal_io.c \
		gif_io.c \
		jpeg_io.c \
		libmw.c \
		list.c \
		list_io.c \
		mimage.c \
		mimage_io.c \
		module.c \
		module_io.c \
		mwio.c \
		pgm_io.c \
		pm_io.c \
		polygon.c \
		polygon_io.c \
		ppm_io.c \
		ps_io.c \
		rawdata.c \
		rawdata_io.c \
		shape.c \
		shape_io.c \
		tiff_io.c \
		type_conv.c \
		wave_io.c \
		wmax2d.c \
		wmax2d_io.c \
		wpack2d.c \
		wpack2d_io.c \
		wtrans1d.c \
		wtrans1d_io.c \
		wtrans2d.c \
		wtrans2d_io.c )
#		window.c \
		wpanel.c \

HDR	= $(addprefix include/, $(notdir $(SRC:.c=.h)))
IPATH	= ./include

BINNAME	= libmw

.PHONY	: bin
bin 	: $(SRCDIR)/$(BINNAME).so
$(SRCDIR)/$(BINNAME).so 	: $(OBJ)
	$(LD) $(LDFLAGS) -shared -o $@ $^

.PHONY	: lint-bin
lint-bin	: lint-bin-flag
lint-bin-flag	: $(SRC) $(HDR)
	$(LINT) $(LINTFLAGS) $?
	$(MAKE) $(addsuffix +charset, $?)
	$(MAKE) $(addsuffix +linelength, $?)
	touch lint-bin-flag

.PHONY	: beautify-bin
beautify-bin	: beautify-bin-flag
beautify-bin-flag	: $(SRC) $(HDR) $(EXTRASRC) $(EXTRAHDR)
	$(MAKE) $(addsuffix +cleanup, $?)
	touch beautify-bin-flag

.PHONY	: clean-bin
clean-bin :
	$(RM) $(OBJ)
	$(RM) $(DEP)
	$(RM) lint-bin-flag beautify-bin-flag

.PHONY	: distclean-bin
distclean-bin : clean-bin
	$(RM) $(SRCDIR)/$(BINNAME).so

# include the prerequisites files
# they will be built and re-read if they are missing
-include $(DEP)

#####################################################################
# TEST
#####################################################################

.PHONY	: test clean-test distclean-test

#####################################################################
# DOC
#####################################################################

MAN	= $(DOCDIR)/$(BINNAME).man

.PHONY	: doc
doc	: srcdoc

.PHONY	: srcdoc
srcdoc	: $(DOXDIR)/html/index.html $(DOXDIR)/srcdoc.pdf
$(DOXDIR)/html/index.html	: $(SRC) $(HDR)
	$(MKDIR) $(DOXDIR)/html
	(cat $(DOXYCONF); \
		echo "INPUT = $(SRCDIR), $(IPATH)"; \
		echo "GENERATE_HTML = YES"; \
		echo "HTML_OUTPUT = $(DOXDIR)/html";) | $(DOXYGEN) -
$(DOXDIR)/latex/refman.tex	: $(SRC) $(HDR)
	$(MKDIR) $(DOXDIR)/latex
	(cat $(DOXYCONF); \
		echo "INPUT = $(SRCDIR), $(IPATH)"; \
		echo "GENERATE_LATEX = YES"; \
		echo "LATEX_OUTPUT = $(DOXDIR)/latex";) | $(DOXYGEN) -
$(DOXDIR)/srcdoc.pdf : $(DOXDIR)/latex/refman.tex
	$(MAKE) -C $(DOXDIR)/latex refman.pdf
	mv $(DOXDIR)/latex/refman.pdf $(DOXDIR)/srcdoc.pdf

.PHONY	: clean-doc
clean-doc	:
	$(RM) $(DOXDIR)/html/*.md5 $(DOXDIR)/html/*.dot
	$(RRM) $(DOXDIR)/latex

.PHONY	: distclean-doc
distclean-doc	: clean-doc
	$(RM) $(MAN) $(MANHTML)
	$(RRM) $(DOXDIR)

