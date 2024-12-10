all: 64xpi tests

64xpi:
	$(MAKE) -C src all

tests:
	$(MAKE) -C native/C all

install:
	$(MAKE) -C src install

clean:
	$(MAKE) -C src clean
	$(MAKE) -C native/C clean

.PHONY:	src native/C