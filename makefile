default	: all
all	: mwplight libmw libmw-wdevice modules
.PHONY	: prebuild mwplight libmw libmw-wdevice modules

OPTIONS	= MODE=$(MODE) LINK=$(LINK) CHECK=$(CHECK)

prebuild	:
	$(MAKE) $(OPTIONS) -C ./libmw-wdevice prebuild
	$(MAKE) $(OPTIONS) -C ./libmw prebuild
	$(MAKE) $(OPTIONS) -C ./mwplight prebuild
	$(MAKE) $(OPTIONS) -C ./modules prebuild

libmw-wdevice	:
	$(MAKE) $(OPTIONS) -C ./libmw-wdevice prebuild
	$(MAKE) $(OPTIONS) -C ./libmw-wdevice

libmw	: libmw-wdevice
	$(MAKE) $(OPTIONS) -C ./libmw prebuild
	$(MAKE) $(OPTIONS) -C ./libmw

mwplight	:
	$(MAKE) $(OPTIONS) -C ./mwplight prebuild
	$(MAKE) $(OPTIONS) -C ./mwplight

modules	: mwplight libmw libmw-wdevice
	$(MAKE) $(OPTIONS) -C ./modules prebuild
	$(MAKE) $(OPTIONS) -C ./modules

test	: modules
	$(MAKE) $(OPTIONS) -C ./modules test

clean	:
	$(MAKE) $(OPTIONS) -C ./mwplight clean
	$(MAKE) $(OPTIONS) -C ./libmw clean
	$(MAKE) $(OPTIONS) -C ./libmw-wdevice clean
	$(MAKE) $(OPTIONS) -C ./modules clean

distclean	:
	$(MAKE) $(OPTIONS) -C ./mwplight distclean
	$(MAKE) $(OPTIONS) -C ./libmw distclean
	$(MAKE) $(OPTIONS) -C ./libmw-wdevice distclean
	$(MAKE) $(OPTIONS) -C ./modules distclean
