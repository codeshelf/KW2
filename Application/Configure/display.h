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

#define WHITE				1
#define	BLACK				0

#define LCDCMD_DUMMY		0x00
#define LCDCMD_DISPLAY		0x00
#define LCDCMD_WRITE		0x80
#define LCDCMD_VCOM			0x40
#define LCDCMD_CLEAR		0x20

#define SPI_CS_ON     		SharpDisplay_CS_SetVal();
#define SPI_CS_OFF    		Wait_Waitus(100); SharpDisplay_CS_ClrVal();


void clearDisplay();
void sendByte(uint8_t data);
void sendByteLSB(uint8_t data);
void sendDisplayBuffer(uint8_t pattern);

#endif /* DISPLAY_H_ */
