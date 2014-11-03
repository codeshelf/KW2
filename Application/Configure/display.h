/*
 * Display.h
 *
 *  Created on: Jun 12, 2014
 *      Author: jeffw
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "PE_Types.h"
#include "SharpDisplay_CS.h"
#include "Wait.h"

#define DISPLAY_WIDTH		400
#define DISPLAY_HEIGHT		240
#define DISPLAY_BYTES		(DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8

#define WHITE				1
#define	BLACK				0

#define LCDCMD_DUMMY		0x00
#define LCDCMD_DISPLAY		0x00
#define LCDCMD_WRITE		0x80
#define LCDCMD_VCOM			0x40
#define LCDCMD_CLEAR		0x20

#define SPI_CS_ON     		SharpDisplay_CS_SetVal();
#define SPI_CS_OFF    		Wait_Waitus(5); SharpDisplay_CS_ClrVal();

#define readFontByte(addr) (*(const unsigned char *)(addr))
#define swap(a, b) { int16_t t = a; a = b; b = t; }
#define getMax(a,b)    (((a) > (b)) ? (a) : (b))
#define getMin(a,b)    (((a) < (b)) ? (a) : (b))

void sendByte(uint8_t data);
void sendByteLSB(uint8_t data);
void sendDisplayBuffer();

void clearDisplay();
void drawPixel(int16_t x, int16_t y);
void drawChar(int16_t x, int16_t y, unsigned char c, uint8_t size);
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h);
void drawFastVLine(int16_t x, int16_t y, int16_t h);
void drawFastHLine(int16_t x, int16_t y, int16_t w);
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void displayString(int16_t x, int16_t y, char_t *stringPtr, uint8_t size);
void displayMessage(uint8_t line, char_t *stringPtr, uint8_t maxChars);

#endif /* DISPLAY_H_ */
