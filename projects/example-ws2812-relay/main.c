
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>


#include "ws2812.h"
#include "usi_twi.h"

#define LED_COUNT 40

int main(void)
{
	// power off all peripheral. all enabled on demand.
	PRR = (1<<PRTIM1) | (1<<PRTIM0) | (1<<PRUSI) | (1<<PRADC);

	wdt_enable(WDTO_2S);
	usi_twi_init();
	ws2812_init();

	sei();

	while(1) {
		wdt_reset();
		_delay_ms(250);
	}
}

