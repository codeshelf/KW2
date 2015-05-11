/*
 Codeshelf
 © Copyright 2005, 2014 Codeshelf, Inc.
 All rights reserved

 $Id$
 $Name$
 */
#include "aisleControllerTask.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "remoteMgmtTask.h"
#include "string.h"
#include "LedDrive.h"
#include "SPI_PDD.h"

#define getMax(a,b)    (((a) > (b)) ? (a) : (b))
#define getMin(a,b)    (((a) < (b)) ? (a) : (b))
xTaskHandle gAisleControllerTask = NULL;

LedCycle gLedCycle = eLedCycleOff;

extern LedPositionType gTotalLedPositions;

extern LedDataStruct gLedFlashData[];
extern LedPositionType gCurLedFlashDataElement;
extern LedPositionType gTotalLedFlashDataElements;
extern LedPositionType gNextFlashLedPosition;

extern LedDataStruct gLedSolidData[];
extern LedPositionType gCurLedSolidDataElement;
extern LedPositionType gTotalLedSolidDataElements;
extern LedPositionType gNextSolidLedPosition;

// --------------------------------------------------------------------------
void ledDriveInterrupt(void) {

	// The Tx FIFO queue is below the watermark, so provide more output data if we haven't transmitted all the bits yet.

	if (LedDrive_DEVICE->RSER && SPI_RSER_TFFF_RE_MASK) {
		if (gLedCycle == eLedCycleOff) {
			while ((SPI_PDD_GetTxFIFOCounter(LedDrive_DEVICE) <= 1) && (gNextSolidLedPosition < gTotalLedPositions)) {
				uint32 data = getNextSolidData();

				uint32 sample3 = data & 0xff;
				uint32 sample2 = ((data >> 8) & 0xff);
				uint32 sample1 = ((data >> 16) & 0xff);

				SPI_PDD_WriteData8Bits(LedDrive_DEVICE, sample1);
				SPI_PDD_WriteData8Bits(LedDrive_DEVICE, sample2);
				SPI_PDD_WriteData8Bits(LedDrive_DEVICE, sample3);
			}
			if ((gNextSolidLedPosition >= gTotalLedPositions)) {
				LedDrive_DEVICE->RSER &= ~SPI_RSER_TFFF_RE_MASK;
				//LedDrive_DEVICE->PUSHR |= SPI_PUSHR_EOQ_MASK;
			}
		} else {
			while ((SPI_PDD_GetTxFIFOCounter(LedDrive_DEVICE) <= 1) && (gNextFlashLedPosition < gTotalLedPositions)) {
				uint32 data = getNextFlashData();

				uint32 sample3 = data & 0xff;
				uint32 sample2 = ((data >> 8) & 0xff);
				uint32 sample1 = ((data >> 16) & 0xff);

				LedDrive_DEVICE->PUSHR = sample1;
				LedDrive_DEVICE->PUSHR = sample2;
				LedDrive_DEVICE->PUSHR = sample3;
			}
			if ((gNextFlashLedPosition >= gTotalLedPositions)) {
				LedDrive_DEVICE->RSER &= ~SPI_RSER_TFFF_RE_MASK;
				//LedDrive_DEVICE->PUSHR |= SPI_PUSHR_EOQ_MASK;
			}
		}
	}
}

