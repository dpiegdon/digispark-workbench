/*
 * USI TWI implementation.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "usi_twi.h"

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


static uint8_t twi_state = TWI_IDLE;
#if USI_TWI_MAX_TX_LENGTH > 253
# error TWI buffer is too large, buffer overflow possible!
#endif
uint8_t twi_rx_buffer[1 + USI_TWI_MAX_TX_LENGTH];
uint8_t twi_rx_buffer_index = 0;



static inline void sda_in(void)
{
	DDRB &= ~(1<<USI_TWI_PIN_SDA);
}

static inline void sda_out_low(void)
{
	DDRB |= (1<<USI_TWI_PIN_SDA);
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

	PORTB &= ~((1<<USI_TWI_PIN_SDA) | (1<<USI_TWI_PIN_SCK));
							// make sure we don't pull on the bus
							// until setup is done
	USIDR = 0xff;

	USICR = (1<<USIWM1)				// enable TWI mode
	      | (1<<USICS1);				// using external pos edge clock.

	usi_twi_set_usisr(0);

	PORTB = (PORTB & ~(1<<USI_TWI_PIN_SDA)) | (1<<USI_TWI_PIN_SCK);
							// SCK: out, high
	DDRB  = (DDRB  & ~(1<<USI_TWI_PIN_SDA)) | (1<<USI_TWI_PIN_SCK);
							// SDA: in, low
#ifdef USI_TWI_DEBUG
	PORTB &= ~((1<<USI_TWI_PIN_DBG1) | (1<<USI_TWI_PIN_DBG2));
	DDRB  |= (1<<USI_TWI_PIN_DBG1) | (1<<USI_TWI_PIN_DBG2);
#endif

	USICR |= (1<<USISIE);				// enable start condition interrupt
}

void usi_twi_deinit(void)
{
#ifdef USI_TWI_DEBUG
	DDRB  &= ~((1<<USI_TWI_PIN_DBG1) | (1<<USI_TWI_PIN_DBG2));
	PORTB &= ~((1<<USI_TWI_PIN_DBG1) | (1<<USI_TWI_PIN_DBG2));
#endif

	PORTB &= ~((1<<USI_TWI_PIN_SDA) | (1<<USI_TWI_PIN_SCK));
	DDRB  &= ~((1<<USI_TWI_PIN_SDA) | (1<<USI_TWI_PIN_SCK));

	USICR = 0;
	USISR = 0;

	PRR |= (1<<PRUSI);
}

ISR(USI_START_vect)
{
#ifdef USI_TWI_DEBUG
	PORTB |= (1<<USI_TWI_PIN_DBG1);
#endif

	/* TODO: optionally handle repeated start condition?
	if(TWI_IDLE != twi_state)
	*/

	sda_in();
	twi_state = TWI_PRE_ADDR;
	usi_twi_set_usisr(15);
	USICR |= (1<<USIOIE);				// enable counter overflow interrupt.

#ifdef USI_TWI_DEBUG
	PORTB &= ~(1<<USI_TWI_PIN_DBG1);
#endif
}

ISR(USI_OVF_vect)
{
	uint8_t data = USIDR;

#ifdef USI_TWI_DEBUG
	PORTB |= (1<<USI_TWI_PIN_DBG2);
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
			if((data >> 1) == USI_TWI_ADDRESS) {
				sda_out_low();
				twi_state = (data&1) ? TWI_MISO_ADDR_ACK : TWI_MOSI_ADDR_ACK;
				usi_twi_set_usisr(14);
				twi_rx_buffer_index = 0;
			} else {
				// we are not addressed, go back to idle:
				twi_state = TWI_IDLE;
				USICR &= ~(1<<USIOIE);
				USISR |= (1<<USIOIF);
			}
			break;

		// TODO: implement MISO side?
		// well we don't really care about this right now.

		case TWI_MOSI_ADDR_ACK:
			twi_state = TWI_MOSI_BYTE;
			usi_twi_set_usisr(0);
			sda_in();
			break;

		case TWI_MOSI_BYTE:
			sda_out_low();
			twi_state = TWI_MOSI_BYTE_ACK;
			usi_twi_set_usisr(14);
			if((0 == twi_rx_buffer_index) && (data > sizeof(twi_rx_buffer)-1)) {
				// first transmitted byte is the length of
				// the upcoming transfer. make sure it
				// fits into the buffer.
				twi_rx_buffer[0] = sizeof(twi_rx_buffer)-1;
			} else {
				twi_rx_buffer[twi_rx_buffer_index] = data;
			}
			++twi_rx_buffer_index;
			break;

		case TWI_MOSI_BYTE_ACK:
			sda_in();
			// first transmitted byte is expected to be the transmit length
			if((0 == twi_rx_buffer_index) || (twi_rx_buffer_index < twi_rx_buffer[0]+1)) {
				usi_twi_set_usisr(0);
				twi_state = TWI_MOSI_BYTE;
			} else {
#ifdef USI_TWI_DEBUG
				PORTB |= (1<<USI_TWI_PIN_DBG1);
				PORTB &= ~(1<<USI_TWI_PIN_DBG1);
#endif
				usi_twi_rx_complete_callback(twi_rx_buffer+1, twi_rx_buffer[0]);
				USICR &= ~(1<<USIOIE);
				USISR |= (1<<USIOIF);
				twi_state = TWI_IDLE;
			}
#ifdef USI_TWI_DEBUG
			PORTB |= (1<<USI_TWI_PIN_DBG1);
			PORTB &= ~(1<<USI_TWI_PIN_DBG1);
#endif
			break;

	}

#ifdef USI_TWI_DEBUG
	PORTB &= ~(1<<USI_TWI_PIN_DBG2);
#endif
}

