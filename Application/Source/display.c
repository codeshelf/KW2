#include "display.h"
#include "PE_Types.h"
#include "IO_Map.h"
#include "string.h"
#include "SharpDisplay.h"
#include "SPI_PDD.h"
#include "lcdfont.h"

__attribute__ ((section(".m_data_20000000"))) byte displayBuffer[DISPLAY_BYTES];

extern const unsigned char lcdfont[];

void sendByte(uint8_t data) {

	// Wait until TX buffer empty.
	while (SPI_PDD_GetTxFIFOCounter(SharpDisplay_DEVICE) > 1)
		;
	SPI_PDD_WriteData8Bits(SharpDisplay_DEVICE, data);
	Wait_Waitns(700);

}

void sendByteLSB(uint8_t data) {

	SPI_PDD_SetDataShiftOrder(SharpDisplay_DEVICE, 0, SPI_PDD_LSB_FIRST);

	while (SPI_PDD_GetTxFIFOCounter(SharpDisplay_DEVICE) > 1)
		;
	SPI_PDD_WriteData8Bits(SharpDisplay_DEVICE, data);
	Wait_Waitns(700);

	SPI_PDD_SetDataShiftOrder(SharpDisplay_DEVICE, 0, SPI_PDD_MSB_FIRST);
}

void sendDisplayBuffer() {
	uint16_t row;
	uint16_t col;
	uint16_t pos;

	SPI_CS_ON

	// Send the write command
	sendByte(LCDCMD_WRITE + LCDCMD_VCOM);

	// Send image buffer
	for (row = 0; row < DISPLAY_HEIGHT; row++) {
		sendByteLSB(row);
		for (col = 0; col < DISPLAY_WIDTH; col += 8) {
			pos = (row * DISPLAY_WIDTH + col) / 8;
			char c = displayBuffer[pos];
			sendByteLSB(~c);
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

void clearDisplay() {
	// Send the clear screen command rather than doing a HW refresh (quicker)
	SPI_CS_ON
	sendByte(LCDCMD_CLEAR);
	SPI_CS_OFF

	// Clear the display buffer as well.
	memset(&displayBuffer, 0x00, DISPLAY_BYTES);
}

void drawPixel(int16_t x, int16_t y) {
	if ((x >= DISPLAY_WIDTH) || (y >= DISPLAY_HEIGHT))
		return;
	uint16_t loc = (y * DISPLAY_WIDTH + x) / 8;
	uint8_t value = (1 << x % 8);
	displayBuffer[loc] |= value;
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h) {
	for (int16_t i = x; i < x + w; i++) {
		drawFastVLine(i, y, h);
	}
}

void drawFastVLine(int16_t x, int16_t y, int16_t h) {
	drawLine(x, y, x, y + h - 1);
}
void drawFastHLine(int16_t x, int16_t y, int16_t w) {
	drawLine(x, y, x + w - 1, y);
}

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}
	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}
	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);
	int16_t err = dx / 2;
	int16_t ystep;
	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}
	for (; x0 <= x1; x0++) {
		if (steep) {
			drawPixel(y0, x0);
		} else {
			drawPixel(x0, y0);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

void drawChar(int16_t x, int16_t y, unsigned char c, uint8_t size) {
	for (int8_t i = 0; i < 6; i++) {
		uint8_t line;
		if (i == 5)
			line = 0x0;
		else
			line = readFontByte(lcdfont+(c*5)+i);
		for (int8_t j = 0; j < 8; j++) {
			if (line & 0x1) {
				if (size == 1) // default size
					drawPixel(x + i, y + j);
				else { // big size
					fillRect(x + (i * size), y + (j * size), size, size);
				}
//			} else {
//				if (size == 1) // default size
//					drawPixel(x + i, y + j);
//				else { // big size
//				       //fillRect(x + i * size, y + j * size, size, size, bg);
//				}
			}
			line >>= 1;
		}
	}
}

void displayString(int16_t x, int16_t y, char_t *stringPtr, uint8_t size) {
	for (uint8_t pos = 0; pos < strlen(stringPtr); pos++) {
		drawChar(x + (pos * size * 6), y, stringPtr[pos], size);
	}
	sendDisplayBuffer();
}

void displayMessage(uint8_t line, char_t *stringPtr, uint8_t maxChars) {
	uint8_t size = 3;
	uint8_t x = 5;
	uint8_t y = 10 + (9 * size * (line - 1));
	uint8_t max = getMin(maxChars, strlen(stringPtr));
	for (uint8_t pos = 0; pos < max; pos++) {
		drawChar(x + (pos * size * 6), y, stringPtr[pos], size);
	}
	sendDisplayBuffer();
}
