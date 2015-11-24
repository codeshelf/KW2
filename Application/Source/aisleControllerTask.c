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
#include "ledControllerLookupTable.h"
#include "PORT_PDD.h"

#define getMax(a,b)    (((a) > (b)) ? (a) : (b))
#define getMin(a,b)    (((a) < (b)) ? (a) : (b))

#define GPIO_S_OUT_OUTPUT	GPIOC_PDDR |= (uint32_t)GPIO_PDDR_PDD(0x40);
#define GPIO_S_OUT_INPUT	GPIOC_PDDR &= (uint32_t)~(uint32_t)GPIO_PDDR_PDD(0x40);

#define ENABLE_SOUT			PORTC_PCR6 = PORT_PCR_MUX(2);
#define DISABLE_SOUT		PORTC_PCR6 = PORT_PCR_MUX(1);

//#define ENABLE_SOUT		LedDrive_DEVICE->MCR &= ~SPI_MCR_MDIS_MASK
//#define DISABLE_SOUT		LedDrive_DEVICE->MCR |= SPI_MCR_MDIS_MASK

//#define ENABLE_SOUT		LedDrive_DEVICE->MCR &= ~SPI_MCR_HALT_MASK
//#define DISABLE_SOUT		LedDrive_DEVICE->MCR |= SPI_MCR_HALT_MASK

//#define ENABLE_FILL_INT		LedDrive_DEVICE->RSER &= ~SPI_RSER_TFFF_RE_MASK;
//#define DISABLE_FILL_INT	LedDrive_DEVICE->RSER |= SPI_RSER_TFFF_RE_MASK;

//#define ENABLE_FILL_INT	LedDrive_DEVICE->RSER |= SPI_RSER_TCF_RE_MASK; LedDrive_DEVICE->SR |= SPI_SR_TCF_MASK;
//#define DISABLE_FILL_INT		LedDrive_DEVICE->RSER &= ~SPI_RSER_TCF_RE_MASK; LedDrive_DEVICE->SR |= SPI_SR_TCF_MASK;

#define ENABLE_FILL_INT		LedDrive_DEVICE->RSER |= SPI_RSER_EOQF_RE_MASK;
#define DISABLE_FILL_INT	LedDrive_DEVICE->RSER &= ~SPI_RSER_EOQF_RE_MASK;

#define SET_EOQF			LedDrive_DEVICE->SR |= SPI_SR_EOQF_MASK
#define CLEAR_EOQF			LedDrive_DEVICE->SR |= SPI_SR_EOQF_MASK  // Clear it by setting it to 1.

#define ModemIRQ_PIN_INDEX 0x03U       /*!< Index of the used pin from the port */
#define ModemIRQ_PIN_MASK 0x08U        /*!< Mask of the used pin from the port */

#define PORTB_INT_ENABLE	PORT_PDD_SetPinInterruptConfiguration(PORTB_BASE_PTR, ModemIRQ_PIN_INDEX, PORT_PDD_INTERRUPT_ON_ZERO);
#define PORTB_INT_DISABLE	PORT_PDD_SetPinInterruptConfiguration(PORTB_BASE_PTR, ModemIRQ_PIN_INDEX, PORT_PDD_INTERRUPT_DMA_DISABLED);

xTaskHandle gAisleControllerTask = NULL;

LedCycle gLedCycle = eLedCycleOff;

extern LedPositionType gTotalLedPositions;

extern LedDataStruct gLedFlashData[];
extern LedPositionType gCurLedFlashDataElement;
extern LedPositionType gTotalLedFlashDataElements;
extern LedPositionType gNextFlashLedPosition;
extern LedData	gCurLedFlashBitPos;

extern LedDataStruct gLedSolidData[];
extern LedPositionType gCurLedSolidDataElement;
extern LedPositionType gTotalLedSolidDataElements;
extern LedPositionType gNextSolidLedPosition;
extern LedData	gCurLedSolidBitPos;

