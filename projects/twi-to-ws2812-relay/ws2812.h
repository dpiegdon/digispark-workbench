
#ifndef __WS2812_H__
# define __WS2812_H__

# include <stdint.h>
# include <avr/cpufunc.h>
# include <avr/io.h>

# ifndef PIN_WS2812_LED
#  define PIN_WS2812_LED PB1
# endif
# ifndef PORT_WS2812_LED
#  define PORT_WS2812_LED ((&PORTB)-__SFR_OFFSET)
# endif

// Define "WS2812_8MHZ_CLOCK" if your uC is running at 8 MHz system clock.
// Otherwise 16 MHz is assumed.

static inline void ws2812_init(void)
{
	DDRB |= (1 << PIN_WS2812_LED);
}

static inline void ws2812_send_single_byte(uint8_t byte)
{
	for(uint8_t mask = 0x80; mask != 0; mask >>= 1) {
		if(byte & mask) {
			__asm__ __volatile__("sbi %0, %1 \n\t"
					     "nop \n\t"
					     "nop \n\t"
					     "nop \n\t"
					     "nop \n\t"
					     "nop \n\t"
# ifndef WS2812_8MHZ_CLOCK
					     "nop \n\t"
					     "nop \n\t"
					     "nop \n\t"
					     "nop \n\t"
					     "nop \n\t"
					     "nop \n\t"
					     "nop \n\t"
					     "nop \n\t"
# endif // WS28128_8MHZ_CLOCK
					     "cbi %0, %1 \n\t"
					     :
					     : "i" (PORT_WS2812_LED), "i" (PIN_WS2812_LED)
					     :
					);
		} else {
			__asm__ __volatile__("sbi %0, %1 \n\t"
# ifndef WS2812_8MHZ_CLOCK
					     "nop \n\t"
					     "nop \n\t"
# endif // WS28128_8MHZ_CLOCK
					     "nop \n\t"
					     "cbi %0, %1 \n\t"
					     "nop \n\t"
					     "nop \n\t"
					     :
					     : "i" (PORT_WS2812_LED), "i" (PIN_WS2812_LED)
					     :
					);
		}
	}
}

static inline void ws2812_set_single(uint8_t r, uint8_t g, uint8_t b)
{
	ws2812_send_single_byte(g);
	ws2812_send_single_byte(r);
	ws2812_send_single_byte(b);
}

#endif // __WS2812_H__

