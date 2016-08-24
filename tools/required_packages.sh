#!/bin/sh
cat << _eof
(using debian package names)
required:
	libusb-dev
	binutils-avr
	gcc-avr
	avr-libc

helpful:
	avra
	avrdude
	openocd
	gdb-avr
	simulavr
_eof