// --------------------------------------------------------------------------
void ledDriveInterrupt(void) {

	// The Tx FIFO queue is below the watermark, so provide more output data if we haven't transmitted all the bits yet.

	// On the last byte ready to send turn off the SPI to prevent spurious "fake" clock edges for WS2811.
//	if (SPI_PDD_GetTxFIFOCounter(LedDrive_DEVICE) == 1) {
		DISABLE_SOUT;
		CLEAR_EOQF;
		ENABLE_FILL_INT;
//	}
	
//	if (LedDrive_DEVICE->RSER && SPI_RSER_TFFF_RE_MASK) {
		if (gLedCycle == eLedCycleOff) {
			/* while ((SPI_PDD_GetTxFIFOCounter(LedDrive_DEVICE) <= 1) &&*/ if ( (gNextSolidLedPosition < gTotalLedPositions)) {
				uint32 data = getNextSolidData();
				data = 0x00888888;

				uint32 sample3 = data & 0xff | SPI_PUSHR_EOQ_MASK;
				uint32 sample2 = ((data >> 8) & 0xff);
				uint32 sample1 = ((data >> 16) & 0xff);

				LedDrive_DEVICE->PUSHR = sample1;
				LedDrive_DEVICE->PUSHR = sample2;
				LedDrive_DEVICE->PUSHR = sample3;
				ENABLE_SOUT;
			}
			if ((gNextSolidLedPosition >= gTotalLedPositions)) {
				//DISABLE_FILL_INT;
				DISABLE_SOUT;
			}
		} else {
			/* while ((SPI_PDD_GetTxFIFOCounter(LedDrive_DEVICE) <= 1) && */ if( (gNextFlashLedPosition < gTotalLedPositions)) {
				uint32 data = getNextFlashData();
				//data = 0x00eeeeee;

				uint32 sample3 = data & 0xff | SPI_PUSHR_EOQ_MASK;
				uint32 sample2 = ((data >> 8) & 0xff);
				uint32 sample1 = ((data >> 16) & 0xff);

				LedDrive_DEVICE->PUSHR = sample1;
				LedDrive_DEVICE->PUSHR = sample2;
				LedDrive_DEVICE->PUSHR = sample3;
				ENABLE_SOUT;
			}
			if ((gNextFlashLedPosition >= gTotalLedPositions)) {
				//DISABLE_FILL_INT;
				DISABLE_SOUT;
			}
		}
//	}
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
	ledData.sample1 = 0;
	ledData.sample2 = 0;
	ledData.sample3 = 0;
	ledData.sample4 = 0;
	
	GPIO_S_OUT_OUTPUT;

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
	gCurLedFlashBitPos = 0;

	for (;;) {
		DISABLE_FILL_INT;
		LedDrive_DEVICE->MCR |= SPI_MCR_CLR_TXF_MASK;
		//LedDrive_DEVICE->PUSHR &= ~SPI_PUSHR_EOQ_MASK;

		if (gLedCycle == eLedCycleOff) {
			// Write 8 words into the FIFO - that will cause the FIFO low watermark ISR to execute.
			gCurLedSolidDataElement = 0;
			gNextSolidLedPosition = 0;
			GW_ENTER_CRITICAL(ccrHolder);
			/*while ((SPI_PDD_GetTxFIFOCounter(LedDrive_DEVICE) <= 1) &&*/ if ((gNextSolidLedPosition <= (gTotalLedPositions))) {
				uint32 data = getNextSolidData();
				data = 0x00888888;

				uint32 sample3 = data & 0xff | SPI_PUSHR_EOQ_MASK;
				uint32 sample2 = ((data >> 8) & 0xff);
				uint32 sample1 = ((data >> 16) & 0xff);

				//ENABLE_SOUT;
				LedDrive_DEVICE->PUSHR = sample1;
				LedDrive_DEVICE->PUSHR = sample2;
				LedDrive_DEVICE->PUSHR = sample3;
				ENABLE_SOUT;
			}
			GW_EXIT_CRITICAL(ccrHolder);
			PORTB_INT_DISABLE
			vPortStopTickTimer();
			ENABLE_FILL_INT;

			// A small chunk of the time will be used to service the ISR to complete the data transmission.
			while (gNextSolidLedPosition < (gTotalLedPositions)) {
				//Wait_Waitms(1);
			}
			PORTB_INT_ENABLE
			vPortStartTickTimer();
			vTaskDelay(LED_OFF_TIME);
			gLedCycle = eLedCycleOn;
		} else {
			// Write 8 words into the FIFO - that will cause the FIFO low watermark ISR to execute.
			//gCurLedFlashDataElement = 0;
			gNextFlashLedPosition = 0;
			
			if (gCurLedFlashDataElement >= gTotalLedFlashDataElements) {
				gCurLedFlashDataElement = 0;
				gCurLedFlashBitPos = 0;
			}
			
			GW_ENTER_CRITICAL(ccrHolder);
			/*while ((SPI_PDD_GetTxFIFOCounter(LedDrive_DEVICE) <= 1) && */ if((gNextFlashLedPosition <= gTotalLedPositions)) {
				uint32 data = getNextFlashData();
				//data = 0x00eeeeee;

				uint32 sample3 = data & 0xff | SPI_PUSHR_EOQ_MASK;
				uint32 sample2 = ((data >> 8) & 0xff);
				uint32 sample1 = ((data >> 16) & 0xff);

				//ENABLE_SOUT;
				LedDrive_DEVICE->PUSHR = sample1;
				LedDrive_DEVICE->PUSHR = sample2;
				LedDrive_DEVICE->PUSHR = sample3;
				ENABLE_SOUT;
			}
			GW_EXIT_CRITICAL(ccrHolder);
			PORTB_INT_DISABLE
			vPortStopTickTimer();
			ENABLE_FILL_INT;

			// A small chunk of the time will be used to service the ISR to complete the data transmission.
			while (gNextFlashLedPosition < (gTotalLedPositions)) {
				//Wait_Waitms(1);
			}
			PORTB_INT_ENABLE
			vPortStartTickTimer();
			vTaskDelay(LED_ON_TIME);
			gLedCycle = eLedCycleOff;
		}
	}
}

