#! make -f
#
# makefile for megawave user manual
#
# Author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>

COMMON_DIR	= ../common
include	$(COMMON_DIR)/makefile

MANUAL	= user_manual

MANUAL_CHAP	= $(addprefix chapter_, $(shell seq 8))
MANUAL_DEP	= license.tex 	\
	$(COMMON_DIR)/head.inc.tex $(COMMON_DIR)/body.inc.tex
MANUAL_SRC	= $(addsuffix .tex, $(MANUAL) $(MANUAL_CHAP))	\
	$(MANUAL_DEP)

MANUAL_PDF	= $(MANUAL).pdf
MANUAL_PS	= $(MANUAL).ps
MANUAL_HTML	= $(MANUAL).html
MANUAL_SPLITHTML	= $(MANUAL)_html
MANUAL_TXT	= $(MANUAL).txt

MANUAL_ALL	= $(MANUAL_PDF) $(MANUAL_PS) \
	$(MANUAL_HTML) $(MANUAL_SPLITHTML)	\
	$(MANUAL_TXT)

# ACTIONS

.PHONY : default all pdf ps html splithtml txt
default	: pdf html splithtml
all	: pdf ps html splithtml txt
pdf	: $(MANUAL_PDF)
ps	: $(MANUAL_PS)
html	: $(MANUAL_HTML)
splithtml	: $(MANUAL_SPLITHTML)
txt	: $(MANUAL_TXT)

$(MANUAL_PDF) $(MANUAL_HTML)	: $(MANUAL_SRC)
