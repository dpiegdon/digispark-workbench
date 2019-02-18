
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "ws2812.h"

static inline uint8_t attenuate(uint8_t value)
{
	uint16_t v = value;
	v = ((v<<2)-v) >> 2;
	return v;
}

int main(void)
{
	uint8_t i;

	const uint8_t light_count = 13;

	int8_t center = 0;
	int8_t direction = 1;

	uint8_t brightness[light_count];

	// power off all peripheral. all enabled on demand.
	PRR = (1<<PRTIM1) | (1<<PRTIM0) | (1<<PRUSI) | (1<<PRADC);

	ws2812_init();

	// start with lights off
	for(i=0; i < light_count; ++i)
		brightness[i] = 0;

	while(1) {
		// attenuate all lights
		for(i=0; i < light_count; ++i)
			brightness[i] = attenuate(brightness[i]);

		// set moving spot
		brightness[center] = 0xff;

		// pick next spot
		center += direction;
		if((center >= light_count) || (center < 0)) {
			direction = -direction;
			center += direction;
		}

		// set lights
		for(i=0; i < light_count; ++i) {
			ws2812_set_single(brightness[i], 0, brightness[i]);
			ws2812_set_single(brightness[i], 0, brightness[i]);
			ws2812_set_single(brightness[i], 0, brightness[i]);
		}

		// wait a bit
		_delay_ms(1400/light_count/2);
	}
}