// --------------------------------------------------------------------------
//gwUINT32 getNextSolidData() {
//	// The default is to return zero for a blank LED.
//	ULedSampleType result;
//	result.word = 0x0;
//
//	// Check if there are any more solid LED values to display.
//	if (gCurLedSolidDataElement < gTotalLedSolidDataElements) {
//		// Check if the current LED data value matches the position and channel we want to light.
//		if ((gLedSolidData[gCurLedSolidDataElement].position == gNextSolidLedPosition)
//				&& (gLedSolidData[gCurLedSolidDataElement].channel == 1)) {
//			result.bytes.byte1 = gLedSolidData[gCurLedSolidDataElement].red;
//			result.bytes.byte2 = gLedSolidData[gCurLedSolidDataElement].green;
//			result.bytes.byte3 = gLedSolidData[gCurLedSolidDataElement].blue;
//			gCurLedSolidDataElement += 1;
//		}
//	}
//
//	gNextSolidLedPosition += 1;
//	return result.word;
//}

// --------------------------------------------------------------------------
//gwUINT32 getNextFlashData() {
//	// The default is to return zero for a blank LED.
//	ULedSampleType result;
//	result.word = 0x0;
//
//	// Check if there are any more flash LED values to display.
//	if (gCurLedFlashDataElement < gTotalLedFlashDataElements) {
//
//		if (gLedFlashData[gCurLedFlashDataElement].position < gNextFlashLedPosition) {
//			// The next flash data element position is lower than the current LED then we've reached the end of a sequence.
//			// We return zeros until we start back at the beginning of the LED strip in the next sequence.
//		} else if ((gLedFlashData[gCurLedFlashDataElement].position == gNextFlashLedPosition)
//				&& (gLedFlashData[gCurLedFlashDataElement].channel == 1)) {
//			// The current LED data value matches the position and channel we want to light.
//			result.bytes.byte1 = gLedFlashData[gCurLedFlashDataElement].red;
//			result.bytes.byte2 = gLedFlashData[gCurLedFlashDataElement].green;
//			result.bytes.byte3 = gLedFlashData[gCurLedFlashDataElement].blue;
//			gCurLedFlashDataElement += 1;
//		}
//	}
//
//	gNextFlashLedPosition += 1;
//	return result.word;
//}
//

