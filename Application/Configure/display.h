/*
 * Display.h
 *
 *  Created on: Jun 12, 2014
 *      Author: jeffw
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "PE_Types.h"
#include "SharpDisplayCS.h"
#include "radioCommon.h"
#ifdef CHE_CONTROLLER
#include "Scanner.h"
#endif

#define DISPLAY_WIDTH		400 // 4.4" = 320, 2.7" = 400
#define DISPLAY_HEIGHT		240
#define DISPLAY_BYTES		(DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8
#define MAX_MSG_BYES		30

#define WHITE				1
#define	BLACK				0

#define LCDCMD_DUMMY		0x00
#define LCDCMD_DISPLAY		0x00
#define LCDCMD_WRITE		0x80
#define LCDCMD_VCOM			0x40
#define LCDCMD_CLEAR		0x20

#define DISPLAY_CS_ON     	SharpDisplayCS_SetVal();
#define DISPLAY_CS_OFF    	Wait_Waitus(50); SharpDisplayCS_ClrVal();

#define ROW_BUFFER_BYTES	DISPLAY_WIDTH / 8

#define FONT_NORMAL			1
#define FONT_MED			2
#define FONT_LARGE			3

#define FONT_ARIAL16			1
#define FONT_ARIAL26			2
#define FONT_3OF9				3
#define FONT_ARIAL16_MONO_BOLD	4
#define FONT_ARIAL20_MONO_BOLD	5
#define FONT_ARIAL24_MONO_BOLD	6
#define FONT_ARIAL26_MONO_BOLD	7

#define readFontByte(addr) (*(const unsigned char *)(addr))
#define swap(a, b) { int16_t t = a; a = b; b = t; }
#define getMax(a,b)    (((a) > (b)) ? (a) : (b))
#define getMin(a,b)    (((a) < (b)) ? (a) : (b))

void sendByte(uint8_t data);
void sendByteLSB(uint8_t data);
void sendDisplayBuffer(void);

void clearDisplay(void);
void drawPixel(int16_t x, int16_t y);
void drawChar(int16_t x, int16_t y, unsigned char c, uint8_t size);
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h);
void drawFastVLine(int16_t x, int16_t y, int16_t h);
void drawFastHLine(int16_t x, int16_t y, int16_t w);
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void drawPixelInRowBuffer(int16_t x, byte *rowBufferPtr);
void putCharSliceInRowBuffer(uint16_t x, unsigned char c, uint8_t *rowBufferPtr, uint8_t slice, uint8_t size);
void putCharByFontSliceInRowBuffer(uint16_t x, unsigned char c, uint8_t *rowBufferPtr, uint8_t slice, uint8_t size, byte fontType);
void putBarcodeInRowBuffer(uint16_t x, unsigned char c, uint8_t *rowBufferPtr, uint8_t slice, uint8_t size);
void sendRowBuffer(uint16_t row, byte *rowBufferPtr);
void displayString(uint16_t x, uint16_t y, char_t *stringPtr, uint8_t size);
void displayStringByFont(uint16_t x, uint16_t y, char_t *stringPtr, uint8_t size, uint8_t fontType);
void displayBarcode(uint16_t x, uint16_t y, char_t *stringPtr, uint8_t size);
void displayMessage(uint8_t line, char_t *stringPtr, uint8_t size);
void setFontType(uint8_t fontType);

void displayCodeshelfLogo(uint8_t x, uint8_t y);

#ifdef CHE_CONTROLLER
void sendLineToScanner(DisplayStringType inString, DisplayStringLenType inLen);
void clearScannerDisplay();
#endif

#endif /* DISPLAY_H_ */
