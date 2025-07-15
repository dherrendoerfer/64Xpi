
all: mojozork

mojozork: mojozork.c
	cl65 -t none --config ../64xpi.cfg  -O -o mojozork.bin mojozork.c
	../../binfmt_misc/mkXpi mojozork.bin mojozork

clean:
	rm -f mojozork.o mojozork.bin mojozork