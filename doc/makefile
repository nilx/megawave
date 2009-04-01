#! make -f
#
# makefile for megawave manuals
#
# Author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2009)

BASEDIR	= ..
COMMONDIR	= $(BASEDIR)/common
include	$(COMMONDIR)/makefile

USER_DIR	= user
SYSTEM_DIR	= system

# ACTIONS

.PHONY : default all pdf ps html splithtml txt clean distclean
default	: pdf html splithtml txt
all	: pdf ps html splithtml txt

pdf	:
	$(MKDIR) $(DOCDIR)
	$(MAKE) -C $(USER_DIR) pdf
	$(MAKE) -C $(SYSTEM_DIR) pdf

ps	:
	$(MKDIR) $(DOCDIR)
	$(MAKE) -C $(USER_DIR) ps
	$(MAKE) -C $(SYSTEM_DIR) ps

html	:
	$(MKDIR) $(DOCDIR)
	$(MAKE) -C $(USER_DIR) html
	$(MAKE) -C $(SYSTEM_DIR) html

splithtml	:
	$(MKDIR) $(DOCDIR)
	$(MAKE) -C $(USER_DIR) splithtml
	$(MAKE) -C $(SYSTEM_DIR) splithtml

txt	:
	$(MKDIR) $(DOCDIR)
	$(MAKE) -C $(USER_DIR) txt
	$(MAKE) -C $(SYSTEM_DIR) txt

.PHONY	: install
install	: pdf html splithtml txt
	$(INSTALL) -d $(DESTDIR)/share/doc/megawave3
	$(INSTALL) $(USER_DIR)/*.pdf $(DESTDIR)/share/doc/megawave3/
	$(INSTALL) $(USER_DIR)/*.txt $(DESTDIR)/share/doc/megawave3/
	$(INSTALL) $(USER_DIR)/*.html $(DESTDIR)/share/doc/megawave3/
	$(INSTALL) $(SYSTEM_DIR)/*.pdf $(DESTDIR)/share/doc/megawave3/
	$(INSTALL) $(SYSTEM_DIR)/*.txt $(DESTDIR)/share/doc/megawave3/
	$(INSTALL) $(SYSTEM_DIR)/*.html $(DESTDIR)/share/doc/megawave3/
	$(INSTALL) -d $(DESTDIR)/share/doc/megawave3/user_manual_html
	$(INSTALL) $(USER_DIR)/user_manual_html/* \
		$(DESTDIR)/share/doc/megawave3/user_manual_html/
	$(INSTALL) -d $(DESTDIR)/share/doc/megawave3/system_manual_html
	$(INSTALL) $(SYSTEM_DIR)/system_manual_html/* \
		$(DESTDIR)/share/doc/megawave3/system_manual_html/

clean	:
	$(MAKE) -C $(USER_DIR) clean
	$(MAKE) -C $(SYSTEM_DIR) clean
distclean	:
	$(MAKE) -C $(USER_DIR) distclean
	$(MAKE) -C $(SYSTEM_DIR) distclean
