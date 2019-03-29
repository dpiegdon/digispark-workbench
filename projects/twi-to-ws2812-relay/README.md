<!-- vim: fo=a tw=80 colorcolumn=80 syntax=markdown :
-->

ATTiny85-based TWI-to-WS2812 translator
=======================================

WS2812 LED strips have a timing-critical protocol. If you have an application
that is real-time and you can't spare the cycles, this firmware and an ATTiny85
will help you relax the timings.

You can simply push the data stream via TWI to the ATTiny85, e.g. using DMA.
Once completed, the ATTiny85 will then take care to send the buffered data
stream to the LEDs using the correct timings.

The TWI transfer must contain a length field at offset 0, that contains the
lenght of the data coming after the length field. E.g.:

`0x03 0x01 0x02 0x03`

Currently only 84 LEDs are supported, as the length field is only 8 bit long and
each LED required 3 bytes of payload (G/R/B).

Expected system clock for the ATTiny is 16 MHz, so `CKSEL` fuses need to be
programmed to `0001`. If you use a DigiSpark this is already all set for the
bootloader.

Optimal fuses likely are set with avrdude via:

    -U lfuse:w:0xf1:m -U hfuse:w:0xdd:m -U efuse:w:0xff:m

TWI pins have to be `PB2` as `SCK` and `PB0` as `SDA`, so the USI TWI mode can
be used. The LEDs should be connected to `PB1`, but that may be changed in
`ws2812.h` .

The TWI address of the ATTiny can be configured in `usi_twi.h`. It currently
defaults to 0x3c.

TWI clock speed has to be between 50 KHz and 280 KHz.

Authors
-------

David R. Piegdon <dgit@piegdon.de>


License
-------

All files in this repository are released unter the GNU General Public License
Version 3 or later. See the file COPYING for more information.

