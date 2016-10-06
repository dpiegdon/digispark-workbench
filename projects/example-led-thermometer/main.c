#include <avr/io.h>
#include <util/delay.h>
#include "obmq.h"

void systemclock_set_1mhz(void)
{
	CLKPR = (1 << CLKPCE);		// change-enable
	CLKPR   = (0 << CLKPS3)
		| (0 << CLKPS2)
		| (1 << CLKPS1)
		| (1 << CLKPS0);
	_delay_ms(1);
}

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
		| (1 << ADPS0)		// prescaler -- more than /2 seems to fail
		| (1 << ADEN);		// enable ADC block

	_delay_ms(100/8);		// wait for block to settle
}

void temp_trigger(void)
{
	ADCSRA |= (1 << ADSC);		// start single conversion
}

char temp_ready(void)
{
	return (ADCSRA & ADSC) ? 0 : 1;
}

uint16_t temp_get(void)
{
	uint16_t ret = ADC;
	return ret;
}


// Digispark Internal LED
#define PIN_LED     PB1

void led_init(void)
{
	// Init LED pin as output
	DDRB |= (1 << PIN_LED);
}

void led_set(void * ignored, char value)
{
	(void)(ignored);
	if(value)
		PORTB |= (1 << PIN_LED);
	else
		PORTB &= ~(1 << PIN_LED);
}

static OneBitMessageQueue m;


/*
 * this example will repeadetly read the temperature
 * in Kelvin and clock it to the LED.
 */
int main(void)
{
	systemclock_set_1mhz();
	led_init();
	temp_init();
	obmq_init(&m, led_set, 0, 0, 0, 4, 8, 0);

	obmq_queuemessage(&m, 0);

	char wait_for_temp = 0;
	char temp_delay = 0;
	while(1) {
		if(wait_for_temp) {
			if(temp_delay != 0) {
				temp_delay -= 1;
			} else if(temp_ready()) {
					uint16_t temp;
					temp = temp_get();
					(void)(temp);
					obmq_queuemessage(&m, temp >> 8);
					obmq_queuemessage(&m, temp & 0xff);
					wait_for_temp = 0;
			}
		} else if(0 == obmq_messages_queued(&m)) {
			temp_trigger();
			wait_for_temp = 1;
			temp_delay = 100;
		}

		obmq_trigger(&m);
		_delay_ms(50/8);
	}

	return 0;
}