// --------------------------------------------------------------------------
gwUINT32 getNextSolidData() {
	// The default is to return zero for a blank LED.
	ULedSampleType result;
	gwUINT8 tempSample;
	result.word = 0x00888888;
	
	// Check if there are any more solid LED values to display.
	if (gCurLedSolidDataElement < gTotalLedSolidDataElements) {
		if (gLedSolidData[gCurLedSolidDataElement].position < gNextSolidLedPosition) {
			// The next solid data element position is lower than the current LED then we've reached the end of a sequence.
			// We return zeros until we start back at the beginning of the LED strip in the next sequence.
		} else if ((gLedSolidData[gCurLedSolidDataElement].position == gNextSolidLedPosition)
		        && (gLedSolidData[gCurLedSolidDataElement].channel == 1)) {
			if (gLedSolidData[gCurLedSolidDataElement].sample4 == 0) {
				// The current LED data value matches the position and channel we want to light.
				if (gCurLedSolidBitPos == 0) {
					// R7->R2
					gLedSolidData[gCurLedSolidDataElement].sample1 = kWS2811Lookup[(gLedSolidData[gCurLedSolidDataElement].red >> 2)];
				} else if (gCurLedSolidBitPos == 6) {
					// R1->R0, G7->G4
					tempSample = (gLedSolidData[gCurLedSolidDataElement].red << 4) & 0x30;
					tempSample += gLedSolidData[gCurLedSolidDataElement].green >> 4;
					gLedSolidData[gCurLedSolidDataElement].sample2 = kWS2811Lookup[tempSample];
				} else if (gCurLedSolidBitPos == 12) {
					// G3->G0, B7->B6
					tempSample = (gLedSolidData[gCurLedSolidDataElement].green << 2) & 0x30;
					tempSample += gLedSolidData[gCurLedSolidDataElement].blue >> 6;
					gLedSolidData[gCurLedSolidDataElement].sample3 = kWS2811Lookup[tempSample];
				} else if (gCurLedSolidBitPos == 18) {
					// B5->B0
					gLedSolidData[gCurLedSolidDataElement].sample4 = kWS2811Lookup[(gLedSolidData[gCurLedSolidDataElement].blue && 0x3f)];
				}
			}
			if (gCurLedSolidBitPos == 0) {
				result.word = gLedSolidData[gCurLedSolidDataElement].sample1;
			} else if (gCurLedSolidBitPos == 6) {
				result.word = gLedSolidData[gCurLedSolidDataElement].sample2;
			} else if (gCurLedSolidBitPos == 12) {
				result.word = gLedSolidData[gCurLedSolidDataElement].sample3;
			} else if (gCurLedSolidBitPos == 18) {
				result.word = gLedSolidData[gCurLedSolidDataElement].sample4;
				gCurLedSolidDataElement += 1;
			}
		}
	}

	gCurLedSolidBitPos += 6;
	if (gCurLedSolidBitPos == 24) {
		gNextSolidLedPosition += 1;
		gCurLedSolidBitPos = 0;
	}
	
	return result.word;
}

// --------------------------------------------------------------------------
gwUINT32 getNextFlashData() {
	// The default is to return zero for a blank LED.
	ULedSampleType result;
	gwUINT8 tempSample;
	result.word = 0x00888888;
	
	// Check if there are any more flash LED values to display.
	if (gCurLedFlashDataElement < gTotalLedFlashDataElements) {
		if (gLedFlashData[gCurLedFlashDataElement].position < gNextFlashLedPosition) {
			// The next flash data element position is lower than the current LED then we've reached the end of a sequence.
			// We return zeros until we start back at the beginning of the LED strip in the next sequence.
		} else if ((gLedFlashData[gCurLedFlashDataElement].position == gNextFlashLedPosition)
		        && (gLedFlashData[gCurLedFlashDataElement].channel == 1)) {
			if (gLedFlashData[gCurLedFlashDataElement].sample4 == 0) {
				// The current LED data value matches the position and channel we want to light.
				if (gCurLedFlashBitPos == 0) {
					// R7->R2
					gLedFlashData[gCurLedFlashDataElement].sample1 = kWS2811Lookup[(gLedFlashData[gCurLedFlashDataElement].red >> 2)];
				} else if (gCurLedFlashBitPos == 6) {
					// R1->R0, G7->G4
					tempSample = (gLedFlashData[gCurLedFlashDataElement].red << 4) & 0x30;
					tempSample += gLedFlashData[gCurLedFlashDataElement].green >> 4;
					gLedFlashData[gCurLedFlashDataElement].sample2 = kWS2811Lookup[tempSample];
				} else if (gCurLedFlashBitPos == 12) {
					// G3->G0, B7->B6
					tempSample = (gLedFlashData[gCurLedFlashDataElement].green << 2) & 0x30;
					tempSample += gLedFlashData[gCurLedFlashDataElement].blue >> 6;
					gLedFlashData[gCurLedFlashDataElement].sample3 = kWS2811Lookup[tempSample];
				} else if (gCurLedFlashBitPos == 18) {
					// B5->B0
					gLedFlashData[gCurLedFlashDataElement].sample4 = kWS2811Lookup[(gLedFlashData[gCurLedFlashDataElement].blue && 0x3f)];
				}
			}
			if (gCurLedFlashBitPos == 0) {
				result.word = gLedFlashData[gCurLedFlashDataElement].sample1;
			} else if (gCurLedFlashBitPos == 6) {
				result.word = gLedFlashData[gCurLedFlashDataElement].sample2;
			} else if (gCurLedFlashBitPos == 12) {
				result.word = gLedFlashData[gCurLedFlashDataElement].sample3;
			} else if (gCurLedFlashBitPos == 18) {
				result.word = gLedFlashData[gCurLedFlashDataElement].sample4;
				gCurLedFlashDataElement += 1;
			}
		}
	}

	gCurLedFlashBitPos += 6;
	if (gCurLedFlashBitPos == 24) {
		gNextFlashLedPosition += 1;
		gCurLedFlashBitPos = 0;
	}
	
	return result.word;
}
