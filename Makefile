all: 64xpi tests

64xpi:
	$(MAKE) -C src all

tests:
	$(MAKE) -C native/C all

install:
	$(MAKE) -C src install
	install -m 755 native/C/mkXpi /usr/bin/mkXpi 
	install -m 755 native/C/register_64Xpi.sh /usr/bin/register_64Xpi.sh

clean:
	$(MAKE) -C src clean
	$(MAKE) -C native/C clean

.PHONY:	src native/C