// --------------------------------------------------------------------------
void aisleControllerTask(void *pvParameters) {

	/*
	 * This routine lights the LED strips by serial clock driving the strip's WS2801 ICs.
	 *
	 * Each LED position has a WS2801 that clocks in 8 bits for each color: RGB.  You clock 24 bits onto the serial
	 * bus to send the data to the first IC.  As you clock in 24 more bits the first IC's data shifts
	 * out and into the next IC down the bus.  You do this until you've clocked in all of the data for all
	 * of the LEDs on the bus.  When there is no clock for 500us then ICs all latch up their current values
	 * and light the LED with it.  It ends up looking like a giant shift register.
	 *
	 * There are two user-visible LED cycles: on and off.  This allows for LED flashing: 500ms off, 250ms on.
	 *
	 * There are some LEDs that don't flash because they are solid.  For instance, work path separators. Solid
	 * LEDs are always lit in both the on and off cycles.  If an LED position has both a solid and flashing
	 * request then we show the solid value in the off cycle and the flash value in the on cycle.
	 *
	 * The LED data is 99% 0-bits clocked onto the bus.  It's not practical to hold the thousands of bytes/bits
	 * in the MCU's limited memory.  Instead we keep an array of non-zero LEDs in position order.  The host sends
	 * us this LED data in bus-position order.  E.g. 3, 4, 5, (blue) 70, 71, 72 (blue).
	 *
	 * We also may need to flash more than one color at a position.  (We may even need to flash several colors at
	 * a position.)  To deal with these multiple flash sequences the host sends the LED data in bus-position order
	 * and loop back to an lower position with a new sequence.  For each color we need to flash at the same position
	 * there will be a separate sequence.  E.g. 3, 4, 5 (blue) 70, 71, 72 (blue) then 3, 4, 5 (green) 70, 71, 72 (blue).
	 * That's two sequences: blue/blue, green/blue.
	 *
	 * Whenever the host sends us an LED position of -1 it means to wipe out all of the current sequences
	 * and rebuild them from the new data that will arrive.
	 *
	 * To accomplish serial bus Tx in the MC13224v we use the SSI module in gated-clock mode.  We start the Tx process by
	 * filling the Tx FIFO buffer with 8 bytes.  When the buffer goes below the low-water mark an interrupt
	 * fires, and that ISR fills the Tx FIFO buffer with more data.  We continue to fill that buffer until we've
	 * clocked out all of the data for the bus.  If there is no more data to clock out to the LED serial bus then
	 * we don't put any more data in the FIFO during the ISR. (We also won't see the interrupt again until
	 * we refill the buffer in the next cycle.)
	 *
	 */

	gwUINT8 ccrHolder;

	// Create some fake test data.
	LedDataStruct ledData;
	ledData.channel = 1;
	ledData.red = 0x3e;
	ledData.green = 0x0;
	ledData.blue = 0x0;

	int index = 0;
	for (int errorSet = 0; errorSet < TOTAL_ERROR_SETS; ++errorSet) {
		// Light the first LED in this set.
		ledData.position = errorSet * DISTANCE_BETWEEN_ERROR_LEDS;
		gLedFlashData[index++] = ledData;

		// Light the last LED in this group.
		ledData.position = errorSet * DISTANCE_BETWEEN_ERROR_LEDS + (DISTANCE_BETWEEN_ERROR_LEDS - 1);
		gLedFlashData[index++] = ledData;
	}

	gLedCycle = eLedCycleOff;
	gTotalLedPositions = TOTAL_ERROR_SETS * DISTANCE_BETWEEN_ERROR_LEDS;
	gTotalLedFlashDataElements = index;
	gTotalLedSolidDataElements = 0;

	for (;;) {
		LedDrive_DEVICE->RSER &= ~SPI_RSER_TFFF_RE_MASK;
		LedDrive_DEVICE->MCR |= SPI_MCR_CLR_TXF_MASK;
		//LedDrive_DEVICE->PUSHR &= ~SPI_PUSHR_EOQ_MASK;

		if (gLedCycle == eLedCycleOff) {
			// Write 8 words into the FIFO - that will cause the FIFO low watermark ISR to execute.
			gCurLedSolidDataElement = 0;
			gNextSolidLedPosition = 0;
			GW_ENTER_CRITICAL(ccrHolder);
			while ((SPI_PDD_GetTxFIFOCounter(LedDrive_DEVICE) <= 1) && (gNextSolidLedPosition <= gTotalLedPositions)) {
				uint32 data = getNextSolidData();

				uint32 sample3 = data & 0xff;
				uint32 sample2 = ((data >> 8) & 0xff);
				uint32 sample1 = ((data >> 16) & 0xff);

				SPI_PDD_WriteData8Bits(LedDrive_DEVICE, sample1);
				SPI_PDD_WriteData8Bits(LedDrive_DEVICE, sample2);
				SPI_PDD_WriteData8Bits(LedDrive_DEVICE, sample3);
			}
			GW_EXIT_CRITICAL(ccrHolder);
			LedDrive_DEVICE->RSER |= SPI_RSER_TFFF_RE_MASK;

			// A small chunk of the time will be used to service the ISR to complete the data transmission.
			vTaskDelay(LED_OFF_TIME);
			gLedCycle = eLedCycleOn;
		} else {
			// Write 8 words into the FIFO - that will cause the FIFO low watermark ISR to execute.
			//gCurLedFlashDataElement = 0;
			gNextFlashLedPosition = 0;
			
			if (gCurLedFlashDataElement >= gTotalLedFlashDataElements) {
				gCurLedFlashDataElement = 0;
			}
			
			GW_ENTER_CRITICAL(ccrHolder);
			while ((SPI_PDD_GetTxFIFOCounter(LedDrive_DEVICE) <= 1) && (gNextFlashLedPosition <= gTotalLedPositions)) {
				uint32 data = getNextFlashData();

				uint32 sample3 = data & 0xff;
				uint32 sample2 = ((data >> 8) & 0xff);
				uint32 sample1 = ((data >> 16) & 0xff);

				LedDrive_DEVICE->PUSHR = sample1;
				LedDrive_DEVICE->PUSHR = sample2;
				LedDrive_DEVICE->PUSHR = sample3;
			}
			GW_EXIT_CRITICAL(ccrHolder);
			LedDrive_DEVICE->RSER |= SPI_RSER_TFFF_RE_MASK;

			// A small chunk of the time will be used to service the ISR to complete the data transmission.
			vTaskDelay(LED_ON_TIME);
			gLedCycle = eLedCycleOff;
		}
	}
}

