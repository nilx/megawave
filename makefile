default	: all
all	: mwp libmw libmw-x11 modules
.PHONY	: prebuild mwp libmw libmw-x11 modules doc

prebuild	:
	$(MAKE) -C ./libmw-x11 prebuild
	$(MAKE) -C ./libmw prebuild
	$(MAKE) -C ./mwp prebuild
	$(MAKE) -C ./modules prebuild

libmw-x11	:
	$(MAKE) -C ./libmw-x11

libmw	: libmw-x11
	$(MAKE) -C ./libmw

mwp	:
	$(MAKE) -C ./mwp

modules	: mwp libmw libmw-x11
	$(MAKE) -C ./modules

test	: modules
	$(MAKE) -C ./modules test

doc	:
	$(MAKE) -C ./doc

srcdoc	:
	$(MAKE) -C ./libmw-x11 srcdoc
	$(MAKE) -C ./libmw srcdoc
	$(MAKE) -C ./mwp srcdoc
	$(MAKE) -C ./modules srcdoc

clean	:
	$(MAKE) -C ./mwp clean
	$(MAKE) -C ./libmw-x11 clean
	$(MAKE) -C ./libmw clean
	$(MAKE) -C ./modules clean
	$(MAKE) -C ./doc clean

distclean	:
	$(MAKE) -C ./mwp distclean
	$(MAKE) -C ./libmw-x11 distclean
	$(MAKE) -C ./libmw distclean
	$(MAKE) -C ./modules distclean
	$(MAKE) -C ./doc distclean
