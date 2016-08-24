
#include "usbdrv.h"

#define ABS(x) ((x) > 0 ? (x) : (-x))

// Called by V-USB after device reset
void handle_usb_reset() {
	int frameLength, targetLength = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);
	int bestDeviation = 9999;
	uchar trialCal;
	uchar bestCal = 0xff;
	uchar step;
	uchar region;

	// do a binary search in regions 0-127 and 128-255 to get optimum OSCCAL
	for(region = 0; region <= 1; region++) {
		frameLength = 0;
		trialCal = (region == 0) ? 0 : 128;

		for(step = 64; step > 0; step >>= 1) {
			if(frameLength < targetLength) // true for initial iteration
				trialCal += step; // frequency too low
			else
				trialCal -= step; // frequency too high

			OSCCAL = trialCal;
			frameLength = usbMeasureFrameLength();

			if(ABS(frameLength-targetLength) < bestDeviation) {
				bestCal = trialCal; // new optimum found
				bestDeviation = ABS(frameLength -targetLength);
			}
		}
	}

	OSCCAL = bestCal;
}

