#!/bin/bash

modprobe binfmt-misc
mount binfmt_misc -t binfmt_misc /proc/sys/fs/binfmt_misc

echo ":64Xpi:M::64Xpi::/root/64Xpi/src/64xpi:" > /proc/sys/fs/binfmt_misc/register