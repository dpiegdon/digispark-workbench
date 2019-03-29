
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

	// clock source is internal 64MHz PLL, divided by 4 => 16MHz input.
	// (CKSEL=1, CLKPR=0)
	// Divide down to 8MHz (/2), so we can safely run at 3.3MHz.
	CLKPR = (1<<CLKPCE);
	CLKPR = (1<<CLKPS0);

	usi_twi_init();
	ws2812_init();

	ws2812_send_single_byte(0xff);
	ws2812_send_single_byte(0x00);
	ws2812_send_single_byte(0x00);

	ws2812_send_single_byte(0x00);
	ws2812_send_single_byte(0xff);
	ws2812_send_single_byte(0x00);

	ws2812_send_single_byte(0x00);
	ws2812_send_single_byte(0x00);
	ws2812_send_single_byte(0xff);

	_delay_ms(250);

	ws2812_send_single_byte(0x00);
	ws2812_send_single_byte(0x00);
	ws2812_send_single_byte(0x00);

	ws2812_send_single_byte(0x00);
	ws2812_send_single_byte(0x00);
	ws2812_send_single_byte(0x00);

	ws2812_send_single_byte(0x00);
	ws2812_send_single_byte(0x00);
	ws2812_send_single_byte(0x00);

	_delay_ms(50);

	sei();

	while(1) {
	}
}

void usi_twi_rx_complete_callback(const uint8_t * buffer, const uint8_t length)
{
	for(int i = 0; i < length; ++i)
		ws2812_send_single_byte(buffer[i]);
}

