
all: test mandelbrot fb_test pi

test: test.c
	cl65 -t none --config ./64xpi.cfg  -O -o test.bin test.c
	#bin2c -i test.bin -o test.h -a test
	../binfmt_misc/mkXpi test.bin test

mandelbrot: mandelbrot.c
	cl65 -t none --config ./64xpi.cfg  -O -o mandelbrot.bin mandelbrot.c
	../binfmt_misc/mkXpi mandelbrot.bin mandelbrot

fb_test: fb_test.c
	cl65 -t none --config ./64xpi.cfg  -O -o fb_test.bin fb_test.c
	../binfmt_misc/mkXpi fb_test.bin fb_test

pi: pi.c
	cl65 -t none --config ./64xpi.cfg  -O -o pi.bin pi.c
	../binfmt_misc/mkXpi pi.bin pi

clean:
	rm -f test.o test.bin test mandelbrot.o mandelbrot.bin mandelbrot fb_test.o fb_test.bin fb_test pi.bin pi