// --------------------------------------------------------------------------
gwUINT32 getNextSolidData() {
	// The default is to return zero for a blank LED.
	ULedSampleType result;
	result.word = 0x0;

	// Check if there are any more solid LED values to display.
	if (gCurLedSolidDataElement < gTotalLedSolidDataElements) {
		// Check if the current LED data value matches the position and channel we want to light.
		if ((gLedSolidData[gCurLedSolidDataElement].position == gNextSolidLedPosition)
				&& (gLedSolidData[gCurLedSolidDataElement].channel == 1)) {
			result.bytes.byte1 = gLedSolidData[gCurLedSolidDataElement].red;
			result.bytes.byte2 = gLedSolidData[gCurLedSolidDataElement].green;
			result.bytes.byte3 = gLedSolidData[gCurLedSolidDataElement].blue;
			gCurLedSolidDataElement += 1;
		}
	}

	gNextSolidLedPosition += 1;
	return result.word;
}

// --------------------------------------------------------------------------
gwUINT32 getNextFlashData() {
	// The default is to return zero for a blank LED.
	ULedSampleType result;
	result.word = 0x0;

	// Check if there are any more flash LED values to display.
	if (gCurLedFlashDataElement < gTotalLedFlashDataElements) {

		if (gLedFlashData[gCurLedFlashDataElement].position < gNextFlashLedPosition) {
			// The next flash data element position is lower than the current LED then we've reached the end of a sequence.
			// We return zeros until we start back at the beginning of the LED strip in the next sequence.
		} else if ((gLedFlashData[gCurLedFlashDataElement].position == gNextFlashLedPosition)
				&& (gLedFlashData[gCurLedFlashDataElement].channel == 1)) {
			// The current LED data value matches the position and channel we want to light.
			result.bytes.byte1 = gLedFlashData[gCurLedFlashDataElement].red;
			result.bytes.byte2 = gLedFlashData[gCurLedFlashDataElement].green;
			result.bytes.byte3 = gLedFlashData[gCurLedFlashDataElement].blue;
			gCurLedFlashDataElement += 1;
		}
	}

	gNextFlashLedPosition += 1;
	return result.word;
}

