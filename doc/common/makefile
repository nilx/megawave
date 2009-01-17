#! make -f
#
# common makefile for megawave manuals
#
# Author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>

.PHONY	: default
default	:

# TOOLS

RM	= /bin/rm -f
RRM	= /bin/rm -rf
MV	= /bin/mv -f
MKDIR	= /bin/mkdir -p
CP	= /bin/cp -f
SED	= /bin/sed
ECHO	= /bin/echo
CAT	= /bin/cat

CSPLIT	= /usr/bin/csplit
PDFLATEX	= /usr/bin/pdflatex
PDF2PS	= /usr/bin/pdf2ps
MAKEINDEX	= /usr/bin/makeindex
HEVEA	= /usr/bin/hevea
HACHA	= /usr/bin/hacha
HTML2TEXT	= /usr/bin/html2text
RECODE	= /usr/bin/recode

PDFLATEX_FLAGS	=
HEVEA_FLAGS	= -fix -O
HEVEA_CSS	= $(COMMON_DIR)/manual.css
HACHA_FLAGS	= -tocbis
HTML2TEXT_FLAGS	= -width 72 -style compact -nobs 	\
	-rcfile $(COMMON_DIR)/html2textrc

# ACTIONS

# TODO: add lint
.PHONY	: clean distclean
clean :
	$(RM) *.log *.aux *.out *.toc *.ilg
	$(RM) *.hind *.haux *.htoc
distclean : clean
	$(RRM) $(MANUAL_ALL)

# RULES

# latex index
%.idx : %.tex
	$(PDFLATEX) $(PDFLATEX_FLAGS) $<

%.ind : %.idx
	$(MAKEINDEX) $*.idx

# tex to pdf
%.pdf : %.tex %.ind
	$(PDFLATEX) $(PDFLATEX_FLAGS) $<
	$(PDFLATEX) $(PDFLATEX_FLAGS) $<

# pdf to ps
%.ps : %.pdf
	$(PDF2PS) $< $@

# tex to html
%.html : %.tex
	$(HEVEA) $(HEVEA_FLAGS) -o $@ $<
	$(MAKE) $@+fixhevea

# html to splitted html
%_html : %.html
	$(MKDIR) $*_html
	$(HACHA) $(HACHA_FLAGS) -o $@/index.html $<
	$(MAKE) $*_html+fixhacha

# html to txt
%.txt : %.html
	$(CP) $< $<.ascii
	$(RECODE) -q -f HTML..ASCII $<.ascii
	$(HTML2TEXT) $(HTML2TEXT_FLAGS) -o $@ $<.ascii
	$(RM) $<.ascii

# fix hevea output
%+fixhevea : %
	$(SED) -i "s/.*This document was translated from.*//" $<
	$(SED) -i "s/.*hevea.inria.fr.*/<\/BODY>/" $<
	$(CSPLIT) -s -f $< $< /STYLE/ /STYLE/
	$(ECHO) "<STYLE type=\"text/css\">" > $<01
	$(CAT) $(HEVEA_CSS) >> $<01
	$(CAT) $<00 $<01 $<02 > $<
	$(RM) $<00 $<01 $<02

# fix hacha output
%+fixhacha : %
	$(SED) -i 's|<IMG SRC="contents_motif.gif" ALT="Up">|<abbr title="table of contents" class="navigation">\&uarr;</abbr>|' $*/*.html
	$(SED) -i 's|<IMG SRC="previous_motif.gif" ALT="Previous">|<abbr title="previous chapter" class="navigation">\&larr;</abbr>|' $*/*.html
	$(SED) -i 's|<IMG SRC="next_motif.gif" ALT="Next">|<abbr title="next chapter" class="navigation">\&rarr;</abbr>|' $*/*.html
	$(RM) $*/*.gif
