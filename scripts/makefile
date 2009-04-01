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

.PHONY	: install
install	: $(SCRIPTS)
	$(INSTALL) -d $(DESTDIR)/share/megawave3/scripts/
	$(INSTALL) $^ $(DESTDIR)/share/megawave3/scripts
