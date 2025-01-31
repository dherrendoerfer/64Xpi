#!/bin/bash

if [ -z "$1" -o -z "$2" ]; then
    echo "usage: $0 <infile> <outfile> <ROMLIST>"
    exit 1
fi

echo "64Xpi" > "$2"
dd status=none if=/dev/zero bs=80 count=1 >> "$2"

dd status=none if="$1" >> "$2" 

chmod 755 "$2"