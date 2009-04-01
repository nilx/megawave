#! make -f
#
# makefile for megawave data
#
# Author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2009)

BASEDIR	= ..
COMMONDIR	= $(BASEDIR)/common
include	$(COMMONDIR)/makefile

DATADIRLIST	=  examples/curves examples/images examples/signals \
	examples/movies utils/image utils/shape_recognition \
	utils/wave/ortho utils/wave/ortho/edge \
	utils/wave/packets/ortho \
	utils/wave/packets/biortho/h utils/wave/packets/biortho/htilde

.PHONY	: install
install	:
	for DIR in $(DATADIRLIST); do \
		$(INSTALL) -d $(DESTDIR)/share/megawave3/data/$${DIR}; \
		$(INSTALL) $$($(FIND) $$DIR -maxdepth 1 -type f) \
			$(DESTDIR)/share/megawave3/data/$${DIR}; \
	done

