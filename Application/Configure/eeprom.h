/*
 * Display.h
 *
 *  Created on: Jun 12, 2014
 *      Author: jeffw
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include "PE_Types.h"
#include "EepromCS.h"
#include "Wait.h"

#define Eeprom_DEVICE 		SPI0_BASE_PTR

#define READ_CMD			0b00000011
#define WRITE_CMD			0b00000010
#define READSRS_CMD			0b00000101
#define WRITESRS_CMD		0b00000001
#define WRITE_ENABLE_CMD	0b00000110
#define WRITE_DISABLECMD	0b00000100

#define EEPROM_CS_ON     	EepromCS_ClrVal();
#define EEPROM_CS_OFF    	Wait_Waitus(5); EepromCS_SetVal();

void spiSendByte(uint8_t data);
uint8_t spiGetByte(void);
bool readEepromData(uint16_t addr, uint8_t* dataPtr, uint8_t bytesToRead);

#endif /* EEPROM_H_ */
