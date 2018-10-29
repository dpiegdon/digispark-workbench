
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "ws2812.h"
#include "usi_twi.h"

int main(void)
{
	// power off all peripheral. all enabled on demand.
	PRR = (1<<PRTIM1) | (1<<PRTIM0) | (1<<PRUSI) | (1<<PRADC);

	wdt_enable(WDTO_2S);
	usi_twi_init();
	ws2812_init();

	sei();

	while(1) {
		wdt_reset(); // FIXME: move to USI_START_vect
		_delay_ms(500); // FIXME: just go into sleep mode?
	}
}

void usi_twi_rx_complete_callback(const uint8_t * buffer, const uint8_t length)
{
	for(int i = 0; i < length; ++i)
		ws2812_send_single_byte(buffer[i]);
}

