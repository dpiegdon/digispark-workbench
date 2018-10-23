
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "usbdrv.h"
#include "hid_keyboard_symbols.h"

// USB HID report descriptor for boot protocol keyboard
const PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
	0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
	0x09, 0x06,  // USAGE (Keyboard)
	0xA1, 0x01,  // COLLECTION (Application)
	0x05, 0x07,  //   USAGE_PAGE (Keyboard)(Key Codes)
	0x19, 0xE0,  //   USAGE_MINIMUM (Keyboard LeftControl)(224)
	0x29, 0xE7,  //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
	0x15, 0x00,  //   LOGICAL_MINIMUM (0)
	0x25, 0x01,  //   LOGICAL_MAXIMUM (1)
	0x75, 0x01,  //   REPORT_SIZE (1)
	0x95, 0x08,  //   REPORT_COUNT (8)
	0x81, 0x02,  //   INPUT (Data,Var,Abs) ; Modifier byte
	0x95, 0x01,  //   REPORT_COUNT (1)
	0x75, 0x08,  //   REPORT_SIZE (8)
	0x81, 0x03,  //   INPUT (Cnst,Var,Abs) ; Reserved byte
	0x95, 0x05,  //   REPORT_COUNT (5)
	0x75, 0x01,  //   REPORT_SIZE (1)
	0x05, 0x08,  //   USAGE_PAGE (LEDs)
	0x19, 0x01,  //   USAGE_MINIMUM (Num Lock)
	0x29, 0x05,  //   USAGE_MAXIMUM (Kana)
	0x91, 0x02,  //   OUTPUT (Data,Var,Abs) ; LED report
	0x95, 0x01,  //   REPORT_COUNT (1)
	0x75, 0x03,  //   REPORT_SIZE (3)
	0x91, 0x03,  //   OUTPUT (Cnst,Var,Abs) ; LED report padding
	0x95, 0x06,  //   REPORT_COUNT (6)
	0x75, 0x08,  //   REPORT_SIZE (8)
	0x15, 0x00,  //   LOGICAL_MINIMUM (0)
	0x25, 0x65,  //   LOGICAL_MAXIMUM (101)
	0x05, 0x07,  //   USAGE_PAGE (Keyboard)(Key Codes)
	0x19, 0x00,  //   USAGE_MINIMUM (Reserved (no event indicated))(0)
	0x29, 0x65,  //   USAGE_MAXIMUM (Keyboard Application)(101)
	0x81, 0x00,  //   INPUT (Data,Ary,Abs)
	0xC0         // END_COLLECTION
};

typedef struct{
	uchar modifier;
	uchar reserved;
	uchar key1;
	uchar key2;
	uchar key3;
	uchar key4;
	uchar key5;
	uchar key6;
} report_t;

static report_t reportBuffer;

uint8_t idle_rate = 500 / 4;  // see HID1_11.pdf sect 7.2.4
uint8_t protocol_version = 0; // see HID1_11.pdf sect 7.2.6

// Digispark Internal LED
#define PIN_LED     PB1

void led_init(void)
{
	// Init LED pin as output
	DDRB |= (1 << PIN_LED);
}

void led_set(char on)
{
	if(on)
		PORTB |= (1 << PIN_LED);
	else
		PORTB &= ~(1 << PIN_LED);
}

char led_numlock;
char led_capslock;

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	// see HID1_11.pdf sect 7.2 and http://vusb.wikidot.com/driver-api
	usbRequest_t *rq = (void *)data;

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS)
		return 0; // ignore request if it's not a class specific request

	// see HID1_11.pdf sect 7.2
	switch (rq->bRequest) {
		case USBRQ_HID_GET_IDLE:
			usbMsgPtr = &idle_rate; // send data starting from this byte
			return 1; // send 1 byte
		case USBRQ_HID_SET_IDLE:
			idle_rate = rq->wValue.bytes[1]; // read in idle rate
			return 0; // send nothing
		case USBRQ_HID_GET_PROTOCOL:
			usbMsgPtr = &protocol_version; // send data starting from this byte
			return 1; // send 1 byte
		case USBRQ_HID_SET_PROTOCOL:
			protocol_version = rq->wValue.bytes[1];
			return 0; // send nothing
		case USBRQ_HID_GET_REPORT:
			usbMsgPtr = (void*)&reportBuffer; // send the report data
			return 8;
		case USBRQ_HID_SET_REPORT:
			if (rq->wLength.word == 1) // check data is available
			{
				// 1 byte, we don't check report type (it can only be output or feature)
				// we never implemented "feature" reports so it can't be feature
				// so assume "output" reports
				// this means set LED status
				// since it's the only one in the descriptor
				return USB_NO_MSG; // send nothing but call usbFunctionWrite
			}
			else // no data or do not understand data, ignore
			{
				return 0; // send nothing
			}
		default: // do not understand data, ignore
			return 0; // send nothing
	}
}

