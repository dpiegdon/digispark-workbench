
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define LED_COUNT 40

uint8_t ledBuffer[3*LED_COUNT];

int main(void)
{
	wdt_enable(WDTO_1S);

	sei();

	while(1) {
		wdt_reset();
	}
}

