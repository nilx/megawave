default	: all
all	: mwp libmw libmw-x11 libmw-cmdline modules
.PHONY	: prebuild mwp libmw libmw-x11 libmw-cmdline modules doc

prebuild	:
	$(MAKE) -C ./libmw-x11 prebuild
	$(MAKE) -C ./libmw-cmdline prebuild
	$(MAKE) -C ./libmw prebuild
	$(MAKE) -C ./mwp prebuild
	$(MAKE) -C ./modules prebuild

libmw-x11	:
	$(MAKE) -C ./libmw-x11

libmw	: libmw-x11
	$(MAKE) -C ./libmw

libmw-cmdline	: libmw
	$(MAKE) -C ./libmw-cmdline

mwp	:
	$(MAKE) -C ./mwp

modules	: mwp libmw libmw-x11 libmw-cmdline
	$(MAKE) -C ./modules

test	: modules
	$(MAKE) -C ./modules test

doc	:
	$(MAKE) -C ./doc

man	:
	$(MAKE) -C ./libmw-x11 man
	$(MAKE) -C ./libmw man
	$(MAKE) -C ./libmw-cmdline man
	$(MAKE) -C ./mwp man
	$(MAKE) -C ./modules man

srcdoc	:
	$(MAKE) -C ./libmw-x11 srcdoc
	$(MAKE) -C ./libmw srcdoc
	$(MAKE) -C ./libmw-cmdline srcdoc
	$(MAKE) -C ./mwp srcdoc
	$(MAKE) -C ./modules srcdoc

clean	:
	$(MAKE) -C ./mwp clean
	$(MAKE) -C ./libmw-x11 clean
	$(MAKE) -C ./libmw clean
	$(MAKE) -C ./libmw-cmdline clean
	$(MAKE) -C ./modules clean
	$(MAKE) -C ./doc clean

distclean	:
	$(MAKE) -C ./mwp distclean
	$(MAKE) -C ./libmw-x11 distclean
	$(MAKE) -C ./libmw distclean
	$(MAKE) -C ./libmw-cmdline distclean
	$(MAKE) -C ./modules distclean
	$(MAKE) -C ./doc distclean
	rm -rf ./build
