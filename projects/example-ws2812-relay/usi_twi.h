
#ifndef __USI_TWI_H__
# define __USI_TWI_H__

/*
 * USI TWI implementation.
 *
 *  - Only receive side is implemented.
 *  - Max 130 KHz TWI clock for whole bus for 16 MHz system clock.
 *    (Without USI_TWI_DEBUG this may be faster. Maybe 150 KHz?)
 *  - First byte of received data is expected to be length of upcoming data.
 *    (As proper stop-condition recognition is not trivial.)
 *  - Callback for received transfer is only called if that many (or more)
 *    bytes are received.
 *  - Excessive bytes are ignored and not acknowledged.
 *  - Expected callback definition:
 *    void usi_twi_rx_complete_callback(const uint8_t * buffer, const uint8_t length);
 *    Where buffer and length exclude the length-defining first byte.
 *  - Callback will be executed in interrupt context.
 *
 * Loosely inspired by:
 *  - Atmel Application Note 312
 *  - mfc_ro7021.c by Axel Gartner,
 *    https://www.mikrocontroller.net/attachment/12871/mfc_ro7021.c
 *  - usitwislave.c by Erik Slagter
 */

//#define USI_TWI_DEBUG
#ifdef USI_TWI_DEBUG
# define USI_TWI_PIN_DBG1 PB3
# define USI_TWI_PIN_DBG2 PB4
#endif

#define USI_TWI_ADDRESS 0x3c

#define USI_TWI_MAX_TX_LENGTH (3*84)

#define USI_TWI_PIN_SCK PB2
#define USI_TWI_PIN_SDA PB0


void usi_twi_init(void);

void usi_twi_deinit(void);

// this has to be implemented by the user:
void usi_twi_rx_complete_callback(const uint8_t * buffer, const uint8_t length);


#endif // __USI_TWI_H__

