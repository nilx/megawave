#! make -f
#
# makefile for megawave manuals
#
# Author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>

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
	$(CP) $(USER_DIR)/*.pdf $(DOCDIR)
	$(MAKE) -C $(SYSTEM_DIR) pdf
	$(CP) $(SYSTEM_DIR)/*.pdf $(DOCDIR)

ps	:
	$(MKDIR) $(DOCDIR)
	$(MAKE) -C $(USER_DIR) ps
	$(CP) $(USER_DIR)/*.ps $(DOCDIR)
	$(MAKE) -C $(SYSTEM_DIR) ps
	$(CP) $(SYSTEM_DIR)/*.ps $(DOCDIR)

html	:
	$(MKDIR) $(DOCDIR)
	$(MAKE) -C $(USER_DIR) html
	$(CP) $(USER_DIR)/*.html $(DOCDIR)
	$(MAKE) -C $(SYSTEM_DIR) html
	$(CP) $(SYSTEM_DIR)/*.html $(DOCDIR)

splithtml	:
	$(MKDIR) $(DOCDIR)
	$(MAKE) -C $(USER_DIR) splithtml
	$(RCP) $(USER_DIR)/*_html $(DOCDIR)
	$(MAKE) -C $(SYSTEM_DIR) splithtml
	$(RCP) $(SYSTEM_DIR)/*_html $(DOCDIR)

txt	:
	$(MKDIR) $(DOCDIR)
	$(MAKE) -C $(USER_DIR) txt
	$(CP) $(USER_DIR)/*.txt $(DOCDIR)
	$(MAKE) -C $(SYSTEM_DIR) txt
	$(CP) $(SYSTEM_DIR)/*.txt $(DOCDIR)

clean	:
	$(MAKE) -C $(USER_DIR) clean
	$(MAKE) -C $(SYSTEM_DIR) clean
distclean	:
	$(MAKE) -C $(USER_DIR) distclean
	$(MAKE) -C $(SYSTEM_DIR) distclean
	$(RRM) $(DOCDIR)/*.pdf $(DOCDIR)/*.ps
	$(RRM) $(DOCDIR)/*.html $(DOCDIR)/*_html $(DOCDIR)/*.txt
