
TARGET := main
OBJECTS := main.o usi_twi.o
include ../generic_makefile

# naked ISR are not supported when figuring out stack usage
STACK_USAGE :=

main.o: ws2812.h usi_twi.h

usi_twi.c: usi_twi.h

