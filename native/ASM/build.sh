#!/bin/bash

xa test2.asm || exit 1
#bin2c -i a.o65 -o test2.h -a test2
../binfmt_misc/add_hdr.sh a.o65 test
rm a.o65

