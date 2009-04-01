#!/usr/bin/make -f
#
# top-level makefile for megawave
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008-2009)

default	: mwp libmw3 libmw3-x11 libmw3-cmdline libmw3-modules modules
.PHONY	: prebuild mwp \
	libmw3 libmw3-x11 libmw3-cmdline libmw3-modules \
	modules test doc srcdoc install

prebuild	:
	$(MAKE) -C ./mwp prebuild
	$(MAKE) -C ./libmw3-x11 prebuild
	$(MAKE) -C ./libmw3 prebuild
	$(MAKE) -C ./libmw3-cmdline prebuild
	$(MAKE) -C ./libmw3-modules prebuild

mwp	:
	$(MAKE) -C ./mwp

libmw3-x11	:
	$(MAKE) -C ./libmw3-x11

libmw3	: libmw3-x11
	$(MAKE) -C ./libmw3

libmw3-cmdline	: libmw3
	$(MAKE) -C ./libmw3-cmdline

libmw3-modules	: libmw3-x11 libmw3 libmw3-cmdline
	$(MAKE) -C ./libmw3-modules

modules	: libmw3-modules libmw3-cmdline mwp
	$(MAKE) -C ./libmw3-modules modules

test	: modules
	$(MAKE) -C ./libmw3-modules test

install	: mwp libmw3-x11 libmw3 libmw3-cmdline libmw3-modules modules
	$(MAKE) -C ./mwp install
	$(MAKE) -C ./libmw3-x11 install
	$(MAKE) -C ./libmw3 install
	$(MAKE) -C ./libmw3-cmdline install
	$(MAKE) -C ./libmw3-modules install
	$(MAKE) -C ./data install

doc	:
	$(MAKE) -C ./doc

install-doc	: doc
	$(MAKE) -C ./doc install

srcdoc	:
	$(MAKE) -C ./mwp srcdoc
	$(MAKE) -C ./libmw3-x11 srcdoc
	$(MAKE) -C ./libmw3 srcdoc
	$(MAKE) -C ./libmw3-cmdline srcdoc
	$(MAKE) -C ./libmw3-modules srcdoc

clean	:
	$(MAKE) -C ./mwp clean
	$(MAKE) -C ./libmw3-x11 clean
	$(MAKE) -C ./libmw3 clean
	$(MAKE) -C ./libmw3-cmdline clean
	$(MAKE) -C ./libmw3-modules clean
	$(MAKE) -C ./doc clean

distclean	:
	$(MAKE) -C ./mwp distclean
	$(MAKE) -C ./libmw3-x11 distclean
	$(MAKE) -C ./libmw3 distclean
	$(MAKE) -C ./libmw3-cmdline distclean
	$(MAKE) -C ./libmw3-modules distclean
	$(MAKE) -C ./doc distclean
	rm -rf ./build
