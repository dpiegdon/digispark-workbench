
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <string.h>

#include "ws2812.h"

// generate next value from linear feedback shift register
static inline uint16_t lfsr_fibonacci(uint16_t value)
{
	return (value >> 1) | ((value >> 0) ^ (value >> 2) ^ (value >> 3) ^ (value >> 5)) << 15;
}

// setup ADC for continuous internal temperature measurement
static inline void temp_init(void)
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
static inline uint8_t gammasight(uint8_t in)
{
	uint16_t v = in;
	v = (v*v) >> 8;
	return v;
}

static inline uint8_t interpolate(uint8_t prev, uint8_t next, uint8_t step, uint8_t max_steps)
{
	uint16_t v;

	v = ((uint16_t)prev) * (max_steps+1-step);
	v += ((uint16_t)next) * step;
	v /= max_steps + 1;

	return v;
}

int main(void)
{
	const uint16_t led_count = 3 * 150;
	uint16_t led_index;

	uint8_t tail[led_count];
	uint8_t front_previous[3];
	uint8_t front_next[3];
	const uint8_t front_interpol_max_steps = 31; // (zero => no interpolation)
	uint8_t front_interpol_step = 0;

	uint16_t lfsr = 0xbeef;

	// power off unneeded peripheral
	PRR = (1<<PRTIM1) | (1<<PRTIM0) | (1<<PRUSI);

	// start with lights off
	memset(tail, 0, sizeof(tail));
	memset(front_previous, 0, sizeof(front_previous));
	memset(front_next, 0, sizeof(front_next));

	ws2812_init();
	temp_init();

	while(1) {
		// shift color stream
		for(led_index = led_count-1; led_index >= 3; --led_index)
			tail[led_index] = tail[led_index-3];

		// assign color to free slot
		if(front_interpol_step >= front_interpol_max_steps) {
			front_interpol_step = 0;
			front_previous[0] = front_next[0];
			front_previous[1] = front_next[1];
			front_previous[2] = front_next[2];
			front_next[0] = gammasight(((lfsr >>  0) & 0x1f) << 3);
			front_next[1] = gammasight(((lfsr >>  5) & 0x1f) << 3);
			front_next[2] = gammasight(((lfsr >> 10) & 0x1f) << 3);
		} else {
			front_interpol_step += 1;
		}

		tail[0] = interpolate(front_previous[0], front_next[0], front_interpol_step, front_interpol_max_steps);
		tail[1] = interpolate(front_previous[1], front_next[1], front_interpol_step, front_interpol_max_steps);
		tail[2] = interpolate(front_previous[2], front_next[2], front_interpol_step, front_interpol_max_steps);

		// set lights to their color
		for(led_index=0; led_index < led_count; ++led_index)
			ws2812_send_single_byte(tail[led_index]);

		// wait a bit and seed LFSR
		_delay_ms(1000/(led_count/3));
		lfsr = lfsr_fibonacci(lfsr ^ (ADC << 15));
	}
}

