default	: all
all	: mwplight libmw libmw-wdevice modules

MAKEOPTS	= CC="ccache gcc"


mwplight	:
	$(MAKE) -C ./mwplight STRICT=1 $(MAKEOPTS)

libmw	: libmw-wdevice
	$(MAKE) -C ./libmw STRICT=1 $(MAKEOPTS)

libmw-wdevice	:
	$(MAKE) -C ./libmw-wdevice STRICT=1 $(MAKEOPTS)

modules	: mwplight libmw libmw-wdevice
	$(MAKE) -C ./modules $(MAKEOPTS)

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
