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
MODULES = $(notdir $(shell $(FIND) $(DESTDIR)/lib/megawave3/modules/ -type f))
WRAPPER	= mw3
ALIAS	= mwdoc cmw2
EXAMPLES	= megawave3.config
INCLUDE	= megawave3.shi

.PHONY	: install
install	: $(SCRIPTS) $(WRAPPER).sh $(INCLUDE)
	$(INSTALL) -d $(DESTDIR)/share/megawave3/scripts/
	$(INSTALL) $(INCLUDE) $(DESTDIR)/share/megawave3/scripts
	$(SED) 's,%%DESTDIR%%,$(DESTDIR),' $(WRAPPER).sh > $(WRAPPER)
	$(INSTALL) -d $(DESTDIR)/bin/
	$(INSTALL) $(WRAPPER) $(DESTDIR)/bin
	$(RM) $(WRAPPER)
	$(INSTALL) $(SCRIPTS) $(DESTDIR)/share/megawave3/scripts
	for SCRIPT in $(SCRIPTS) $(ALIAS) ; do \
		$(LN) -s ./$(WRAPPER) $(DESTDIR)/bin/$$SCRIPT; \
	done
	for MODULE in $(MODULES); do \
		$(LN) -s ./$(WRAPPER) $(DESTDIR)/bin/$$MODULE; \
	done
	$(INSTALL) -d $(DESTDIR)/share/doc/megawave3/examples/
	$(INSTALL) $(EXAMPLES) $(DESTDIR)/share/doc/megawave3/examples
