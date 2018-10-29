
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>


#include "ws2812.h"

/*
 * USI TWI is partially based on mfc_ro7021.c by Axel Gartner,
 * https://www.mikrocontroller.net/attachment/12871/mfc_ro7021.c
 */

#define PIN_SCK PB2
#define PIN_SDA PB0

#ifdef DEBUG
# define PIN_DBG1 PB3
# define PIN_DBG2 PB4
#endif

#define TWI_ADDRESS 0x3c

enum TwiStates {
	TWI_IDLE,
	TWI_PRE_ADDR,
	TWI_ADDR,

	TWI_MISO_ADDR_ACK,
	TWI_MISO_BYTE,
	TWI_MISO_BYTE_ACK,
	TWI_MISO_STOP,

	TWI_MOSI_ADDR_ACK,
	TWI_MOSI_BYTE,
	TWI_MOSI_BYTE_ACK,
	TWI_MOSI_STOP,
};

static volatile uint8_t twi_state = TWI_IDLE;

static inline void sda_in(void)
{
	DDRB &= ~(1<<PIN_SDA);
}

static inline void sda_out_low(void)
{
	DDRB |= (1<<PIN_SDA);
}

static inline void usi_twi_set_usisr(uint8_t counter)
{
	USISR = (1<<USISIF)
	      | (1<<USIOIF)				// clear all interrupt flags
	      | (1<<USIPF)				// and stop-condition detection.
	      | counter;
}

void usi_twi_init(void)
{
	PRR &= ~(1<<PRUSI);				// power on device

	PORTB &= ~((1<<PIN_SDA) | (1<<PIN_SCK));	// make sure we don't pull on the bus
							// until setup is done
	USIDR = 0xff;

	USICR = (1<<USIWM1)				// enable TWI mode
	      | (1<<USICS1);				// using external pos edge clock.

	usi_twi_set_usisr(0);

	PORTB = (PORTB & ~(1<<PIN_SDA)) | (1<<PIN_SCK); // SCK: out, high
	DDRB  = (DDRB  & ~(1<<PIN_SDA)) | (1<<PIN_SCK);	// SDA: in, low
#ifdef DEBUG
	PORTB &= ~((1<<PIN_DBG1) | (1<<PIN_DBG2));
	DDRB  |= (1<<PIN_DBG1) | (1<<PIN_DBG2);
#endif

	USICR |= (1<<USISIE);				// enable start condition interrupt
}

void usi_twi_deinit(void)
{
#ifdef DEBUG
	DDRB  &= ~((1<<PIN_DBG1) | (1<<PIN_DBG2));
	PORTB &= ~((1<<PIN_DBG1) | (1<<PIN_DBG2));
#endif

	PORTB &= ~((1<<PIN_SDA) | (1<<PIN_SCK));
	DDRB  &= ~((1<<PIN_SDA) | (1<<PIN_SCK));

	USICR = 0;
	USISR = 0;

	PRR |= (1<<PRUSI);
}

uint8_t twi_rx_buffer[5*8*3];
uint8_t buffer_index = 0;

ISR(USI_START_vect)
{
#ifdef DEBUG
	PORTB |= (1<<PIN_DBG1);
#endif

	/* TODO: optionally handle repeated start condition?
	if(TWI_IDLE != twi_state)
	*/

	sda_in();
	twi_state = TWI_PRE_ADDR;
	usi_twi_set_usisr(15);
	USICR |= (1<<USIOIE);				// enable counter overflow interrupt.

#ifdef DEBUG
	PORTB &= ~(1<<PIN_DBG1);
#endif
}

ISR(USI_OVF_vect)
{
	uint8_t data = USIDR;

#ifdef DEBUG
	PORTB |= (1<<PIN_DBG2);
#endif

	switch(twi_state) {
		default: // fall through!
		case TWI_IDLE: // bug?
			USICR &= ~(1<<USIOIE);
			USISR |= (1<<USIOIF);
			break;

		case TWI_PRE_ADDR:
			// catch clock edge of start condition
			twi_state = TWI_ADDR;
			usi_twi_set_usisr(0);
			USIDR = 0;
			break;

		case TWI_ADDR:
			if((data >> 1) == TWI_ADDRESS) {
				sda_out_low();
				twi_state = (data&1) ? TWI_MISO_ADDR_ACK : TWI_MOSI_ADDR_ACK;
				usi_twi_set_usisr(14);
				buffer_index = 0;
			} else {
				// we are not addressed, go back to idle:
				twi_state = TWI_IDLE;
				USICR &= ~(1<<USIOIE);
				USISR |= (1<<USIOIF);
			}
			break;

		// TODO: implement MISO side.

		case TWI_MOSI_ADDR_ACK:
			twi_state = TWI_MOSI_BYTE;
			usi_twi_set_usisr(0);
			sda_in();
			break;

		case TWI_MOSI_BYTE:
			sda_out_low();
			twi_state = TWI_MOSI_BYTE_ACK;
			usi_twi_set_usisr(14);
			twi_rx_buffer[buffer_index] = data;
			++buffer_index;
			break;

		case TWI_MOSI_BYTE_ACK:
			sda_in();
			if(buffer_index < sizeof(twi_rx_buffer)) {
				usi_twi_set_usisr(0);
				twi_state = TWI_MOSI_BYTE;
			} else {
#ifdef DEBUG
				PORTB |= (1<<PIN_DBG1);
				PORTB &= ~(1<<PIN_DBG1);
#endif
				USICR &= ~(1<<USIOIE);
				USISR |= (1<<USIOIF);
				twi_state = TWI_IDLE;

				cli();
				for(uint8_t * p = twi_rx_buffer; p < twi_rx_buffer+sizeof(twi_rx_buffer); ++p)
					ws2812_send_single_byte(*p);
				sei();

			}
#ifdef DEBUG
			PORTB |= (1<<PIN_DBG1);
			PORTB &= ~(1<<PIN_DBG1);
#endif
			break;

	}


#ifdef DEBUG
	PORTB &= ~(1<<PIN_DBG2);
#endif
}


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

