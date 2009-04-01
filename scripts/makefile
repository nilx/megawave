#! make -f
#
# makefile for megawave scripts
#
# Author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2009)

BASEDIR	= ..
COMMONDIR	= $(BASEDIR)/common
include	$(COMMONDIR)/makefile

SCRIPTS	= $(addsuffix .sh, cfezw cfwivq cfwvq clear_base fezw fwivq \
	fwlbg_adap fwvq km_demo mkmovie put_to_base read_base \
	recognize sr_demo swtvdenoise vp_demo wp2dcheck)
WRAPPER	= mw3

.PHONY	: install
install	: $(SCRIPTS) $(WRAPPER).sh
	$(INSTALL) -d $(DESTDIR)/share/megawave3/scripts/
	$(INSTALL) $(SCRIPTS) $(DESTDIR)/share/megawave3/scripts
	$(SED) 's,%%DESTDIR%%,$(DESTDIR),' $(WRAPPER).sh > $(WRAPPER)
	$(INSTALL) -d $(DESTDIR)/bin/
	$(INSTALL) $(WRAPPER) $(DESTDIR)/bin
	$(RM) $(WRAPPER)