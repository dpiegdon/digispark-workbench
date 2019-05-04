
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

#define COLOR_PINK
#if defined COLOR_RAL2011
const uint8_t R = 0xFF;
const uint8_t G = 0x61;
const uint8_t B = 0x06;
#elif defined COLOR_PINK
const uint8_t R = 0xFF;
const uint8_t G = 0x00;
const uint8_t B = 0xFF;
#elif defined COLOR_YELLOW
const uint8_t R = 0xFF;
const uint8_t G = 0xD0;
const uint8_t B = 0x00;
#elif defined COLOR_BLUE
const uint8_t R = 0x00;
const uint8_t G = 0x00;
const uint8_t B = 0xFF;
#elif defined COLOR_RED
const uint8_t R = 0xFF;
const uint8_t G = 0x00;
const uint8_t B = 0x00;
#endif

int main(void)
{
	uint8_t i;

	const uint8_t light_count = 13;

	int8_t center = 0;
	int8_t direction = 1;

	uint16_t brightness[light_count];

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
			uint16_t r = R * brightness[i] / 256;
			uint16_t g = G * brightness[i] / 256;
			uint16_t b = B * brightness[i] / 256;
			ws2812_set_single(r, g, b);
			ws2812_set_single(r, g, b);
			ws2812_set_single(r, g, b);
		}

		// wait a bit
		_delay_ms(1400/light_count/2);
	}
}

