#include <avr/io.h>
#include <util/delay.h>

// Digispark Internal LED
#define PIN_LED     PB1
#define DELAY_MS    500

int main(void) {
	// Init LED pin as output
	DDRB |= (1 << PIN_LED);
	// Light up LED
	PORTB |= (1 << PIN_LED);

	// Blink !
	for (;;) {
		PORTB ^= (1 << PIN_LED);
		_delay_ms(DELAY_MS);
	}

	return 0;
}
