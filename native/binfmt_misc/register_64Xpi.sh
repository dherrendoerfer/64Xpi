#!/bin/bash

if [ -e /proc/sys/fs/binfmt_misc/64Xpi ]; then
  echo "already registered, exiting."
  exit 1
fi

if [ ! -e /proc/sys/fs/binfmt_misc/status ]; then
  modprobe binfmt-misc
  mount binfmt_misc -t binfmt_misc /proc/sys/fs/binfmt_misc
fi

echo ":64Xpi:M::64Xpi::/usr/bin/64xpi:" > /proc/sys/fs/binfmt_misc/register