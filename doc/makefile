#! make -f
#
# makefile for megawave manuals
#
# Author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>

USER_DIR	= user
SYSTEM_DIR	= system

# ACTIONS

.PHONY : default all pdf ps html splithtml txt clean distclean
default	: pdf html splithtml txt
all	: pdf ps html splithtml txt

pdf	:
	$(MAKE) -C $(USER_DIR) pdf
	$(MAKE) -C $(SYSTEM_DIR) pdf
ps	:
	$(MAKE) -C $(USER_DIR) ps
	$(MAKE) -C $(SYSTEM_DIR) ps
html	:
	$(MAKE) -C $(USER_DIR) html
	$(MAKE) -C $(SYSTEM_DIR) html
splithtml	:
	$(MAKE) -C $(USER_DIR) splithtml
	$(MAKE) -C $(SYSTEM_DIR) splithtml
txt	:
	$(MAKE) -C $(USER_DIR) txt
	$(MAKE) -C $(SYSTEM_DIR) txt

clean	:
	$(MAKE) -C $(USER_DIR) clean
	$(MAKE) -C $(SYSTEM_DIR) clean
distclean	:
	$(MAKE) -C $(USER_DIR) distclean
	$(MAKE) -C $(SYSTEM_DIR) distclean
