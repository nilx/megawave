default	: all
all	: mwp libmw libmw-wdevice modules
.PHONY	: prebuild mwp libmw libmw-wdevice modules doc

prebuild	:
	$(MAKE) -C ./libmw-wdevice prebuild
	$(MAKE) -C ./libmw prebuild
	$(MAKE) -C ./mwp prebuild
	$(MAKE) -C ./modules prebuild

libmw-wdevice	:
	$(MAKE) -C ./libmw-wdevice

libmw	: libmw-wdevice
	$(MAKE) -C ./libmw

mwp	:
	$(MAKE) -C ./mwp

modules	: mwp libmw libmw-wdevice
	$(MAKE) -C ./modules

test	: modules
	$(MAKE) -C ./modules test

doc	:
	$(MAKE) -C ./doc

srcdoc	:
	$(MAKE) -C ./libmw-wdevice srcdoc
	$(MAKE) -C ./libmw srcdoc
	$(MAKE) -C ./mwp srcdoc
	$(MAKE) -C ./modules srcdoc

clean	:
	$(MAKE) -C ./mwp clean
	$(MAKE) -C ./libmw-wdevice clean
	$(MAKE) -C ./libmw clean
	$(MAKE) -C ./modules clean
	$(MAKE) -C ./doc clean

distclean	:
	$(MAKE) -C ./mwp distclean
	$(MAKE) -C ./libmw-wdevice distclean
	$(MAKE) -C ./libmw distclean
	$(MAKE) -C ./modules distclean
	$(MAKE) -C ./doc distclean
