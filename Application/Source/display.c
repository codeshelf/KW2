#include "Display.h"
#include "PE_Types.h"
#include "IO_Map.h"
#include "string.h"
#include "SharpDisplay.h"
#include "SPI_PDD.h"

void clearDisplay() {
	// Send the clear screen command rather than doing a HW refresh (quicker)
	SPI_CS_ON
	sendByte(LCDCMD_CLEAR);
	SPI_CS_OFF
}

void sendByte(uint8_t data) {
		
	// Wait until TX buffer empty.
	while (SPI_PDD_GetTxFIFOCounter(SharpDisplay_DEVICE) > 1)
		;
	SPI_PDD_WriteData8Bits(SharpDisplay_DEVICE, data);
	Wait_Waitus(5);

}

void sendByteLSB(uint8_t data) {

	SPI_PDD_SetDataShiftOrder(SharpDisplay_DEVICE, 0, SPI_PDD_LSB_FIRST);
	
	while (SPI_PDD_GetTxFIFOCounter(SharpDisplay_DEVICE) > 1)
		;
	SPI_PDD_WriteData8Bits(SharpDisplay_DEVICE, data);
	Wait_Waitus(5);
	
	SPI_PDD_SetDataShiftOrder(SharpDisplay_DEVICE, 0, SPI_PDD_MSB_FIRST);
}

void sendDisplayBuffer(uint8_t pattern) {
	uint16_t row;
	uint16_t col;

	SPI_CS_ON

	// Send the write command
	sendByte(LCDCMD_WRITE);

	// Send image buffer
	for (row = 1; row <= DISPLAY_HEIGHT; row++) {
		sendByteLSB(row);
		for (col = 1; col <= DISPLAY_WIDTH; col += 8) {
			sendByteLSB(pattern);
		}
		sendByteLSB(LCDCMD_DUMMY);
	}

	// Send another trailing 8 bits for the last line
	sendByteLSB(LCDCMD_DUMMY);

	SPI_CS_OFF

	SPI_CS_ON
	
	sendByte(LCDCMD_DISPLAY);
	sendByteLSB(LCDCMD_DUMMY);

	SPI_CS_OFF
}
