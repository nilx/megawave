default	: all
all	: mwplight libmw libmw-wdevice modules
.PHONY	: prebuild mwplight libmw libmw-wdevice modules

prebuild	:
	$(MAKE) $(MAKEFLAGS) -C ./libmw-wdevice prebuild
	$(MAKE) $(MAKEFLAGS) -C ./libmw prebuild
	$(MAKE) $(MAKEFLAGS) -C ./mwplight prebuild
	$(MAKE) $(MAKEFLAGS) -C ./modules prebuild

libmw-wdevice	:
	$(MAKE) $(MAKEFLAGS) -C ./libmw-wdevice prebuild
	$(MAKE) $(MAKEFLAGS) -C ./libmw-wdevice

libmw	: libmw-wdevice
	$(MAKE) $(MAKEFLAGS) -C ./libmw prebuild
	$(MAKE) $(MAKEFLAGS) -C ./libmw

mwplight	:
	$(MAKE) $(MAKEFLAGS) -C ./mwplight prebuild
	$(MAKE) $(MAKEFLAGS) -C ./mwplight

modules	: mwplight libmw libmw-wdevice
	$(MAKE) $(MAKEFLAGS) -C ./modules prebuild
	$(MAKE) $(MAKEFLAGS) -C ./modules

test	: modules
	$(MAKE) $(MAKEFLAGS) -C ./modules test

clean	:
	$(MAKE) $(MAKEFLAGS) -C ./mwplight clean
	$(MAKE) $(MAKEFLAGS) -C ./libmw clean
	$(MAKE) $(MAKEFLAGS) -C ./libmw-wdevice clean
	$(MAKE) $(MAKEFLAGS) -C ./modules clean

distclean	:
	$(MAKE) $(MAKEFLAGS) -C ./mwplight distclean
	$(MAKE) $(MAKEFLAGS) -C ./libmw distclean
	$(MAKE) $(MAKEFLAGS) -C ./libmw-wdevice distclean
	$(MAKE) $(MAKEFLAGS) -C ./modules distclean
