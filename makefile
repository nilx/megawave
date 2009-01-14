default	: all
all	: mwplight libmw libmw-wdevice modules
.PHONY	: prebuild mwplight libmw libmw-wdevice modules

prebuild	:
	$(MAKE) -C ./libmw-wdevice prebuild
	$(MAKE) -C ./libmw prebuild
	$(MAKE) -C ./mwplight prebuild
	$(MAKE) -C ./modules prebuild

libmw-wdevice	:
	$(MAKE) -C ./libmw-wdevice

libmw	: libmw-wdevice
	$(MAKE) -C ./libmw

mwplight	:
	$(MAKE) -C ./mwplight

modules	: mwplight libmw libmw-wdevice
	$(MAKE) -C ./modules

test	: modules
	$(MAKE) -C ./modules test

clean	:
	$(MAKE) -C ./mwplight clean
	$(MAKE) -C ./libmw-wdevice clean
	$(MAKE) -C ./libmw clean
	$(MAKE) -C ./modules clean

distclean	:
	$(MAKE) -C ./mwplight distclean
	$(MAKE) -C ./libmw-wdevice distclean
	$(MAKE) -C ./libmw distclean
	$(MAKE) -C ./modules distclean
