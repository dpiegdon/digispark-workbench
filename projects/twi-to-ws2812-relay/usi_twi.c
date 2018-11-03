/*
 * USI TWI implementation.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "usi_twi.h"

#ifdef USR_TWI_NAKED_ISR
# define USI_TWI_ISR_OPTS ISR_NAKED
# define USI_TWI_ISR_RET(...) __asm__ __volatile__("reti")
#else
# define USI_TWI_ISR_OPTS
# define USI_TWI_ISR_RET(...)
#endif

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

static inline void timer_init(void)
{
	TCCR0A = (1<<WGM01);
	GTCCR &= ~((1<<TSM) | (1<<PSR0));
}

static inline void timer_enable(int enable_interrupt)
{
#ifdef USI_TWI_DEBUG
	PORTB |= (1<<USI_TWI_PIN_DBG2);
#endif
	TCNT0 = 0;
	if(enable_interrupt)
		TIMSK = (TIMSK & ~((1<<OCIE0B) | (1<<TOIE0))) | (1<<OCIE0A);
	else
		TIMSK &= ~(1<<OCIE0A);
	TCCR0B = (1<<CS01) | (1<<CS00);
}

static inline void timer_disable(void)
{
	TCCR0B = 0;
	TIMSK &= ~(1<<OCIE0A);
#ifdef USI_TWI_DEBUG
	PORTB &= ~(1<<USI_TWI_PIN_DBG2);
#endif
}

static inline void timer_set_timeout(void)
{
	OCR0A = TCNT0 + 1;
}

static inline void sda_in(void)
{
	DDRB &= ~(1<<USI_TWI_PIN_SDA);
	timer_disable();
}

static inline void sda_out_low(void)
{
	DDRB |= (1<<USI_TWI_PIN_SDA);
	timer_enable(1);
}

static inline void usi_twi_set_usisr(uint8_t counter)
{
	USISR = (1<<USISIF)
	      | (1<<USIOIF)				// clear all interrupt flags
	      | (1<<USIPF)				// and stop-condition detection.
	      | counter;
}

ISR(TIMER0_COMPA_vect, USI_TWI_ISR_OPTS)
{
#ifdef USI_TWI_DEBUG
	PORTB &= ~(1<<USI_TWI_PIN_DBG2);
	PORTB |= (1<<USI_TWI_PIN_DBG2);
#endif
	usi_twi_set_usisr(0);
	sda_in();
	USICR &= ~(1<<USIOIE);
	twi_state = TWI_IDLE;

	USI_TWI_ISR_RET();
}

void usi_twi_init(void)
{
	PRR &= ~((1<<PRUSI)|(1<<PRTIM0));		// power on device

	PORTB &= ~((1<<USI_TWI_PIN_SDA) | (1<<USI_TWI_PIN_SCK));
							// make sure we don't pull on the bus
							// until setup is done
	USIDR = 0xff;

	USICR = (1<<USIWM1)				// enable TWI mode
	      | (1<<USICS1);				// using external pos edge clock.

	usi_twi_set_usisr(0);

	timer_init();

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

	PRR |= (1<<PRUSI)|(1<<PRTIM0);
}

ISR(USI_START_vect, USI_TWI_ISR_OPTS)
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
	USISR |= (1<<USISIF);				// make interrupt as handled

#ifdef USI_TWI_DEBUG
	PORTB &= ~(1<<USI_TWI_PIN_DBG1);
#endif

	USI_TWI_ISR_RET();
}

ISR(USI_OVF_vect, USI_TWI_ISR_OPTS)
{
	uint8_t data = USIDR;

#ifdef USI_TWI_DEBUG
	PORTB |= (1<<USI_TWI_PIN_DBG1);
#endif

	switch(twi_state) {
		default: // fall through!
		case TWI_IDLE: // bug?
			USICR &= ~(1<<USIOIE);
			USISR |= (1<<USIOIF);
			break;

		case TWI_PRE_ADDR:
			// catch clock edge of start condition
			usi_twi_set_usisr(0);
			timer_enable(0);
			twi_state = TWI_ADDR;
			USIDR = 0;
			// lets measure byte length so we can adjust timeout
			break;

		case TWI_ADDR:
			if((data >> 1) == USI_TWI_ADDRESS) {
				timer_set_timeout();
				usi_twi_set_usisr(14);
				sda_out_low();
				twi_state = (data&1) ? TWI_MISO_ADDR_ACK : TWI_MOSI_ADDR_ACK;
				twi_rx_buffer_index = 0;
			} else {
				// we are not addressed, go back to idle:
				timer_disable();
				twi_state = TWI_IDLE;
				USICR &= ~(1<<USIOIE);
				USISR |= (1<<USIOIF);
			}
			break;

		// well we don't really care about these right now:
		// case TWI_MISO_ADDR_ACK:
		//	TODO
		// case TWI_MISO_BYTE:
		//	TODO
		// case TWI_MISO_BYTE_ACK:
		//	TODO
		// case TWI_MISO_STOP:
		//	TODO

		case TWI_MOSI_ADDR_ACK:
			usi_twi_set_usisr(0);
			sda_in();
			twi_state = TWI_MOSI_BYTE;
			break;

		case TWI_MOSI_BYTE:
			usi_twi_set_usisr(14);
			sda_out_low();
			twi_state = TWI_MOSI_BYTE_ACK;
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
			usi_twi_set_usisr(0);
			sda_in();
			// first transmitted byte is expected to be the transmit length
			if((0 == twi_rx_buffer_index) || (twi_rx_buffer_index < twi_rx_buffer[0]+1)) {
				twi_state = TWI_MOSI_BYTE;
			} else {
				usi_twi_rx_complete_callback(twi_rx_buffer+1, twi_rx_buffer[0]);
				USICR &= ~(1<<USIOIE);
				USISR |= (1<<USIOIF);
				twi_state = TWI_IDLE;
			}
			break;

	}

#ifdef USI_TWI_DEBUG
	PORTB &= ~(1<<USI_TWI_PIN_DBG1);
#endif

	USI_TWI_ISR_RET();
}

