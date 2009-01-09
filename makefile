default	: all
all	: mwplight libmw libmw-wdevice modules
.PHONY	: prebuild mwplight libmw libmw-wdevice modules

prebuild	:
	$(MAKE) -C ./mwplight prebuild
	$(MAKE) -C ./modules prebuild

mwplight	:
	$(MAKE) -C ./mwplight

libmw	: libmw-wdevice
	$(MAKE) -C ./libmw

libmw-wdevice	:
	$(MAKE) -C ./libmw-wdevice

modules	: mwplight libmw libmw-wdevice
	$(MAKE) -C ./modules

test	:
	$(MAKE) -C ./modules test

clean	:
	$(MAKE) -C ./mwplight clean
	$(MAKE) -C ./libmw clean
	$(MAKE) -C ./libmw-wdevice clean
	$(MAKE) -C ./modules clean

distclean	:
	$(MAKE) -C ./mwplight distclean
	$(MAKE) -C ./libmw distclean
	$(MAKE) -C ./libmw-wdevice distclean
	$(MAKE) -C ./modules distclean
