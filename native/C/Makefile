
all: test mandelbrot fb_test

test: test.c
	cl65 -t none --config ./64xpi.cfg  -O -o test.bin test.c
	#bin2c -i test.bin -o test.h -a test
	../binfmt_misc/add_hdr.sh test.bin test

mandelbrot: mandelbrot.c
	cl65 -t none --config ./64xpi.cfg  -O -o mandelbrot.bin mandelbrot.c
	../binfmt_misc/add_hdr.sh mandelbrot.bin mandelbrot

fb_test: fb_test.c
	cl65 -t none --config ./64xpi.cfg  -O -o fb_test.bin fb_test.c
	../binfmt_misc/add_hdr.sh fb_test.bin fb_test

clean:
	rm -f test.o test.bin test mandelbrot.o mandelbrot.bin mandelbrot fb_test.o fb_test.bin fb_test
