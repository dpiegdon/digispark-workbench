
TARGET := main
OBJECTS := main.o usi_twi.o
include ../generic_makefile

CLOCK := 8000000

AVRDUDE_OPTS=-p t85 -c stk500v2 -B 40 -P /dev/ttyACM0

.PHONY: readfuses writefuses writeprogram

# naked ISR are not supported when figuring out stack usage
STACK_USAGE :=

CFLAGS += -DWS2812_8MHZ_CLOCK

main.o: ws2812.h usi_twi.h

usi_twi.c: usi_twi.h

readfuses:
	avrdude ${AVRDUDE_OPTS} -U lfuse:r:fuse_low.hex:h -U hfuse:r:fuse_high.hex:h -U efuse:r:fuse_extended:h

writefuses:
	avrdude ${AVRDUDE_OPTS} -U lfuse:w:0xf1:m -U hfuse:w:0xdd:m -U efuse:w:0xff:m

writeprogram: main.hex
	avrdude ${AVRDUDE_OPTS} -e -U flash:w:main.hex

prog: writeprogram writefuses

