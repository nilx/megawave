default	: all
all	: mwp libmw3 libmw3-x11 libmw3-cmdline linmw3-modules
.PHONY	: prebuild mwp libmw3 libmw3-x11 libmw3-cmdline libmw3-modules doc

prebuild	:
	$(MAKE) -C ./mwp prebuild
	$(MAKE) -C ./libmw3-x11 prebuild
	$(MAKE) -C ./libmw3-cmdline prebuild
	$(MAKE) -C ./libmw3 prebuild
	$(MAKE) -C ./libmw3-modules prebuild

mwp	:
	$(MAKE) -C ./mwp

libmw3-x11	:
	$(MAKE) -C ./libmw3-x11

libmw3	: libmw3-x11
	$(MAKE) -C ./libmw3

libmw3-cmdline	: libmw3
	$(MAKE) -C ./libmw3-cmdline

libmw3-modules	: libmw3 libmw3-x11 libmw3-cmdline
	$(MAKE) -C ./libmw3-modules

modules	: libmw3-modules libmw3-cmdline mwp
	$(MAKE) -C ./libmw3-modules modules

test	: modules
	$(MAKE) -C ./libmw3-modules test

doc	:
	$(MAKE) -C ./doc

man	:
	$(MAKE) -C ./mwp man
	$(MAKE) -C ./libmw3-x11 man
	$(MAKE) -C ./libmw3 man
	$(MAKE) -C ./libmw3-cmdline man
	$(MAKE) -C ./libmw3-modules man

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