// see http://vusb.wikidot.com/driver-api
usbMsgLen_t usbFunctionWrite(uint8_t * data, uchar len)
{
	led_numlock = data[0] & (1 << (LED_NUM_LOCK-1));
	led_capslock = data[0] & (1 << (LED_CAPS_LOCK-1));

	led_set(led_numlock);

	return 1; // 1 byte read
}

void char_to_keyboard_buffer(char c)
{
	if(c >= 'a' && c <= 'z') {
		reportBuffer.key1 = c - 'a' + KEY_A; return;
	} else if(c >= '1' && c <= '9') {
		reportBuffer.key1 = c - '1' + KEY_1; return;
	} else switch(c) {
		case '0': reportBuffer.key1 = KEY_0; return;
		case ' ': reportBuffer.key1 = KEY_SPACE; return;
		case '\n': reportBuffer.key1 = KEY_ENTER; return;
		case '`': reportBuffer.key1 = KEY_GRAVEACCENT; return;
		case '-': reportBuffer.key1 = KEY_MINUS; return;
		case '=': reportBuffer.key1 = KEY_EQUAL; return;
		case '[': reportBuffer.key1 = KEY_BRACKET_LEFT; return;
		case ']': reportBuffer.key1 = KEY_BRACKET_RIGHT; return;
		case ';': reportBuffer.key1 = KEY_SEMICOLON; return;
		case '\'': reportBuffer.key1 = KEY_SINGLEQUOTE; return;
		case '\\': reportBuffer.key1 = KEY_BACKSLASH; return;
		case ',': reportBuffer.key1 = KEY_COMMA; return;
		case '.': reportBuffer.key1 = KEY_PERIOD; return;
		case '/': reportBuffer.key1 = KEY_SLASH; return;
	}

	reportBuffer.modifier =	MOD_SHIFT_LEFT;

	if(c >= 'A' && c <= 'Z') {
		reportBuffer.key1 = c - 'A' + KEY_A; return;
	} else switch(c) {
		case '~': reportBuffer.key1 = KEY_GRAVEACCENT; return;
		case '!': reportBuffer.key1 = KEY_1; return;
		case '@': reportBuffer.key1 = KEY_2; return;
		case '#': reportBuffer.key1 = KEY_3; return;
		case '$': reportBuffer.key1 = KEY_4; return;
		case '%': reportBuffer.key1 = KEY_5; return;
		case '^': reportBuffer.key1 = KEY_6; return;
		case '&': reportBuffer.key1 = KEY_7; return;
		case '*': reportBuffer.key1 = KEY_8; return;
		case '(': reportBuffer.key1 = KEY_9; return;
		case ')': reportBuffer.key1 = KEY_0; return;
		case '_': reportBuffer.key1 = KEY_MINUS; return;
		case '+': reportBuffer.key1 = KEY_EQUAL; return;
		case '{': reportBuffer.key1 = KEY_BRACKET_LEFT; return;
		case '}': reportBuffer.key1 = KEY_BRACKET_RIGHT; return;
		case ':': reportBuffer.key1 = KEY_SEMICOLON; return;
		case '"': reportBuffer.key1 = KEY_SINGLEQUOTE; return;
		case '|': reportBuffer.key1 = KEY_BACKSLASH; return;
		case '<': reportBuffer.key1 = KEY_COMMA; return;
		case '>': reportBuffer.key1 = KEY_PERIOD; return;
		case '?': reportBuffer.key1 = KEY_SLASH; return;
	};
}

static const char * text_to_write =
			"echo Example Keyboard. 1234.\n";

static int keypress_current = 0;
static char keypress_done = 0;
static int keyboard_delay = 0;

void keyboard_set(void)
{
	if(keyboard_delay) {
		keyboard_delay -= 1;
	} else {
		if(!keypress_done) {
			char_to_keyboard_buffer(text_to_write[keypress_current]);
			keypress_done = 1;
		} else {
			reportBuffer.modifier = 0;
			reportBuffer.key1 = 0;
			keypress_current += 1;
			if(0 == text_to_write[keypress_current])
				keypress_current = 0;
			keypress_done = 0;
		}
		keyboard_delay = 3;
	};
}

int main(void)
{
	uchar   i;

	wdt_enable(WDTO_1S);

	led_init();

	usbInit();
	usbDeviceDisconnect();
	i = 0;
	while(--i){
		wdt_reset();
		_delay_ms(1);
	}
	usbDeviceConnect();

	sei();

	while(1) {
		wdt_reset();
		usbPoll();
		if(usbInterruptIsReady()){
			/* called after every poll of the interrupt endpoint */
			usbSetInterrupt((void *)&reportBuffer, sizeof(reportBuffer));
			/* prepare new keyboard output */
			if(led_capslock)
				keyboard_set();
		}
	}
}

