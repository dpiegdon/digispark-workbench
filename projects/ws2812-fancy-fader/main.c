
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <string.h>

#include "ws2812.h"

// generate next value from linear feedback shift register
uint16_t lfsr_fibonacci(uint16_t value)
{
	return (value >> 1) | ((value >> 0) ^ (value >> 2) ^ (value >> 3) ^ (value >> 5)) << 15;
}

// setup ADC for continuous internal temperature measurement
void temp_init(void)
{
	ADMUX   = (0 << REFS2)
		| (1 << REFS1)
		| (0 << REFS0)		// internal 1.1V reference
		| (1 << MUX3)
		| (1 << MUX2)
		| (1 << MUX1)
		| (1 << MUX0);		// mux ADC4, temp-sensor

	ADCSRA  = (0 << ADPS2)
		| (0 << ADPS1)
		| (0 << ADPS0)		// prescaler /16
		| (1 << ADATE)		// continuous conversion
		| (1 << ADSC)		// start conversions right now
		| (1 << ADEN);		// enable ADC block
}

// do a tiny bit of gamma correction do color differences get more intensive
uint8_t gammasight(uint8_t in)
{
	uint16_t v = in;
	v = (v*v) >> 8;
	return v;
}

int main(void)
{
	uint8_t i;

	const uint8_t light_count = 90;

	uint8_t brightness[light_count];
	uint16_t lfsr = 0xbeef;

	// power off unneeded peripheral
	PRR = (1<<PRTIM1) | (1<<PRTIM0) | (1<<PRUSI);

	ws2812_init();
	temp_init();

	// start with lights off
	memset(brightness, 0, sizeof(brightness));

	while(1) {
		// shift color stream
		for(i = 0; i < light_count-3; ++i)
			brightness[i] = brightness[i+3];

		// assign new color to free slot
		brightness[light_count - 3] = gammasight(((lfsr >>  0) & 0x1f) << 3);
		brightness[light_count - 2] = gammasight(((lfsr >>  5) & 0x1f) << 3);
		brightness[light_count - 1] = gammasight(((lfsr >> 10) & 0x1f) << 3);

		// set lights to their color
		for(i=0; i < light_count; i += 3)
			ws2812_set_single(brightness[i], brightness[i+1], brightness[i+2]);

		// wait a bit and seed LFSR
		_delay_ms(300/light_count*3);
		lfsr = lfsr_fibonacci(lfsr ^ (ADC << 15));
		_delay_ms(300/light_count*3);
		lfsr = lfsr_fibonacci(lfsr ^ (ADC << 15));
		_delay_ms(300/light_count*3);
		lfsr = lfsr_fibonacci(lfsr ^ (ADC << 15));
	}
}

