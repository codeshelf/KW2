#include "display.h"
#include "PE_Types.h"
#include "IO_Map.h"
#include "string.h"
#include "stdlib.h"
#include "gwSystemMacros.h"
#include "SharpDisplay.h"
#include "SPI_PDD.h"
#include "fontArial16.h"
#include "fontArial26.h"
#include "fontBarcode.h"
#include "codeshelf.logo.h"
#include "Wait.h"

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

	DISPLAY_CS_ON

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

	DISPLAY_CS_OFF

	DISPLAY_CS_ON

	sendByte(LCDCMD_DISPLAY);
	sendByteLSB(LCDCMD_DUMMY);

	DISPLAY_CS_OFF
}

void clearDisplay() {
	// Send the clear screen command rather than doing a HW refresh (quicker)
	DISPLAY_CS_ON
	sendByte(LCDCMD_CLEAR);
	DISPLAY_CS_OFF

	// Clear the display buffer as well.
	memset(&displayBuffer, 0x00, DISPLAY_BYTES);
}

void sendRowBuffer(uint16_t row, byte* rowBuffer) {
	uint16_t pos;
	char c;

	DISPLAY_CS_ON

	// Send the write command
	sendByte(LCDCMD_WRITE + LCDCMD_VCOM);

	// Send image buffer
	sendByteLSB(row);
	for (pos = 0; pos < ROW_BUFFER_BYTES; pos++) {
		c = rowBuffer[pos];
		sendByteLSB(~c);
	}
	sendByteLSB(LCDCMD_DUMMY);

	// Send another trailing 8 bits for the last line
	sendByteLSB(LCDCMD_DUMMY);

	DISPLAY_CS_OFF

	DISPLAY_CS_ON

	sendByte(LCDCMD_DISPLAY);
	sendByteLSB(LCDCMD_DUMMY);

	DISPLAY_CS_OFF
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

void drawPixelInRowBuffer(int16_t x, byte *rowBufferPtr) {
	uint16_t loc = x / 8;
	uint8_t value = (1 << x % 8);
	if ((loc >= 0) && (loc < ROW_BUFFER_BYTES)) {
		rowBufferPtr[loc] |= value;
	}
}

void putCharSliceInRowBuffer(uint16_t x, unsigned char drawChar, uint8_t *rowBufferPtr, uint8_t slice, uint8_t size) {
	uint8_t hzPixelNum;
	uint8_t pixelsByte;
	uint8_t extra;
	uint8_t sliceByteOffset;
	uint8_t charDescOffset;
	uint16_t pixDataOffset;

	charDescOffset = (uint8_t) (drawChar - arial_26ptFontInfo.startChar);
	pixDataOffset = arial_26ptDescriptors[charDescOffset].offset;

	sliceByteOffset = 0;
	if (slice > 0) {
		sliceByteOffset = slice * (arial_26ptDescriptors[charDescOffset].widthBits / 8);
		if (arial_26ptDescriptors[charDescOffset].widthBits % 8) {
			sliceByteOffset += slice;
		}
	}

	for (hzPixelNum = 0; hzPixelNum < arial_26ptDescriptors[charDescOffset].widthBits; hzPixelNum++) {
		pixelsByte = arial_26ptBitmaps[pixDataOffset + sliceByteOffset + hzPixelNum / 8];
		if (pixelsByte & (0x80 >> (hzPixelNum % 8))) {
			for (extra = 0; extra < size; extra++) {
				drawPixelInRowBuffer(x + hzPixelNum + extra, rowBufferPtr);
			}
		}
	}
}

void displayString(uint16_t x, uint16_t y, char_t *stringPtr, uint8_t size) {
	byte rowBuffer[ROW_BUFFER_BYTES];
	uint8_t slice;
	uint8_t charPos;
	uint8_t extra;
	uint8_t charDescOffset;
	uint16_t totalBits;

	for (slice = 0; slice < arial_26ptFontInfo.charInfo->heightBits; slice++) {
		memset(&rowBuffer, 0x00, ROW_BUFFER_BYTES);
		totalBits = 0;
		for (charPos = 0; charPos < strlen(stringPtr); charPos++) {
			if (stringPtr[charPos] == ' ') {
				totalBits += arial_26ptFontInfo.spacePixels * size;
			} else {
				putCharSliceInRowBuffer(x + totalBits, stringPtr[charPos], rowBuffer, slice, size);
				charDescOffset = stringPtr[charPos] - arial_26ptFontInfo.startChar;
				totalBits += arial_26ptDescriptors[charDescOffset].widthBits + size;
			}
		}
		for (extra = 0; extra < size; extra++) {
			sendRowBuffer(y + (slice * size) + extra, rowBuffer);
		}
	}
}


void displayMessage(uint8_t line, char_t *stringPtr, uint8_t size) {
	if (line == 1) {
		displayString(5, 5 + ((line - 1) * 30 * size), stringPtr, 2);
	} else {
		displayString(5, 5 + (line * 30 * size), stringPtr, size);
	}
}

void putBarcodeSliceInRowBuffer(uint16_t x, unsigned char drawChar, uint8_t *rowBufferPtr, uint8_t slice, uint8_t size) {
	uint8_t hzPixelNum;
	uint8_t pixelsByte;
	uint8_t extra;
	uint8_t sliceByteOffset;
	uint8_t charDescOffset;
	uint16_t pixDataOffset;

	charDescOffset = (uint8_t) (drawChar - barcodeFontInfo.startChar);
	pixDataOffset = barcodeDescriptors[charDescOffset].offset;

	sliceByteOffset = 0;
	if (slice > 0) {
		sliceByteOffset = slice * (barcodeDescriptors[charDescOffset].widthBits / 8);
		if (barcodeDescriptors[charDescOffset].widthBits % 8) {
			sliceByteOffset += slice;
		}
	}

	for (hzPixelNum = 0; hzPixelNum < barcodeDescriptors[charDescOffset].widthBits; hzPixelNum++) {
		pixelsByte = barcodeBitmaps[pixDataOffset + sliceByteOffset + hzPixelNum / 8];
		if (pixelsByte & (0x80 >> (hzPixelNum % 8))) {
			for (extra = 0; extra < size; extra++) {
				drawPixelInRowBuffer(x + hzPixelNum + extra, rowBufferPtr);
			}
		}
	}
}

void displayBarcode(uint16_t x, uint16_t y, char_t *stringPtr, uint8_t size) {
	byte rowBuffer[ROW_BUFFER_BYTES];
	uint8_t slice;
	uint8_t charPos;
	uint8_t extra;
	uint8_t charDescOffset;
	uint16_t totalBits;

	for (slice = 0; slice < barcodeFontInfo.charInfo->heightBits; slice++) {
		memset(&rowBuffer, 0x00, ROW_BUFFER_BYTES);
		totalBits = 0;
		for (charPos = 0; charPos < strlen(stringPtr); charPos++) {
			if (stringPtr[charPos] == ' ') {
				totalBits += barcodeFontInfo.spacePixels + size;
			} else {
				putBarcodeSliceInRowBuffer(x + totalBits, stringPtr[charPos], rowBuffer, slice, size);
				charDescOffset = stringPtr[charPos] - barcodeFontInfo.startChar;
				totalBits += barcodeDescriptors[charDescOffset].widthBits + size;
			}
		}
		for (extra = 0; extra < size; extra++) {
			sendRowBuffer(y + (slice * size) + extra, rowBuffer);
		}
	}
}

void displayCodeshelfLogo(uint8_t x, uint8_t y) {
	byte rowBuffer[ROW_BUFFER_BYTES];
	uint8_t slice;
	uint8_t bytePos;

	for (slice = 0; slice < codeshelf_logobwHeightPixels; slice++) {
		memset(&rowBuffer, 0x00, ROW_BUFFER_BYTES);
		for (bytePos = 0; bytePos < codeshelf_logobwWidthPages; bytePos++) {
			rowBuffer[bytePos + (x / 8)] = codeshelf_logobwBitmaps[(slice * codeshelf_logobwWidthPages) + bytePos];
		}
		sendRowBuffer(slice + y, rowBuffer);
	}
}
