
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#include "usi_twi.h"

int main(void)
{
	// power off all peripheral. all enabled on demand.
	PRR = (1<<PRTIM1) | (1<<PRTIM0) | (1<<PRUSI) | (1<<PRADC);

	// disable brown-out
	

	usi_twi_init();

	sei();

	while(1) {
	}
}

void usi_twi_rx_complete_callback(const uint8_t * buffer, const uint8_t length)
{
}

